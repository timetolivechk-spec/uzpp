'use strict';

const vscode = require('vscode');
const path = require('path');
const fs = require('fs');
const { execSync } = require('child_process');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');
const cm = require('./componentManager');

/** @type {LanguageClient|undefined} */
let client;
/** @type {vscode.StatusBarItem} */
let statusBarItem;

// ─── Compiler Resolution ──────────────────────────────────────────────────────

/**
 * Looks up `uzpp` on the system PATH using `where` (Windows) / `which` (POSIX).
 * Returns the first absolute path found, or null. Used so users who installed
 * uz++ via the Windows installer or apt/brew don't need a per-extension copy
 * downloaded into globalStorage.
 */
function findCompilerOnPath() {
    try {
        const cmd = process.platform === 'win32' ? 'where uzpp' : 'which uzpp';
        const out = execSync(cmd, { timeout: 3000, stdio: ['ignore', 'pipe', 'ignore'] })
            .toString().trim();
        if (!out) return null;
        // `where` may return multiple matches (one per line); pick the first.
        const first = out.split(/\r?\n/)[0].trim();
        return first.length > 0 && fs.existsSync(first) ? first : null;
    } catch {
        return null;
    }
}

/**
 * Returns the path to run uzpp. Priority:
 * 1. Wrapper script in globalStorage (handles CWD + MinGW PATH on Windows)
 * 2. Embedded bin/ inside extension
 * 3. User settings compilerPath
 * 4. uzpp on the system PATH (e.g. installed via uzpp-setup.exe)
 * 5. Workspace folder
 * 6. Bare exe name as last-resort PATH fallback
 */
function findCompilerPath(context) {
    const p = cm.getPaths(context);

    // Prefer the wrapper script — it sets CWD and PATH correctly
    if (fs.existsSync(p.wrapperScript) && fs.existsSync(p.compilerExe)) {
        return p.wrapperScript;
    }

    // Embedded in extension (legacy path for devs who manually placed the binary)
    const embeddedExe = context.asAbsolutePath(
        path.join('bin', process.platform === 'win32' ? 'uzpp.exe' : 'uzpp')
    );
    if (fs.existsSync(embeddedExe)) return embeddedExe;

    // User setting
    const configured = vscode.workspace.getConfiguration('uzpp').get('compilerPath');
    if (configured && configured.length > 0) return configured;

    // System install: look on PATH so users of uzpp-setup.exe / Homebrew /
    // apt don't need a duplicate in globalStorage.
    const onPath = findCompilerOnPath();
    if (onPath) return onPath;

    // Workspace folder
    const folders = vscode.workspace.workspaceFolders;
    if (folders && folders.length > 0) {
        const candidate = path.join(folders[0].uri.fsPath,
            process.platform === 'win32' ? 'uzpp.exe' : 'uzpp');
        if (fs.existsSync(candidate)) return candidate;
    }

    return process.platform === 'win32' ? 'uzpp.exe' : 'uzpp';
}

const RELEASES_LATEST_URL = 'https://github.com/timetolivechk-spec/uzpp/releases/latest';
const WIN_SETUP_URL = 'https://github.com/timetolivechk-spec/uzpp/releases/latest/download/uzpp-setup.exe';

/**
 * Show a notification offering install paths when uzpp is not found anywhere.
 * Three actions:
 *   - "Avtomatik o'rnatish" — Windows: open the uzpp-setup.exe download URL
 *     in the browser. Linux/macOS: open the releases page.
 *   - "Komponentlar (eski usul)" — fall back to the existing globalStorage
 *     install flow (downloads MinGW + uzpp directly).
 *   - "Yo'lni ko'rsatish" — let the user point to an existing uzpp binary
 *     and persist that into the `uzpp.compilerPath` setting.
 */
async function offerCompilerInstall(context) {
    const winInstall  = "Avtomatik o'rnatish";
    const fallback    = 'Komponentlar (eski usul)';
    const manual      = "Yo'lni ko'rsatish";
    const dismiss     = "Keyinroq";

    const buttons = process.platform === 'win32'
        ? [winInstall, fallback, manual, dismiss]
        : [fallback, manual, dismiss];

    const choice = await vscode.window.showInformationMessage(
        "uz++ kompilyator topilmadi. Tezkor o'rnatishni tanlang yoki mavjud yo'lni ko'rsating.",
        ...buttons
    );

    if (!choice || choice === dismiss) return;

    if (choice === winInstall) {
        // Open the download in the user's browser. Once they run uzpp-setup.exe
        // and reload VS Code, findCompilerOnPath() will pick it up.
        await vscode.env.openExternal(vscode.Uri.parse(WIN_SETUP_URL));
        vscode.window.showInformationMessage(
            "uzpp-setup.exe yuklab olinmoqda. Yuklab olish tugagach ishga tushiring, "
          + "so'ng VS Code ni qayta ishga tushiring."
        );
        return;
    }

    if (choice === fallback) {
        await cmdInstallComponents(context);
        return;
    }

    if (choice === manual) {
        const picked = await vscode.window.showOpenDialog({
            canSelectFolders: false, canSelectFiles: true, canSelectMany: false,
            openLabel: "uzpp.exe ni tanlang",
            filters: process.platform === 'win32' ? { 'uzpp executable': ['exe'] } : undefined,
        });
        if (!picked || picked.length === 0) return;
        const chosenPath = picked[0].fsPath;
        await vscode.workspace.getConfiguration('uzpp')
            .update('compilerPath', chosenPath, vscode.ConfigurationTarget.Global);
        vscode.window.showInformationMessage(
            `Saqlandi: ${chosenPath}. VS Code ni qayta ishga tushiring.`
        );
    }
}

