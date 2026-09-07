#!/usr/bin/env python3
"""LSP ramkasi (framing) va diagnostikasini tekshiradi.

Nega kerak: Windows da `stdout` matn rejimida bo'lsa, `std::cout` har bir
`\\n` ni `\\r\\n` ga aylantiradi va LSP sarlavha ajratuvchisi `\\r\\n\\r\\n`
o'rniga `\\r\\r\\n\\r\\r\\n` bo'lib chiqadi. Bunday oqimda `\\r\\n\\r\\n`
ketma-ketligi umuman uchramaydi — hech bir mijoz sarlavhani ajrata olmaydi.
Oqibati: VS Code da diagnostika, hover, avtoto'ldirish jim o'ladi, xato esa
hech qayerda ko'rinmaydi. Shuning uchun bu testda ramka BAYT darajasida
tekshiriladi.

Foydalanish:  python tests/lsp_ramka_tekshir.py build/uzpp.exe
"""
import json
import os
import subprocess
import sys
import threading
import time

UZPP = sys.argv[1] if len(sys.argv) > 1 else 'build/uzpp.exe'

TOZA = '''ulash "uzpp_runtime.hpp"

shartnoma Hisobotli {
    matn hisobot();
}

sinf Kitob : Hisobotli {
yopiq:
    matn nomi_;
ochiq:
    Kitob(matn nomi) : nomi_(nomi) {}
    matn nomi() { qaytarish nomi_; }
    matn hisobot() ustidan_yozish { qaytarish "Kitob: " + nomi_; }
};

butun sanash(vektor<butun>& v, funksiya<mantiqiy(butun)> shart) {
    butun n = 0;
    uchun (o'zgaruvchan x : v) { agar (shart(x)) { n = n + 1; } }
    qaytarish n;
}

butun asosiy() {
    Kitob k("Alpomish");
    yozish << k.hisobot() << qator_oxiri;
    qaytarish 0;
}
'''

XATOLI = '''ulash "uzpp_runtime.hpp"

butun asosiy() {
    butun x = "matn";
    qaytarish 0;
}
'''


def frame(obj):
    body = json.dumps(obj).encode('utf-8')
    return b'Content-Length: ' + str(len(body)).encode() + b'\r\n\r\n' + body


def parse_frames(raw):
    out, i = [], 0
    while True:
        sep = raw.find(b'\r\n\r\n', i)
        if sep < 0:
            break
        header = raw[i:sep].decode('ascii', 'replace')
        length = 0
        for part in header.split('\r\n'):
            if part.lower().startswith('content-length:'):
                length = int(part.split(':', 1)[1].strip())
        start = sep + 4
        if len(raw) < start + length:
            break
        try:
            out.append(json.loads(raw[start:start + length].decode('utf-8', 'replace')))
        except ValueError:
            pass
        i = start + length
    return out


def talk(code):
    proc = subprocess.Popen([UZPP, 'lsp'], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    raw = bytearray()

    def reader():
        while True:
            try:
                chunk = os.read(proc.stdout.fileno(), 65536)
            except OSError:
                return
            if not chunk:
                return
            raw.extend(chunk)

    threading.Thread(target=reader, daemon=True).start()
    proc.stdin.write(frame({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                            "params": {"processId": 1, "rootUri": None, "capabilities": {}}}))
    proc.stdin.flush()
    time.sleep(1.5)
    proc.stdin.write(frame({"jsonrpc": "2.0", "method": "initialized", "params": {}}))
    proc.stdin.write(frame({"jsonrpc": "2.0", "method": "textDocument/didOpen",
                            "params": {"textDocument": {"uri": "file:///tmp/x.uzpp",
                                                        "languageId": "uzpp", "version": 1,
                                                        "text": code}}}))
    proc.stdin.flush()
    time.sleep(4.0)
    proc.kill()
    return bytes(raw)


failures = []


def check(cond, msg):
    print(('  OK   ' if cond else '  XATO ') + msg)
    if not cond:
        failures.append(msg)


print('=' * 45)
print(' LSP ramkasi va diagnostikasi')
print('=' * 45)

raw = talk(TOZA)

check(b'\r\n\r\n' in raw, 'sarlavha ajratuvchisi to\'g\'ri (\\r\\n\\r\\n bor)')
check(b'\r\r\n' not in raw, 'matn rejimi buzilishi yo\'q (\\r\\r\\n uchramaydi)')

msgs = parse_frames(raw)
init = next((m for m in msgs if m.get('id') == 1), None)
check(init is not None, 'initialize javob berdi')

if init:
    caps = init.get('result', {}).get('capabilities', {})
    for name in ('hoverProvider', 'completionProvider', 'definitionProvider',
                 'documentSymbolProvider', 'semanticTokensProvider'):
        check(name in caps, 'imkoniyat e\'lon qilindi: ' + name)

diag_msgs = [m for m in msgs if m.get('method') == 'textDocument/publishDiagnostics']
diags = [d for m in diag_msgs for d in m.get('params', {}).get('diagnostics', [])]
check(len(diag_msgs) > 0, 'publishDiagnostics yuborildi')
# Diqqat: xabar umuman kelmagan bo'lsa, "0 ta diagnostika" ni muvaffaqiyat deb
# hisoblab bo'lmaydi — bu aynan jim o'lgan serverning alomati.
check(len(diag_msgs) > 0 and len(diags) == 0,
      'toza kodda diagnostika yo\'q (%d ta topildi)' % len(diags))
for d in diags:
    print('         -> %d:%d %s' % (d['range']['start']['line'] + 1,
                                    d['range']['start']['character'] + 1, d['message']))

# Nazorat: diagnostika umuman tirikmi?
msgs2 = parse_frames(talk(XATOLI))
diags2 = [d for m in msgs2 if m.get('method') == 'textDocument/publishDiagnostics'
          for d in m.get('params', {}).get('diagnostics', [])]
check(len(diags2) > 0, 'ataylab xatoli kodda diagnostika chiqdi (%d ta)' % len(diags2))

print('=' * 45)
if failures:
    print(' YIQILDI: %d ta' % len(failures))
    sys.exit(1)
print(' Hammasi joyida')
sys.exit(0)
