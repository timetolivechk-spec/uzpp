'use strict';

/*
 * ANALYSIS FINDINGS (src/main.cpp, src/package_manager.h):
 *
 * CLI commands (both Uzbek and English aliases exist):
 *   run:     "ishga-tushirish" or "run"
 *   build:   "qurish" or "build"
 *   format:  "formatlah"  (Uzbek only — no English alias)
 *   lint:    "tekshirish" / "check" / "lint"
 *   version: "--version" or "-v"
 *
 * Stdlib discovery (ProjectManager::findStdlibRoot):
 *   Walks UP from process.cwd() looking for "stdlib/uzpp_runtime.hpp".
 *   No env-var or CLI flag supported.
 *   Fix: the wrapper script changes CWD to storageRoot before running uzpp,
 *   so "storageRoot/stdlib/uzpp_runtime.hpp" is found automatically.
 *
 * g++ invocation:
 *   uzpp.exe calls g++ from PATH. No embedded path.
 *   Fix: wrapper script prepends <storageRoot>/mingw/bin to PATH.
 *
 * Generated C++ location (no uzpp.toml):
 *   <cwd>/build/<inputStem>.generated.cpp
 *   When running via wrapper: <storageRoot>/build/<stem>.generated.cpp
 */

const path = require('path');
const fs = require('fs');
const https = require('https');
const http = require('http');
const { execSync, spawnSync } = require('child_process');

const RELEASE_BASE = 'https://github.com/timetolivechk-spec/uzpp/releases/latest/download';

// Platform-specific asset names for GitHub releases
const PLATFORM_ASSETS = {
    'win32-x64':    { compiler: 'uzpp-windows-x64.zip',    exe: 'uzpp.exe', mingw: 'mingw-w64-windows-x64.zip' },
    'linux-x64':    { compiler: 'uzpp-linux-x64.tar.gz',   exe: 'uzpp',     mingw: null },
    'darwin-x64':   { compiler: 'uzpp-macos-x64.tar.gz',   exe: 'uzpp',     mingw: null },
    'darwin-arm64': { compiler: 'uzpp-macos-arm64.tar.gz', exe: 'uzpp',     mingw: null },
};

function getPlatformKey() {
    const p = process.platform;
    const a = process.arch === 'arm64' ? 'arm64' : 'x64';
    return `${p}-${a}`;
}

/** All computed storage paths in one object. */
function getPaths(context) {
    const root = context.globalStorageUri.fsPath;
    const asset = PLATFORM_ASSETS[getPlatformKey()] || PLATFORM_ASSETS['linux-x64'];
    const exeName = process.platform === 'win32' ? 'uzpp.exe' : 'uzpp';

    return {
        root,
        compilerDir:    path.join(root, 'compiler'),
        compilerExe:    path.join(root, 'compiler', exeName),
        stdlibDir:      path.join(root, 'stdlib'),
        mingwDir:       path.join(root, 'mingw'),
        mingwBin:       path.join(root, 'mingw', 'bin'),
        buildDir:       path.join(root, 'build'),
        wrapperScript:  path.join(root, process.platform === 'win32' ? 'uzpp-run.bat' : 'uzpp-run.sh'),
        versionFile:    path.join(root, 'version.json'),
        asset,
    };
}

/**
 * Checks which components are installed.
 * Returns an object with booleans and the computed paths.
 */
function checkComponents(context) {
    const p = getPaths(context);
    const compilerOk = fs.existsSync(p.compilerExe);
    const stdlibOk   = fs.existsSync(path.join(p.stdlibDir, 'uzpp_runtime.hpp'));
    const needsMingw = process.platform === 'win32';
    const mingwOk    = !needsMingw || fs.existsSync(path.join(p.mingwBin, 'g++.exe'));

    let systemCpp = null;
    if (!needsMingw) {
        for (const cmd of ['g++', 'clang++']) {
            try { execSync(`${cmd} --version`, { stdio: 'ignore', timeout: 3000 }); systemCpp = cmd; break; } catch {}
        }
    }

    const allOk = compilerOk && stdlibOk && (needsMingw ? mingwOk : (systemCpp !== null));
    return { compilerOk, stdlibOk, mingwOk, allOk, systemCpp, paths: p };
}

