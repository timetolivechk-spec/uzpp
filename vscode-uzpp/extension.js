'use strict';

const vscode = require('vscode');
const path = require('path');
const fs = require('fs');
const { execSync } = require('child_process');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

/** @type {LanguageClient | undefined} */
let client;
/** @type {vscode.StatusBarItem} */
let statusBarItem;

// ─── Compiler Discovery ───────────────────────────────────────────────────────

function findCompilerPath(context) {
    const embeddedPath = context.asAbsolutePath(
        path.join('bin', process.platform === 'win32' ? 'uzpp.exe' : 'uzpp')
    );
    if (fs.existsSync(embeddedPath)) return embeddedPath;

    const config = vscode.workspace.getConfiguration('uzpp');
    const configured = config.get('compilerPath');
    if (configured && configured.length > 0) return configured;

    const folders = vscode.workspace.workspaceFolders;
    if (folders && folders.length > 0) {
        const candidate = path.join(
            folders[0].uri.fsPath,
            process.platform === 'win32' ? 'uzpp.exe' : 'uzpp'
        );
        if (fs.existsSync(candidate)) return candidate;
    }

    return process.platform === 'win32' ? 'uzpp.exe' : 'uzpp';
}

function getCompilerVersion(compilerPath) {
    try {
        const out = execSync(`"${compilerPath}" --version`, { timeout: 5000 }).toString().trim();
        const m = out.match(/\d+\.\d+\.\d+/);
        return m ? m[0] : out.split('\n')[0];
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

// ─── Commands ─────────────────────────────────────────────────────────────────

async function cmdRunFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();
    const compiler = findCompilerPath(context);
    const file = editor.document.uri.fsPath;
    const term = getTerminal();
    term.show(true);
    term.sendText(`"${compiler}" ishga-tushirish "${file}"`);
}

async function cmdBuildFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();
    const compiler = findCompilerPath(context);
    const file = editor.document.uri.fsPath;
    const term = getTerminal();
    term.show(true);
    term.sendText(`"${compiler}" qurish "${file}"`);
}

async function cmdFormatFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();
    const compiler = findCompilerPath(context);
    const file = editor.document.uri.fsPath;
    try {
        execSync(`"${compiler}" format "${file}"`, { timeout: 10000 });
        vscode.window.showInformationMessage('Fayl formatlandi.');
    } catch (e) {
        vscode.window.showErrorMessage(`Format xatosi: ${e.message}`);
    }
}

async function cmdShowGeneratedCpp(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) { vscode.window.showWarningMessage('Fayl ochilmagan.'); return; }
    await editor.document.save();

    const compiler = findCompilerPath(context);
    const file = editor.document.uri.fsPath;
    const baseName = path.basename(file, '.uzpp');
    const dir = path.dirname(file);

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: 'C++ kodi generatsiya qilinmoqda...' },
        async () => {
            try {
                execSync(`"${compiler}" qurish "${file}"`, { timeout: 30000 });
            } catch (e) {
                vscode.window.showErrorMessage(`Build xatosi: ${e.stderr ? e.stderr.toString() : e.message}`);
                return;
            }

            // Look for generated .cpp in common locations
            const candidates = [
                path.join(dir, 'build', `${baseName}.generated.cpp`),
                path.join(dir, `${baseName}.generated.cpp`),
                path.join(dir, '..', 'build', `${baseName}.generated.cpp`),
            ];

            for (const candidate of candidates) {
                if (fs.existsSync(candidate)) {
                    const doc = await vscode.workspace.openTextDocument(candidate);
                    await vscode.window.showTextDocument(doc, vscode.ViewColumn.Beside);
                    return;
                }
            }

            vscode.window.showWarningMessage(
                `Generatsiya qilingan C++ fayl topilmadi. build/ papkasini tekshiring.`
            );
        }
    );
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
        ``,
        `[kompilyator]`,
        `standart = "c++23"`,
        ``
    ].join('\n'));

    await vscode.commands.executeCommand(
        'vscode.openFolder',
        vscode.Uri.file(projectDir),
        { forceNewWindow: false }
    );
}