function getCompilerVersion(compilerPath) {
    try {
        // For wrapper .bat — invoke via cmd to get clean output
        let cmd = `"${compilerPath}" --version`;
        if (process.platform === 'win32' && compilerPath.endsWith('.bat')) {
            cmd = `cmd /c "${cmd}"`;
        }
        const out = execSync(cmd, { timeout: 5000, stdio: ['ignore', 'pipe', 'ignore'] }).toString().trim();
        const m = out.match(/\d+\.\d+\.\d+/);
        return m ? m[0] : out.split('\n')[0].slice(0, 30);
    } catch {
        return null;
    }
}

// ─── Terminal ─────────────────────────────────────────────────────────────────

let _terminal = null;
function getTerminal() {
    if (_terminal && !_terminal.exitStatus) return _terminal;
    _terminal = vscode.window.createTerminal({ name: 'uz++' });
    return _terminal;
}

/** Build the terminal invocation line for a given uzpp verb and file. */
function buildRunCommand(context, verb, filePath) {
    const compiler = findCompilerPath(context);
    // On Windows with a .bat wrapper: invoke via & in PowerShell
    if (process.platform === 'win32' && compiler.endsWith('.bat')) {
        // & invokes the batch file in a subprocess — CWD change is isolated
        return `& "${compiler}" ${verb} "${filePath}"`;
    }
    return `"${compiler}" ${verb} "${filePath}"`;
}

// ─── Component Guard ──────────────────────────────────────────────────────────

/**
 * Checks components are ready. If not, offers to install.
 * Returns true if safe to proceed, false if user must install first.
 *
 * "Ready" means EITHER the per-extension globalStorage payload is present
 * OR `uzpp` is reachable on the system PATH (installer / package manager).
 */
async function requireComponents(context) {
    const status = cm.checkComponents(context);
    if (status.allOk) return true;
    if (findCompilerOnPath() !== null) return true;

    const missing = [];
    if (!status.compilerOk) missing.push('uz++ kompilyatori');
    if (!status.stdlibOk)   missing.push('standart kutubxona');
    if (process.platform === 'win32' && !status.mingwOk) missing.push('C++ kompilyatori (MinGW)');

    const answer = await vscode.window.showWarningMessage(
        `Zarur komponentlar topilmadi: ${missing.join(', ')}`,
        "O'rnatish", 'Bekor qilish'
    );
    if (answer === "O'rnatish") {
        await cmdInstallComponents(context);
        return cm.checkComponents(context).allOk;
    }
    return false;
}

// ─── Commands ─────────────────────────────────────────────────────────────────

async function cmdRunFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();
    if (!await requireComponents(context)) return;

    const file = editor.document.uri.fsPath;
    const term = getTerminal();
    term.show(true);
    term.sendText(buildRunCommand(context, 'ishga-tushirish', file));
}

async function cmdBuildFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();
    if (!await requireComponents(context)) return;

    const file = editor.document.uri.fsPath;
    const term = getTerminal();
    term.show(true);
    term.sendText(buildRunCommand(context, 'qurish', file));
}

async function cmdFormatFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();

    const compiler = findCompilerPath(context);
    const file = editor.document.uri.fsPath;
    try {
        let cmd = `"${compiler}" formatlah "${file}"`;
        if (process.platform === 'win32' && compiler.endsWith('.bat')) {
            cmd = `cmd /c ${cmd}`;
        }
        execSync(cmd, { timeout: 10000 });
        vscode.window.showInformationMessage('Fayl formatlandi.');
    } catch (e) {
        vscode.window.showErrorMessage(`Format xatosi: ${e.message}`);
    }
}