// ─── Download ────────────────────────────────────────────────────────────────

/**
 * Downloads a URL to destPath with redirect support and per-chunk progress.
 * onProgress(downloadedBytes, totalBytes)
 */
function downloadFile(url, destPath, onProgress) {
    return new Promise((resolve, reject) => {
        fs.mkdirSync(path.dirname(destPath), { recursive: true });

        const doRequest = (urlStr, redirects = 0) => {
            if (redirects > 8) return reject(new Error('Too many redirects'));
            const mod = urlStr.startsWith('https') ? https : http;
            const req = mod.get(urlStr, { headers: { 'User-Agent': 'vscode-uzpp/2.1' } }, (res) => {
                const { statusCode, headers } = res;
                if ([301, 302, 307, 308].includes(statusCode)) {
                    res.resume();
                    return doRequest(headers.location, redirects + 1);
                }
                if (statusCode !== 200) {
                    res.resume();
                    return reject(new Error(`HTTP ${statusCode} — ${urlStr}`));
                }
                const total = parseInt(headers['content-length'] || '0', 10);
                let downloaded = 0;
                const file = fs.createWriteStream(destPath);
                res.on('data', (chunk) => {
                    downloaded += chunk.length;
                    if (onProgress && total > 0) onProgress(downloaded, total);
                });
                res.pipe(file);
                file.on('finish', () => file.close(() => resolve(destPath)));
                file.on('error', (e) => { fs.unlink(destPath, () => {}); reject(e); });
                res.on('error',  (e) => { fs.unlink(destPath, () => {}); reject(e); });
            });
            req.on('error', reject);
            req.setTimeout(60000, () => { req.destroy(); reject(new Error('Download timeout')); });
        };
        doRequest(url);
    });
}

// ─── Extract ─────────────────────────────────────────────────────────────────

function extractArchive(archivePath, destDir) {
    fs.mkdirSync(destDir, { recursive: true });
    if (archivePath.endsWith('.zip')) {
        if (process.platform === 'win32') {
            // PowerShell Expand-Archive — available since PS 5.0 (Win10+)
            const r = spawnSync('powershell.exe', [
                '-NoProfile', '-NonInteractive', '-Command',
                `Expand-Archive -Force -Path '${archivePath}' -DestinationPath '${destDir}'`
            ], { timeout: 180000 });
            if (r.status !== 0) throw new Error(`ZIP extract failed: ${r.stderr?.toString() || 'unknown'}`);
        } else {
            execSync(`unzip -o "${archivePath}" -d "${destDir}"`, { timeout: 180000 });
        }
    } else if (archivePath.endsWith('.tar.gz') || archivePath.endsWith('.tgz')) {
        execSync(`tar -xzf "${archivePath}" -C "${destDir}"`, { timeout: 180000 });
    } else {
        throw new Error(`Unknown archive format: ${path.basename(archivePath)}`);
    }
}

// ─── Search helpers ──────────────────────────────────────────────────────────

function findFile(dir, name) {
    try {
        for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
            const full = path.join(dir, entry.name);
            if (entry.isDirectory()) { const found = findFile(full, name); if (found) return found; }
            else if (entry.name === name) return full;
        }
    } catch {}
    return null;
}

function findDir(dir, name, markerFile) {
    try {
        for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
            if (!entry.isDirectory()) continue;
            const full = path.join(dir, entry.name);
            if (entry.name === name) {
                if (!markerFile || fs.existsSync(path.join(full, markerFile))) return full;
            }
            const found = findDir(full, name, markerFile);
            if (found) return found;
        }
    } catch {}
    return null;
}

// ─── Install ─────────────────────────────────────────────────────────────────

/**
 * Installs one named component. name = 'compiler' | 'mingw'
 * onProgress(message, fraction 0-1)
 */