async function cmdUpdateCompiler(context) {
    const platform = process.platform;
    const arch = process.arch;

    const assetName = platform === 'win32' ? 'uzpp-windows-x64.exe'
        : platform === 'darwin' ? (arch === 'arm64' ? 'uzpp-macos-arm64' : 'uzpp-macos-x64')
        : 'uzpp-linux-x64';

    const answer = await vscode.window.showInformationMessage(
        `uz++ kompilyatorini yangilash (${assetName})?`,
        'Ha', "Yo'q"
    );
    if (answer !== 'Ha') return;

    const binDir = context.asAbsolutePath('bin');
    const destName = platform === 'win32' ? 'uzpp.exe' : 'uzpp';
    const destPath = path.join(binDir, destName);

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: 'Kompilyator yuklanmoqda...' },
        async () => {
            try {
                if (!fs.existsSync(binDir)) fs.mkdirSync(binDir, { recursive: true });
                const url = `https://github.com/timetolivechk-spec/uz-plus-plus/releases/latest/download/${assetName}`;
                const cmd = platform === 'win32'
                    ? `curl -L --output "${destPath}" "${url}"`
                    : `curl -L --output "${destPath}" "${url}" && chmod +x "${destPath}"`;
                execSync(cmd, { timeout: 120000 });
                updateStatusBar(context);
                vscode.window.showInformationMessage('uz++ kompilyatori muvaffaqiyatli yangilandi!');
            } catch (e) {
                vscode.window.showErrorMessage(`Yuklash xatosi: ${e.message}`);
            }
        }
    );
}

// ─── Status Bar ───────────────────────────────────────────────────────────────

function updateStatusBar(context) {
    const compiler = findCompilerPath(context);
    const version = getCompilerVersion(compiler);
    statusBarItem.text = version ? `$(bracket) uz++ v${version}` : '$(bracket) uz++';
    statusBarItem.tooltip = `uz++ kompilyatori: ${compiler}\nBosish: faylni ishga tushirish (F5)`;
    statusBarItem.command = 'uzpp.runFile';
    statusBarItem.show();
}

// ─── Welcome Screen ───────────────────────────────────────────────────────────