async function cmdShowGeneratedCpp(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();
    if (!await requireComponents(context)) return;

    const file = editor.document.uri.fsPath;
    const stem = path.basename(file, '.uzpp');
    const p = cm.getPaths(context);

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: 'C++ kodi generatsiya qilinmoqda...' },
        async () => {
            try {
                let cmd = `"${findCompilerPath(context)}" qurish "${file}"`;
                if (process.platform === 'win32' && findCompilerPath(context).endsWith('.bat')) {
                    cmd = `cmd /c ${cmd}`;
                }
                execSync(cmd, { timeout: 30000 });
            } catch (e) {
                vscode.window.showErrorMessage(`Build xatosi: ${e.message}`);
                return;
            }
        }
    );

    // Search in likely locations
    const candidates = [
        path.join(p.buildDir, `${stem}.generated.cpp`),      // via wrapper (CWD=storageRoot)
        path.join(path.dirname(file), 'build', `${stem}.generated.cpp`), // local project
        path.join(path.dirname(file), `${stem}.generated.cpp`),
    ];

    for (const candidate of candidates) {
        if (fs.existsSync(candidate)) {
            const doc = await vscode.workspace.openTextDocument(candidate);
            await vscode.window.showTextDocument(doc, vscode.ViewColumn.Beside);
            return;
        }
    }
    vscode.window.showWarningMessage('Generatsiya qilingan C++ fayl topilmadi. Birinchi qurish buyrug\'ini bajaring.');
}

async function cmdNewProject(context) {
    const name = await vscode.window.showInputBox({
        prompt: 'Loyiha nomi',
        value: 'mening_loyiham',
        validateInput: v => /^[\w-]+$/.test(v) ? null : 'Faqat harflar, raqamlar, _ va - belgilari'
    });
    if (!name) return;

    const folderResult = await vscode.window.showOpenDialog({
        canSelectFolders: true, canSelectFiles: false,
        openLabel: 'Loyiha joyini tanlang'
    });
    if (!folderResult || folderResult.length === 0) return;

    const projectDir = path.join(folderResult[0].fsPath, name);
    fs.mkdirSync(projectDir, { recursive: true });

    fs.writeFileSync(path.join(projectDir, 'asosiy.uzpp'), [
        `// uz++ loyihasi: ${name}`,
        '',
        'ulash "uzpp_runtime.hpp"',
        '',
        'butun asosiy() {',
        `    yozish << "Salom, ${name}!" << qator_oxiri;`,
        '    qaytarish 0;',
        '}',
        ''
    ].join('\n'));

    fs.writeFileSync(path.join(projectDir, 'uzpp.toml'), [
        `[loyiha]`,
        `nom = "${name}"`,
        `versiya = "0.1.0"`,
        '',
        `[kompilyator]`,
        `standart = "c++23"`,
        ''
    ].join('\n'));

    await vscode.commands.executeCommand(
        'vscode.openFolder',
        vscode.Uri.file(projectDir),
        { forceNewWindow: false }
    );
}

async function cmdInstallComponents(context) {
    await vscode.window.withProgress(
        {
            location: vscode.ProgressLocation.Notification,
            title: "uz++ komponentlari o'rnatilmoqda...",
            cancellable: false,
        },
        async (progress) => {
            try {
                await cm.installAll(context, (msg, fraction) => {
                    progress.report({ message: msg, increment: Math.round((fraction ?? 0) * 100) });
                });
                updateStatusBar(context);
                vscode.window.showInformationMessage(
                    "Tayyor! F5 tugmasini bosib birinchi dasturni ishga tushiring.",
                    'Yangi loyiha'
                ).then(action => {
                    if (action === 'Yangi loyiha') cmdNewProject(context);
                });
            } catch (e) {
                vscode.window.showErrorMessage(
                    `O'rnatish xatosi: ${e.message}`,
                    'Fayldan o\'rnatish'
                ).then(action => {
                    if (action === "Fayldan o'rnatish") cmdInstallFromFile(context);
                });
            }
        }
    );
}

async function cmdCheckComponents(context) {
    const status = cm.checkComponents(context);
    const p = status.paths;
    const ok = (b) => b ? '✓' : '✗';
    const lines = [
        `uz++ kompilyatori: ${ok(status.compilerOk)} ${status.compilerOk ? p.compilerExe : '(topilmadi)'}`,
        `Standart kutubxona: ${ok(status.stdlibOk)} ${status.stdlibOk ? p.stdlibDir : '(topilmadi)'}`,
    ];
    if (process.platform === 'win32') {
        lines.push(`MinGW (C++ kompilyatori): ${ok(status.mingwOk)} ${status.mingwOk ? p.mingwBin : '(topilmadi)'}`);
    } else {
        lines.push(`Tizim C++ kompilyatori: ${ok(status.systemCpp !== null)} ${status.systemCpp || '(topilmadi)'}`);
    }
    lines.push(`Umumiy holat: ${status.allOk ? '✓ Tayyor' : '✗ Komponentlar yetishmayapti'}`);

    const msg = lines.join('\n');
    const action = await vscode.window.showInformationMessage(
        msg,
        ...(status.allOk ? [] : ["O'rnatish"])
    );
    if (action === "O'rnatish") await cmdInstallComponents(context);
}

