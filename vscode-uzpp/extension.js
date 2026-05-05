// uz++ VS Code Extension — LSP Client Entry Point
// Версия 2.0: подключает uzpm lsp как Language Server
'use strict';

const vscode = require('vscode');
const path = require('path');
const { LanguageClient, TransportKind } = require('vscode-languageclient/node');

/** @type {LanguageClient | undefined} */
let client;

/**
 * Finds the uzpm executable: embedded in extension > settings > workspace > PATH
 */
function findServerPath(context) {
    // Priority 1: Use embedded compiler from extension's bin/ folder
    const embeddedPath = context.asAbsolutePath(
        path.join('bin', process.platform === 'win32' ? 'uzpp.exe' : 'uzpp')
    );
    const fs = require('fs');
    if (fs.existsSync(embeddedPath)) {
        return embeddedPath;
    }
    
    // Priority 2: Check user settings
    const config = vscode.workspace.getConfiguration('uzpp');
    const configured = config.get('serverPath');
    if (configured && configured.length > 0) {
        return configured;
    }
    
    // Priority 3: Look next to the first workspace folder
    const workspaceFolders = vscode.workspace.workspaceFolders;
    if (workspaceFolders && workspaceFolders.length > 0) {
        const candidate = path.join(workspaceFolders[0].uri.fsPath, 'uzpp.exe');
        if (fs.existsSync(candidate)) return candidate;
    }
    
    // Priority 4: Last resort — rely on PATH
    return process.platform === 'win32' ? 'uzpp.exe' : 'uzpp';
}

function activate(context) {
    const serverExecutable = findServerPath(context);

    // Server process: uzpm lsp
    const serverOptions = {
        run:   { command: serverExecutable, args: ['lsp'], transport: TransportKind.stdio },
        debug: { command: serverExecutable, args: ['lsp'], transport: TransportKind.stdio }
    };

    // LSP client options: only activate for .uzpp files
    const clientOptions = {
        documentSelector: [{ scheme: 'file', language: 'uzpp' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.uzpp')
        },
        outputChannelName: 'uz++ Language Server'
    };

    client = new LanguageClient(
        'uzpp-lsp',
        'uz++ Language Server',
        serverOptions,
        clientOptions
    );

    // Kick off the server — it starts in background and VS Code auto-connects
    client.start();

    // Status bar item to show server state
    const statusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
    statusBar.text = '$(check) uz++';
    statusBar.tooltip = 'uz++ Language Server ishlamoqda';
    statusBar.show();
    context.subscriptions.push(statusBar);

    // Restart command — useful during development
    const restartCmd = vscode.commands.registerCommand('uzpp.restartServer', async () => {
        if (client) {
            await client.stop();
            client.start();
            vscode.window.showInformationMessage('uz++ LSP serveri qayta ishga tushirildi.');
        }
    });
    context.subscriptions.push(restartCmd);

    vscode.window.showInformationMessage('uz++ Language Server ishga tushdi. Muallif: UzAICoding');
}

async function deactivate() {
    if (client) {
        await client.stop();
    }
}

module.exports = { activate, deactivate };