async function showWelcomeIfFirstRun(context) {
    const key = 'uzpp.welcomeShown.v2';
    if (context.globalState.get(key)) return;
    await context.globalState.update(key, true);

    const panel = vscode.window.createWebviewPanel(
        'uzppWelcome',
        "uz++ ga Xush Kelibsiz!",
        vscode.ViewColumn.One,
        { enableScripts: false }
    );

    panel.webview.html = `<!DOCTYPE html>
<html lang="uz">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>uz++ ga Xush Kelibsiz</title>
<style>
  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
         max-width: 720px; margin: 40px auto; padding: 0 24px;
         color: var(--vscode-foreground);
         background: var(--vscode-editor-background); line-height: 1.7; }
  h1   { color: var(--vscode-textLink-foreground); font-size: 2em; margin-bottom: 4px; }
  h2   { color: var(--vscode-textLink-foreground); margin-top: 2em; }
  code { background: var(--vscode-textBlockQuote-background); padding: 2px 6px;
         border-radius: 3px; font-family: 'Cascadia Code', Consolas, monospace; }
  pre  { background: var(--vscode-textBlockQuote-background); padding: 16px;
         border-radius: 6px; overflow-x: auto; font-family: 'Cascadia Code', Consolas, monospace; }
  .badge { display: inline-block; background: var(--vscode-badge-background);
           color: var(--vscode-badge-foreground); padding: 2px 10px;
           border-radius: 12px; font-size: 0.82em; margin: 3px 2px; }
  kbd  { background: var(--vscode-keybindingLabel-background);
         border: 1px solid var(--vscode-keybindingLabel-border);
         padding: 2px 7px; border-radius: 4px; font-size: 0.88em; }
  table { border-collapse: collapse; width: 100%; margin-top: 8px; }
  td, th { border: 1px solid var(--vscode-editorGroup-border); padding: 8px 12px; text-align: left; }
  th { background: var(--vscode-textBlockQuote-background); }
  a  { color: var(--vscode-textLink-foreground); }
  .subtitle { color: var(--vscode-descriptionForeground); font-size: 1.05em; }
</style>
</head>
<body>
<h1>uz++ 2.1</h1>
<p class="subtitle">O'zbek tilidagi dasturlash tili — C++ ning barcha kuchi, o'z tilida.</p>

<h2>Birinchi dastur</h2>
<pre>ulash "uzpp_runtime.hpp"

butun asosiy() {
    yozish &lt;&lt; "Salom, Dunyo!" &lt;&lt; qator_oxiri;
    qaytarish 0;
}</pre>

<h2>Asosiy buyruqlar</h2>
<table>
  <tr><th>Tugma</th><th>Amal</th></tr>
  <tr><td><kbd>F5</kbd></td><td>Faylni ishga tushirish</td></tr>
  <tr><td><kbd>Ctrl+F5</kbd></td><td>Faylni faqat qurish (build)</td></tr>
  <tr><td><kbd>Ctrl+Shift+P</kbd> → <code>uz++ yangi loyiha</code></td><td>Yangi loyiha yaratish</td></tr>
  <tr><td><kbd>Ctrl+Shift+P</kbd> → <code>uz++ C++ kodi</code></td><td>Generatsiya qilingan C++ ni ko'rish</td></tr>
  <tr><td><kbd>Ctrl+Shift+P</kbd> → <code>uz++ kompilyatorni yangilash</code></td><td>Eng yangi versiyani yuklab olish</td></tr>
</table>

<h2>Kalit so'zlar</h2>
<p>
  <span class="badge">butun</span> <span class="badge">haqiqiy</span>
  <span class="badge">matn</span> <span class="badge">mantiqiy</span>
  <span class="badge">belgilangan</span> <span class="badge">ozgaruvchan</span>
  <span class="badge">ozgarmas</span> <span class="badge">agar</span>
  <span class="badge">aks holda</span> <span class="badge">uchun</span>
  <span class="badge">holda</span> <span class="badge">qaytarish</span>
  <span class="badge">sinf</span> <span class="badge">tuzilma</span>
  <span class="badge">yozish</span> <span class="badge">o'qish</span>
  <span class="badge">urinish</span> <span class="badge">ushlash</span>
</p>

<h2>Resurslar</h2>
<ul>
  <li><a href="https://github.com/timetolivechk-spec/uz-plus-plus">GitHub repository</a></li>
  <li><a href="https://github.com/timetolivechk-spec/uz-plus-plus/blob/main/docs/getting-started.md">Boshlash qo'llanmasi</a></li>
  <li><a href="https://github.com/timetolivechk-spec/uz-plus-plus/tree/main/examples">Misollar (examples/)</a></li>
</ul>
</body>
</html>`;
}

// ─── Activate / Deactivate ────────────────────────────────────────────────────

function activate(context) {
    const compilerPath = findCompilerPath(context);

    const serverOptions = {
        run:   { command: compilerPath, args: ['lsp'], transport: TransportKind.stdio },
        debug: { command: compilerPath, args: ['lsp'], transport: TransportKind.stdio }
    };
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'uzpp' }],
        synchronize: { fileEvents: vscode.workspace.createFileSystemWatcher('**/*.uzpp') },
        outputChannelName: 'uz++ Language Server'
    };

    client = new LanguageClient('uzpp-lsp', 'uz++ Language Server', serverOptions, clientOptions);
    client.start();

    // Status bar
    statusBarItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
    context.subscriptions.push(statusBarItem);
    updateStatusBar(context);

    // Commands
    [
        vscode.commands.registerCommand('uzpp.runFile',          () => cmdRunFile(context)),
        vscode.commands.registerCommand('uzpp.buildFile',        () => cmdBuildFile(context)),
        vscode.commands.registerCommand('uzpp.formatFile',       () => cmdFormatFile(context)),
        vscode.commands.registerCommand('uzpp.showGeneratedCpp', () => cmdShowGeneratedCpp(context)),
        vscode.commands.registerCommand('uzpp.newProject',       () => cmdNewProject(context)),
        vscode.commands.registerCommand('uzpp.updateCompiler',   () => cmdUpdateCompiler(context)),
        vscode.commands.registerCommand('uzpp.restartServer',    async () => {
            if (client) { await client.stop(); client.start(); }
            vscode.window.showInformationMessage('uz++ LSP qayta ishga tushirildi.');
        }),
    ].forEach(c => context.subscriptions.push(c));

    showWelcomeIfFirstRun(context);
}

async function deactivate() {
    if (client) await client.stop();
}

module.exports = { activate, deactivate };