async function installComponent(context, name, onProgress) {
    const p = getPaths(context);
    const tmpDir = path.join(p.root, 'tmp');
    fs.mkdirSync(tmpDir, { recursive: true });

    if (name === 'compiler') {
        const assetName = p.asset.compiler;
        const url       = `${RELEASE_BASE}/${assetName}`;
        const tmpFile   = path.join(tmpDir, assetName);

        onProgress?.('Yuklab olinmoqda: uz++ kompilyatori...', 0);
        await downloadFile(url, tmpFile, (dl, total) => {
            const dlMB    = (dl    / 1048576).toFixed(1);
            const totalMB = (total / 1048576).toFixed(1);
            onProgress?.(`uz++ kompilyatori yuklanmoqda: ${dlMB}/${totalMB} MB`, dl / total);
        });

        onProgress?.('Arxiv ochilmoqda...', 0.9);
        const extractDir = path.join(tmpDir, 'compiler-extract');
        if (fs.existsSync(extractDir)) fs.rmSync(extractDir, { recursive: true });
        extractArchive(tmpFile, extractDir);

        // Install exe
        const exePath = findFile(extractDir, p.asset.exe);
        if (!exePath) throw new Error(`${p.asset.exe} not found in release archive`);
        fs.mkdirSync(p.compilerDir, { recursive: true });
        fs.copyFileSync(exePath, p.compilerExe);
        if (process.platform !== 'win32') fs.chmodSync(p.compilerExe, 0o755);

        // Install stdlib (find the stdlib/ dir containing uzpp_runtime.hpp)
        const stdlibSrc = findDir(extractDir, 'stdlib', 'uzpp_runtime.hpp');
        if (stdlibSrc) {
            if (fs.existsSync(p.stdlibDir)) fs.rmSync(p.stdlibDir, { recursive: true });
            fs.cpSync(stdlibSrc, p.stdlibDir, { recursive: true });
        }

        fs.rmSync(tmpFile,     { force: true });
        fs.rmSync(extractDir,  { recursive: true, force: true });
        onProgress?.('uz++ kompilyatori o\'rnatildi!', 1);
    }

    if (name === 'mingw' && process.platform === 'win32') {
        const assetName = p.asset.mingw;
        if (!assetName) return;
        const url     = `${RELEASE_BASE}/${assetName}`;
        const tmpFile = path.join(tmpDir, assetName);

        onProgress?.('Yuklab olinmoqda: C++ kompilyatori (MinGW) — 150+ MB...', 0);
        await downloadFile(url, tmpFile, (dl, total) => {
            const dlMB    = (dl    / 1048576).toFixed(1);
            const totalMB = total > 0 ? (total / 1048576).toFixed(1) : '?';
            onProgress?.(`MinGW (C++ kompilyatori) yuklanmoqda: ${dlMB}/${totalMB} MB`, total > 0 ? dl / total : 0);
        });

        onProgress?.('MinGW ochilmoqda...', 0.92);
        const extractDir = path.join(tmpDir, 'mingw-extract');
        if (fs.existsSync(extractDir)) fs.rmSync(extractDir, { recursive: true });
        extractArchive(tmpFile, extractDir);

        // Find the mingw root — the directory whose bin/g++.exe exists
        const gppExe = findFile(extractDir, 'g++.exe');
        if (!gppExe) throw new Error('g++.exe not found in MinGW archive');
        const mingwRoot = path.dirname(path.dirname(gppExe)); // bin/../ = root

        if (fs.existsSync(p.mingwDir)) fs.rmSync(p.mingwDir, { recursive: true });
        fs.cpSync(mingwRoot, p.mingwDir, { recursive: true });

        fs.rmSync(tmpFile,    { force: true });
        fs.rmSync(extractDir, { recursive: true, force: true });
        onProgress?.('C++ kompilyatori (MinGW) o\'rnatildi!', 1);
    }
}