async function cmdUninstallComponents(context) {
    const answer = await vscode.window.showWarningMessage(
        "Barcha uz++ komponentlarini o'chirishni xohlaysizmi? (kompilyator, stdlib, MinGW)",
        "Ha, o'chirish", 'Bekor qilish'
    );
    if (answer !== "Ha, o'chirish") return;
    cm.uninstallAll(context);
    updateStatusBar(context);
    vscode.window.showInformationMessage("Barcha komponentlar o'chirildi.");
}

async function cmdInstallFromFile(context) {
    const fileResult = await vscode.window.showOpenDialog({
        canSelectFolders: false, canSelectFiles: true,
        openLabel: 'Arxivni tanlang',
        filters: {
            'Arxiv fayllari': ['zip', 'tar.gz', 'tgz'],
            'Barcha fayllar': ['*']
        }
    });
    if (!fileResult || fileResult.length === 0) return;

    const archivePath = fileResult[0].fsPath;
    const isMinGW = await vscode.window.showQuickPick(
        [
            { label: 'uz++ kompilyatori + stdlib', value: 'compiler' },
            { label: 'MinGW (C++ kompilyatori, faqat Windows)', value: 'mingw' },
        ],
        { placeHolder: 'Bu arxiv nimani o\'z ichiga oladi?' }
    );
    if (!isMinGW) return;

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: 'Fayldan o\'rnatilmoqda...' },
        async () => {
            try {
                cm.installFromArchive(context, archivePath, isMinGW.value);
                updateStatusBar(context);
                vscode.window.showInformationMessage('O\'rnatildi!');
            } catch (e) {
                vscode.window.showErrorMessage(`Xato: ${e.message}`);
            }
        }
    );
}

async function cmdUpdateCompiler(context) {
    const answer = await vscode.window.showInformationMessage(
        "uz++ kompilyatorini GitHub'dan yangilash (kompilator + stdlib)?",
        'Ha', "Yo'q"
    );
    if (answer !== 'Ha') return;

    const p = cm.getPaths(context);
    // Backup existing compiler dir
    if (fs.existsSync(p.compilerDir)) {
        const backup = p.compilerDir + '.bak';
        if (fs.existsSync(backup)) fs.rmSync(backup, { recursive: true });
        fs.renameSync(p.compilerDir, backup);
    }

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: 'Yangilanmoqda...' },
        async (progress) => {
            try {
                await cm.installComponent(context, 'compiler', (msg, fraction) => {
                    progress.report({ message: msg, increment: Math.round((fraction ?? 0) * 100) });
                });
                cm.ensureWrapperScript(context);
                updateStatusBar(context);
                // Remove backup
                const backup = p.compilerDir + '.bak';
                if (fs.existsSync(backup)) fs.rmSync(backup, { recursive: true });
                vscode.window.showInformationMessage('uz++ muvaffaqiyatli yangilandi!');
            } catch (e) {
                // Restore backup on failure
                const backup = p.compilerDir + '.bak';
                if (fs.existsSync(backup)) {
                    if (fs.existsSync(p.compilerDir)) fs.rmSync(p.compilerDir, { recursive: true });
                    fs.renameSync(backup, p.compilerDir);
                }
                vscode.window.showErrorMessage(`Yangilash xatosi: ${e.message}`);
            }
        }
    );
}

// ─── Status Bar ───────────────────────────────────────────────────────────────

/**
 * Determines if a usable uz++ compiler is reachable.
 * Looks first at the per-extension globalStorage payload, then at the system
 * PATH (so users who installed via uzpp-setup.exe / Homebrew / apt are
 * recognized as already set up — no second download needed).
 */
function isCompilerReady(context) {
    if (cm.checkComponents(context).allOk) return true;
    return findCompilerOnPath() !== null;
}

function updateStatusBar(context) {
    const status = cm.checkComponents(context);
    const onPath = findCompilerOnPath();

    if (status.allOk) {
        const version = getCompilerVersion(status.paths.compilerExe);
        statusBarItem.text    = version ? `$(bracket) uz++ v${version}` : '$(bracket) uz++';
        statusBarItem.tooltip = `uz++ tayyor. F5 → ishga tushirish.`;
        statusBarItem.color   = undefined;
        statusBarItem.command = 'uzpp.runFile';
    } else if (onPath) {
        const version = getCompilerVersion(onPath);
        statusBarItem.text    = version ? `$(bracket) uz++ v${version}` : '$(bracket) uz++';
        statusBarItem.tooltip = `uz++ PATH'da topildi: ${onPath}`;
        statusBarItem.color   = undefined;
        statusBarItem.command = 'uzpp.runFile';
    } else {
        statusBarItem.text    = '$(bracket) uz++ ⚠';
        statusBarItem.tooltip = "Komponentlar o'rnatilmagan. Bosing.";
        statusBarItem.color   = new vscode.ThemeColor('statusBarItem.warningForeground');
        statusBarItem.command = 'uzpp.installComponents';
    }
    statusBarItem.show();
}

// ─── Welcome Screen ───────────────────────────────────────────────────────────

