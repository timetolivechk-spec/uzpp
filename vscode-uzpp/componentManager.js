'use strict';

const path = require('path');
const fs = require('fs');
const https = require('https');
const http = require('http');
const { execSync, spawnSync } = require('child_process');

const RELEASE_BASE = 'https://github.com/timetolivechk-spec/uzpp/releases/latest/download';

// Platform-specific asset names for GitHub releases
const PLATFORM_ASSETS = {
    'win32-x64':    { bundle: 'uzpp-windows-x64.zip',    exe: 'uzpp.exe' },
    'linux-x64':    { bundle: 'uzpp-linux-x64.tar.gz',   exe: 'uzpp' },
    'darwin-x64':   { bundle: 'uzpp-macos-x64.tar.gz',   exe: 'uzpp' },
    'darwin-arm64': { bundle: 'uzpp-macos-arm64.tar.gz', exe: 'uzpp' },
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
    const exeName = asset.exe;

    return {
        root,
        compilerDir:    path.join(root, 'compiler'),
        compilerExe:    path.join(root, 'compiler', exeName),
        stdlibDir:      path.join(root, 'compiler', 'stdlib'),
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
    const allOk = compilerOk && stdlibOk;

    return { compilerOk, stdlibOk, allOk, systemCpp: null, paths: p };
}

// ─── Download ────────────────────────────────────────────────────────────────

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
            const r = spawnSync('powershell.exe', [
                '-NoProfile', '-NonInteractive', '-Command',
                `Expand-Archive -Force -Path '${archivePath}' -DestinationPath '${destDir}'`
            ], { timeout: 300000 });
            if (r.status !== 0) throw new Error(`ZIP extract failed: ${r.stderr?.toString() || 'unknown'}`);
        } else {
            execSync(`unzip -o "${archivePath}" -d "${destDir}"`, { timeout: 300000 });
        }
    } else if (archivePath.endsWith('.tar.gz') || archivePath.endsWith('.tgz')) {
        execSync(`tar -xzf "${archivePath}" -C "${destDir}"`, { timeout: 300000 });
    } else {
        throw new Error(`Unknown archive format: ${path.basename(archivePath)}`);
    }
}

// ─── Install ─────────────────────────────────────────────────────────────────

async function installComponent(context, name, onProgress) {
    const p = getPaths(context);
    const tmpDir = path.join(p.root, 'tmp');
    fs.mkdirSync(tmpDir, { recursive: true });

    if (name === 'compiler') {
        const assetName = p.asset.bundle;
        const url       = `${RELEASE_BASE}/${assetName}`;
        const tmpFile   = path.join(tmpDir, assetName);

        onProgress?.('Yuklab olinmoqda: uz++ to\'liq paketi...', 0);
        await downloadFile(url, tmpFile, (dl, total) => {
            const dlMB    = (dl    / 1048576).toFixed(1);
            const totalMB = (total / 1048576).toFixed(1);
            onProgress?.(`uz++ to'liq paketi yuklanmoqda: ${dlMB}/${totalMB} MB`, dl / total);
        });

        onProgress?.('Arxiv ochilmoqda...', 0.9);
        const extractDir = path.join(tmpDir, 'compiler-extract');
        if (fs.existsSync(extractDir)) fs.rmSync(extractDir, { recursive: true });
        extractArchive(tmpFile, extractDir);

        // Assume the bundle extracts to a single directory (e.g., uzpp-windows-x64/) or directly contains files.
        // We need to find where uzpp.exe is.
        let bundleRoot = extractDir;
        const findExe = (dir) => {
            for (const item of fs.readdirSync(dir, { withFileTypes: true })) {
                if (item.isDirectory()) {
                    const res = findExe(path.join(dir, item.name));
                    if (res) return res;
                } else if (item.name === p.asset.exe) {
                    return dir;
                }
            }
            return null;
        };

        const actualRoot = findExe(extractDir);
        if (!actualRoot) throw new Error(`${p.asset.exe} not found in bundle archive!`);

        // Copy everything to compilerDir
        if (fs.existsSync(p.compilerDir)) fs.rmSync(p.compilerDir, { recursive: true });
        fs.cpSync(actualRoot, p.compilerDir, { recursive: true });
        if (process.platform !== 'win32') fs.chmodSync(p.compilerExe, 0o755);

        fs.rmSync(tmpFile,     { force: true });
        fs.rmSync(extractDir,  { recursive: true, force: true });
        onProgress?.('uz++ to\'liq paketi o\'rnatildi!', 1);
    }
}

function ensureWrapperScript(context) {
    const p = getPaths(context);
    fs.mkdirSync(p.root, { recursive: true });

    if (process.platform === 'win32') {
        const bat = [
            '@echo off',
            `set "UZPP_ROOT=${p.compilerDir}"`,
            `cd /d "${p.compilerDir}"`,
            `"${p.compilerExe}" %*`,
        ].join('\r\n') + '\r\n';
        fs.writeFileSync(p.wrapperScript, bat, 'ascii');
    } else {
        const sh = [
            '#!/bin/sh',
            `export UZPP_ROOT="${p.compilerDir}"`,
            `cd "${p.compilerDir}"`,
            `exec "${p.compilerExe}" "$@"`,
        ].join('\n') + '\n';
        fs.writeFileSync(p.wrapperScript, sh, 'utf8');
        fs.chmodSync(p.wrapperScript, 0o755);
    }
}

async function installAll(context, onProgress) {
    const status = checkComponents(context);
    const p = status.paths;
    fs.mkdirSync(p.root, { recursive: true });

    if (!status.allOk) {
        await installComponent(context, 'compiler', onProgress);
    }

    ensureWrapperScript(context);
    fs.writeFileSync(p.versionFile, JSON.stringify({
        installedAt: new Date().toISOString(),
        extensionVersion: '2.1.2',
    }));
    onProgress?.("Barcha komponentlar o'rnatildi!", 1);
}

function uninstallAll(context) {
    const p = getPaths(context);
    if (fs.existsSync(p.root)) fs.rmSync(p.root, { recursive: true, force: true });
}

function installFromArchive(context, archivePath, componentName) {
    const p = getPaths(context);
    const extractDir = path.join(p.root, 'tmp', 'manual-extract');
    if (fs.existsSync(extractDir)) fs.rmSync(extractDir, { recursive: true });
    fs.mkdirSync(extractDir, { recursive: true });

    extractArchive(archivePath, extractDir);

    let bundleRoot = extractDir;
    const findExe = (dir) => {
        for (const item of fs.readdirSync(dir, { withFileTypes: true })) {
            if (item.isDirectory()) {
                const res = findExe(path.join(dir, item.name));
                if (res) return res;
            } else if (item.name === p.asset.exe) {
                return dir;
            }
        }
        return null;
    };

    const actualRoot = findExe(extractDir);
    if (!actualRoot) throw new Error(`${p.asset.exe} not found in archive!`);

    if (fs.existsSync(p.compilerDir)) fs.rmSync(p.compilerDir, { recursive: true });
    fs.cpSync(actualRoot, p.compilerDir, { recursive: true });
    if (process.platform !== 'win32') fs.chmodSync(p.compilerExe, 0o755);

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