/**
 * Creates/refreshes the platform wrapper script in storageRoot.
 * The wrapper: sets PATH to include MinGW, CDs to storageRoot (so stdlib
 * is found by uzpp's walk-up search), then execs uzpp with all args.
 */
function ensureWrapperScript(context) {
    const p = getPaths(context);
    fs.mkdirSync(p.root, { recursive: true });

    if (process.platform === 'win32') {
        const bat = [
            '@echo off',
            `set "UZPP_ROOT=${p.root}"`,
            `set "PATH=${p.mingwBin};%PATH%"`,
            `cd /d "${p.root}"`,
            `"${p.compilerExe}" %*`,
        ].join('\r\n') + '\r\n';
        fs.writeFileSync(p.wrapperScript, bat, 'ascii');
    } else {
        const sh = [
            '#!/bin/sh',
            `export PATH="${p.mingwBin}:$PATH"`,
            `cd "${p.root}"`,
            `exec "${p.compilerExe}" "$@"`,
        ].join('\n') + '\n';
        fs.writeFileSync(p.wrapperScript, sh, 'utf8');
        fs.chmodSync(p.wrapperScript, 0o755);
    }
}

/** Install all missing components, then write wrapper script. */
async function installAll(context, onProgress) {
    const status = checkComponents(context);
    const p = status.paths;
    fs.mkdirSync(p.root, { recursive: true });

    if (!status.compilerOk || !status.stdlibOk) {
        await installComponent(context, 'compiler', onProgress);
    }
    if (process.platform === 'win32' && !status.mingwOk) {
        await installComponent(context, 'mingw', onProgress);
    }

    ensureWrapperScript(context);
    fs.writeFileSync(p.versionFile, JSON.stringify({
        installedAt: new Date().toISOString(),
        extensionVersion: '2.1.0',
    }));
    onProgress?.("Barcha komponentlar o'rnatildi!", 1);
}

/** Delete all downloaded components. */
function uninstallAll(context) {
    const p = getPaths(context);
    if (fs.existsSync(p.root)) fs.rmSync(p.root, { recursive: true, force: true });
}

/**
 * Manual offline install from a user-selected archive.
 * componentName = 'compiler' | 'mingw'
 */
function installFromArchive(context, archivePath, componentName) {
    const p = getPaths(context);
    const extractDir = path.join(p.root, 'tmp', 'manual-extract');
    if (fs.existsSync(extractDir)) fs.rmSync(extractDir, { recursive: true });
    fs.mkdirSync(extractDir, { recursive: true });

    extractArchive(archivePath, extractDir);

    if (componentName === 'compiler') {
        const exePath = findFile(extractDir, p.asset.exe);
        if (!exePath) throw new Error(`${p.asset.exe} not found in archive`);
        fs.mkdirSync(p.compilerDir, { recursive: true });
        fs.copyFileSync(exePath, p.compilerExe);
        if (process.platform !== 'win32') fs.chmodSync(p.compilerExe, 0o755);

        const stdlibSrc = findDir(extractDir, 'stdlib', 'uzpp_runtime.hpp');
        if (stdlibSrc) {
            if (fs.existsSync(p.stdlibDir)) fs.rmSync(p.stdlibDir, { recursive: true });
            fs.cpSync(stdlibSrc, p.stdlibDir, { recursive: true });
        }
    } else if (componentName === 'mingw') {
        const gppExe = findFile(extractDir, 'g++.exe');
        if (!gppExe) throw new Error('g++.exe not found in archive');
        const mingwRoot = path.dirname(path.dirname(gppExe));
        if (fs.existsSync(p.mingwDir)) fs.rmSync(p.mingwDir, { recursive: true });
        fs.cpSync(mingwRoot, p.mingwDir, { recursive: true });
    }

    ensureWrapperScript(context);
    fs.rmSync(extractDir, { recursive: true, force: true });
}

module.exports = {
    getPaths,
    checkComponents,
    downloadFile,
    extractArchive,
    installComponent,
    installAll,
    uninstallAll,
    installFromArchive,
    ensureWrapperScript,
    RELEASE_BASE,
};