function showWelcome(context) {
    const panel = vscode.window.createWebviewPanel(
        'uzppWelcome',
        "uz++ — Xush Kelibsiz!",
        vscode.ViewColumn.One,
        { enableScripts: true, retainContextWhenHidden: true }
    );

    // Compute compiler info for the welcome screen. If reachable via PATH or
    // via globalStorage, show its version + path so the user knows uz++ is set
    // up — but the "Install / reinstall" button is ALWAYS visible regardless.
    const status   = cm.checkComponents(context);
    const onPath   = findCompilerOnPath();
    const exePath  = status.allOk ? status.paths.compilerExe : (onPath || null);
    const version  = exePath ? getCompilerVersion(exePath) : null;
    const compilerInfo = exePath ? { ready: true, exe: exePath, version } : { ready: false };

    panel.webview.onDidReceiveMessage(async (msg) => {
        switch (msg.command) {
            case 'runInstaller':
                await runOfficialInstaller(panel);
                break;
            case 'newProject':
                await cmdNewProject(context);
                break;
            case 'installFromFile':
                await cmdInstallFromFile(context);
                break;
            case 'openDocs':
                vscode.env.openExternal(
                    vscode.Uri.parse('https://github.com/timetolivechk-spec/uzpp/blob/main/docs/getting-started.md')
                );
                break;
            case 'openReleases':
                vscode.env.openExternal(vscode.Uri.parse(RELEASES_LATEST_URL));
                break;
        }
    }, undefined, context.subscriptions);

    panel.webview.html = buildWelcomeHtml(compilerInfo);
    return panel;
}

/**
 * Downloads the official uzpp-setup.exe from the latest GitHub release and
 * launches it via the system shell. After the user finishes the installer
 * and reloads VS Code, findCompilerOnPath() will discover uz++ automatically.
 *
 * Linux/macOS users get the releases page in their browser instead — Inno
 * Setup is Windows-only and bundling AppImage / pkg installers is on the
 * roadmap (see README Quick Install table).
 */
async function runOfficialInstaller(panel) {
    if (process.platform !== 'win32') {
        await vscode.env.openExternal(vscode.Uri.parse(RELEASES_LATEST_URL));
        panel?.webview.postMessage({
            type: 'info',
            text: 'Linux/macOS uchun rasmiy o\'rnatuvchi hali tayyor emas. ' +
                  'Manba koddan qurish bo\'yicha ko\'rsatma brauzerda ochildi.'
        });
        return;
    }

    panel?.webview.postMessage({ type: 'progress', text: 'O\'rnatuvchi yuklanmoqda...', fraction: 0 });
    try {
        const tmpDir   = require('os').tmpdir();
        const dest     = path.join(tmpDir, 'uzpp-setup.exe');
        const setupUrl = `${RELEASES_LATEST_URL}/download/uzpp-setup.exe`;

        await cm.downloadFile(setupUrl, dest, (dl, total) => {
            const dlMB    = (dl    / 1048576).toFixed(1);
            const totalMB = total > 0 ? (total / 1048576).toFixed(1) : '?';
            panel?.webview.postMessage({
                type: 'progress',
                text: `O'rnatuvchi yuklanmoqda: ${dlMB}/${totalMB} MB`,
                fraction: total > 0 ? dl / total : 0,
            });
        });

        panel?.webview.postMessage({
            type: 'info',
            text: "O'rnatuvchi ishga tushdi. Qadamlar bo'yicha o'rnatishni yakunlang, " +
                  "so'ng VS Code'ni qayta ishga tushiring (Reload Window)."
        });

        // Detached so the installer survives the VS Code window restart that
        // typically follows. UAC will appear if user picks Program Files.
        const child = require('child_process').spawn(dest, [], {
            detached: true, stdio: 'ignore', windowsHide: false,
        });
        child.unref();
    } catch (e) {
        panel?.webview.postMessage({ type: 'error', text: e.message });
        const action = await vscode.window.showErrorMessage(
            `Avtomatik yuklash xatosi: ${e.message}`,
            'Brauzerda ochish'
        );
        if (action === 'Brauzerda ochish') {
            vscode.env.openExternal(vscode.Uri.parse(RELEASES_LATEST_URL));
        }
    }
}

async function showWelcomeIfFirstRun(context) {
    const key = 'uzpp.welcomeShown.v2';
    if (context.globalState.get(key)) return;
    await context.globalState.update(key, true);
    showWelcome(context);
}

function buildWelcomeHtml(compilerInfo) {
    const isWin = process.platform === 'win32';
    const ready = compilerInfo && compilerInfo.ready;

    // Status banner: green if compiler is reachable, yellow otherwise.
    const statusBanner = ready
        ? `<div class="status-box ready">
             <div class="status-icon">✓</div>
             <div class="status-text">
               <strong>uz++ tayyor.</strong>
               ${compilerInfo.version ? ` Versiya <code>${compilerInfo.version}</code>.` : ''}
               <div class="status-path"><code>${compilerInfo.exe}</code></div>
             </div>
           </div>`
        : `<div class="status-box not-ready">
             <div class="status-icon">⚠</div>
             <div class="status-text">
               <strong>uz++ kompilyatori topilmadi.</strong>
               Pastdagi tugma orqali o'rnating yoki PATH'dagi mavjud nusxani ko'rsating.
             </div>
           </div>`;

    // Install button is ALWAYS visible. Label adapts to current state so users
    // who already have uz++ can still re-install / upgrade from this screen.
    const installLabel = isWin
        ? (ready ? "uz++ ni qayta o'rnatish / yangilash (~115 MB)"
                 : "Hammasini o'rnatish — uzpp-setup.exe (~115 MB)")
        : (ready ? "Releases sahifasini ochish (yangilash uchun)"
                 : "Releases sahifasini ochish");

    const installDescription = isWin
        ? `<p>Bu rasmiy <code>uzpp-setup.exe</code> ni yuklab oladi va ishga tushiradi.
            Ichida MinGW GCC 14.2 ham bor — boshqa hech narsa o'rnatish kerak emas.
            O'rnatuvchi <code>%LOCALAPPDATA%\\Programs\\uzpp\\</code> ga o'rnatadi
            (admin huquqsiz).</p>`
        : `<p>Linux/macOS uchun rasmiy paket hali yo'q. Brauzerda releases
            sahifasi ochiladi — manba koddan qurish bo'yicha
            <a href="#" onclick="vscode.postMessage({command:'openDocs'})">qo'llanma</a>
            bilan davom eting.</p>`;

    const installSection = `<div class="install-box">
        ${statusBanner}
        <button id="btn-install" class="btn-primary" onclick="startInstall()">
          ${installLabel}
        </button>
        ${installDescription}
        <div id="progress-area" style="display:none">
          <div class="progress-bar-bg"><div id="progress-bar" class="progress-bar-fill"></div></div>
          <div id="progress-text" class="progress-text">Tayyorlanmoqda...</div>
        </div>
        <div id="info-area"  style="display:none" class="info-box"></div>
        <div id="error-area" style="display:none" class="error-box"></div>
      </div>`;

    return `<!DOCTYPE html>
<html lang="uz">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<meta http-equiv="Content-Security-Policy" content="default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline';">
<title>uz++ ga Xush Kelibsiz</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
         max-width: 720px; margin: 0 auto; padding: 32px 24px;
         color: var(--vscode-foreground);
         background: var(--vscode-editor-background);
         line-height: 1.7; }
  h1 { color: var(--vscode-textLink-foreground); font-size: 2em; margin-bottom: 6px; }
  h2 { color: var(--vscode-textLink-foreground); margin: 28px 0 10px; font-size: 1.2em; }
  .subtitle { color: var(--vscode-descriptionForeground); margin-bottom: 24px; }
  pre, code { font-family: 'Cascadia Code', Consolas, monospace; }
  pre  { background: var(--vscode-textBlockQuote-background); padding: 16px;
         border-radius: 6px; overflow-x: auto; margin: 12px 0; font-size: 0.9em; }
  code { background: var(--vscode-textBlockQuote-background);
         padding: 2px 6px; border-radius: 3px; font-size: 0.9em; }
  a { color: var(--vscode-textLink-foreground); }
  ul { padding-left: 20px; }
  li { margin: 4px 0; }
  table { border-collapse: collapse; width: 100%; margin: 10px 0; }
  td, th { border: 1px solid var(--vscode-editorGroup-border); padding: 7px 12px; text-align: left; }
  th { background: var(--vscode-textBlockQuote-background); font-size: 0.9em; }
  kbd { background: var(--vscode-keybindingLabel-background);
        border: 1px solid var(--vscode-keybindingLabel-border);
        padding: 2px 8px; border-radius: 4px; font-size: 0.88em; }
  .badge { display: inline-block; background: var(--vscode-badge-background);
           color: var(--vscode-badge-foreground); padding: 2px 10px;
           border-radius: 12px; font-size: 0.8em; margin: 2px; }
  .install-box, .ready-box { border: 1px solid var(--vscode-editorGroup-border);
    border-radius: 8px; padding: 20px 24px; margin: 20px 0;
    background: var(--vscode-editorWidget-background); }
  .ready-box { display: flex; align-items: center; gap: 16px; }
  .ready-icon { font-size: 2em; color: var(--vscode-testing-iconPassed); }
  .status-box { display: flex; align-items: center; gap: 14px;
    padding: 12px 16px; border-radius: 6px; margin-bottom: 18px;
    border: 1px solid var(--vscode-editorGroup-border); }
  .status-box.ready     { border-color: var(--vscode-testing-iconPassed); }
  .status-box.not-ready { border-color: var(--vscode-inputValidation-warningBorder); }
  .status-icon { font-size: 1.6em; flex-shrink: 0; }
  .status-box.ready     .status-icon { color: var(--vscode-testing-iconPassed); }
  .status-box.not-ready .status-icon { color: var(--vscode-inputValidation-warningForeground); }
  .status-text { font-size: 0.95em; }
  .status-path { font-size: 0.82em; color: var(--vscode-descriptionForeground); margin-top: 2px;
    word-break: break-all; }
  .info-box { background: var(--vscode-textBlockQuote-background);
    border: 1px solid var(--vscode-editorGroup-border);
    border-radius: 4px; padding: 10px 14px; margin-top: 12px; font-size: 0.9em; }
  .btn-primary { display: inline-block; margin: 14px 0 8px;
    background: var(--vscode-button-background);
    color: var(--vscode-button-foreground);
    border: none; border-radius: 4px; padding: 10px 24px;
    font-size: 1em; cursor: pointer; font-weight: 600; }
  .btn-primary:hover { background: var(--vscode-button-hoverBackground); }
  .btn-primary:disabled { opacity: 0.6; cursor: not-allowed; }
  .btn-secondary { display: inline-block; margin: 6px 8px 6px 0;
    background: var(--vscode-button-secondaryBackground);
    color: var(--vscode-button-secondaryForeground);
    border: none; border-radius: 4px; padding: 8px 18px;
    font-size: 0.95em; cursor: pointer; }
  .progress-bar-bg { background: var(--vscode-editorGroup-border);
    border-radius: 4px; height: 8px; margin: 14px 0 6px; }
  .progress-bar-fill { background: var(--vscode-progressBar-background);
    border-radius: 4px; height: 8px; width: 0%; transition: width 0.3s; }
  .progress-text { font-size: 0.88em; color: var(--vscode-descriptionForeground); }
  .error-box { background: var(--vscode-inputValidation-errorBackground);
    border: 1px solid var(--vscode-inputValidation-errorBorder);
    border-radius: 4px; padding: 10px 14px; margin-top: 12px; font-size: 0.9em; }
  .note { font-size: 0.85em; color: var(--vscode-descriptionForeground);
    font-style: italic; margin: 6px 0; }
  .actions { margin-top: 20px; }
</style>
</head>
<body>

<h1>uz++ 2.1</h1>
<p class="subtitle">O'zbek tilidagi dasturlash tili — C++ ning barcha kuchi, o'z tilida.</p>

${installSection}

<h2>Birinchi dastur</h2>
<pre>ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish &lt;&lt; "Salom, Dunyo!" &lt;&lt; qator_oxiri;
    qaytarish 0;
}</pre>

<h2>Asosiy buyruqlar</h2>
<table>
  <tr><th>Tugma / Buyruq</th><th>Amal</th></tr>
  <tr><td><kbd>F5</kbd></td><td>Faylni ishga tushirish (run)</td></tr>
  <tr><td><kbd>Ctrl+F5</kbd></td><td>Faylni qurish (build)</td></tr>
  <tr><td>Ctrl+Shift+P → <code>uz++ yangi loyiha</code></td><td>Yangi loyiha yaratish</td></tr>
  <tr><td>Ctrl+Shift+P → <code>uz++ C++ kodi</code></td><td>Generatsiya qilingan C++ ni ko'rish</td></tr>
  <tr><td>Ctrl+Shift+P → <code>uz++ komponentlar holati</code></td><td>O'rnatilgan komponentlarni tekshirish</td></tr>
</table>

<h2>Kalit so'zlar</h2>
<p>
  <span class="badge">butun</span> <span class="badge">haqiqiy</span>
  <span class="badge">matn</span> <span class="badge">mantiqiy</span>
  <span class="badge">ozgaruvchan</span> <span class="badge">ozgarmas</span>
  <span class="badge">agar</span> <span class="badge">aks holda</span>
  <span class="badge">uchun</span> <span class="badge">holda</span>
  <span class="badge">qaytarish</span> <span class="badge">sinf</span>
  <span class="badge">yozish</span> <span class="badge">o'qish</span>
  <span class="badge">urinish</span> <span class="badge">ushlash</span>
</p>

<div class="actions">
  <button class="btn-secondary" onclick="vscode.postMessage({command:'newProject'})">Yangi loyiha yaratish</button>
  <button class="btn-secondary" onclick="vscode.postMessage({command:'openDocs'})">Qo'llanma (Docs)</button>
  <button class="btn-secondary" onclick="vscode.postMessage({command:'openReleases'})">Releases sahifasi</button>
</div>

<script>
  const vscode = acquireVsCodeApi();

  function startInstall() {
    const btn = document.getElementById('btn-install');
    btn.disabled = true;
    btn.textContent = "Yuklanmoqda...";
    document.getElementById('progress-area').style.display = 'block';
    document.getElementById('error-area').style.display = 'none';
    document.getElementById('info-area').style.display = 'none';
    vscode.postMessage({ command: 'runInstaller' });
  }

  function showText(id, text) {
    const el = document.getElementById(id);
    el.style.display = 'block';
    el.textContent = text;
  }

  window.addEventListener('message', (event) => {
    const msg = event.data;
    const btn = document.getElementById('btn-install');
    if (msg.type === 'progress') {
      document.getElementById('progress-text').textContent = msg.text;
      document.getElementById('progress-bar').style.width = Math.round((msg.fraction ?? 0) * 100) + '%';
    } else if (msg.type === 'info') {
      showText('info-area', msg.text);
      document.getElementById('progress-bar').style.width = '100%';
      document.getElementById('progress-text').textContent = 'Tayyor.';
      if (btn) { btn.disabled = false; btn.textContent = "Qayta yuklab olish"; }
    } else if (msg.type === 'error') {
      showText('error-area', "Xato: " + msg.text);
      if (btn) { btn.disabled = false; btn.textContent = "Qayta urinish"; }
    }
  });
</script>
</body>
</html>`;
}

// ─── Activate / Deactivate ────────────────────────────────────────────────────

/** Persistent status-bar entry that always offers "Install / Reinstall uz++". */
let installStatusBarItem;
function ensureInstallStatusBarItem(context) {
    if (installStatusBarItem) return;
    installStatusBarItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 99);
    installStatusBarItem.text    = '$(cloud-download) uz++ install';
    installStatusBarItem.tooltip = "uz++ ni o'rnatish yoki qayta o'rnatish";
    installStatusBarItem.command = 'uzpp.openWelcome';
    installStatusBarItem.show();
    context.subscriptions.push(installStatusBarItem);
}

function activate(context) {
    const compilerPath = findCompilerPath(context);

    // LSP server (the compiler itself handles LSP via "uzpp lsp")
    const serverOptions = {
        run:   { command: compilerPath, args: ['lsp'], transport: TransportKind.stdio },
        debug: { command: compilerPath, args: ['lsp'], transport: TransportKind.stdio }
    };
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'uzpp' }],
        synchronize: { fileEvents: vscode.workspace.createFileSystemWatcher('**/*.uzpp') },
        outputChannelName: 'uz++ Language Server'
    };

    // Resolve "is compiler usable?" beyond the legacy globalStorage check.
    // We're happy if it's in globalStorage, OR on PATH, OR a hard-coded
    // setting points at a real file.
    const compilerUsable =
        cm.checkComponents(context).compilerOk ||
        fs.existsSync(compilerPath) ||
        findCompilerOnPath() !== null;

    if (compilerUsable) {
        client = new LanguageClient('uzpp-lsp', 'uz++ Language Server', serverOptions, clientOptions);
        client.start().catch(() => {/* LSP optional */});
    } else {
        // Compiler is genuinely missing. Don't fail silently — surface the
        // installer flow as a dismissible notification.
        offerCompilerInstall(context).catch(() => {});
    }

    // Always-visible install entrypoint in the status bar — independent of
    // whether the compiler is currently detected, so users can re-install /
    // upgrade at any moment.
    ensureInstallStatusBarItem(context);

    // Status bar
    statusBarItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
    context.subscriptions.push(statusBarItem);
    updateStatusBar(context);

    // Register all commands
    [
        vscode.commands.registerCommand('uzpp.runFile',              () => cmdRunFile(context)),
        vscode.commands.registerCommand('uzpp.buildFile',            () => cmdBuildFile(context)),
        vscode.commands.registerCommand('uzpp.formatFile',           () => cmdFormatFile(context)),
        vscode.commands.registerCommand('uzpp.showGeneratedCpp',     () => cmdShowGeneratedCpp(context)),
        vscode.commands.registerCommand('uzpp.newProject',           () => cmdNewProject(context)),
        vscode.commands.registerCommand('uzpp.installComponents',    () => cmdInstallComponents(context)),
        vscode.commands.registerCommand('uzpp.checkComponents',      () => cmdCheckComponents(context)),
        vscode.commands.registerCommand('uzpp.uninstallComponents',  () => cmdUninstallComponents(context)),
        vscode.commands.registerCommand('uzpp.installFromFile',      () => cmdInstallFromFile(context)),
        vscode.commands.registerCommand('uzpp.updateCompiler',       () => cmdUpdateCompiler(context)),
        vscode.commands.registerCommand('uzpp.openWelcome',          () => showWelcome(context)),
        vscode.commands.registerCommand('uzpp.restartServer', async () => {
            if (client) { await client.stop(); client.start().catch(() => {}); }
            vscode.window.showInformationMessage('uz++ LSP qayta ishga tushirildi.');
        }),
    ].forEach(c => context.subscriptions.push(c));

    // Show welcome on first run
    showWelcomeIfFirstRun(context);
}

async function deactivate() {
    if (client) await client.stop();
}

module.exports = { activate, deactivate };
