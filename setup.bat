@echo off
chcp 65001 >nul
color 0A
echo ==============================================================
echo        uz++ (C++23) Avtomatik O'rnatuvchi (VS Code uchun)
echo ==============================================================
echo.

set "UZPP_DIR=%~dp0"
:: Удаляем последний слеш из пути, если он есть
if "%UZPP_DIR:~-1%"=="\" set "UZPP_DIR=%UZPP_DIR:~0,-1%"

echo [1/3] uzpp.exe kengaytma papkasiga nusxalanmoqda...
if not exist "%UZPP_DIR%\vscode-uzpp\bin" mkdir "%UZPP_DIR%\vscode-uzpp\bin"
copy /Y "%UZPP_DIR%\build\uzpp.exe" "%UZPP_DIR%\vscode-uzpp\bin\uzpp.exe" >nul
echo     -> uzpp.exe nusxalandi.

echo.
echo [2/3] VS Code uchun uz++ kengaytmasi paketlanmoqda va o'rnatilmoqda...
cd /d "%UZPP_DIR%\vscode-uzpp"
call npm install
call npx vsce package -o uzpp-lang.vsix
call code --install-extension uzpp-lang.vsix --force
echo     -> Kengaytma o'rnatildi.

echo.
echo [3/3] Sinov loyihasi (Salom Dunyo) yaratilmoqda...
set "PROJECT_DIR=%USERPROFILE%\Desktop\MeningUzppLoyiham"
if not exist "%PROJECT_DIR%" mkdir "%PROJECT_DIR%"
if not exist "%PROJECT_DIR%\src" mkdir "%PROJECT_DIR%\src"

echo ulash "matn"; > "%PROJECT_DIR%\src\asosiy.uzpp"
echo. >> "%PROJECT_DIR%\src\asosiy.uzpp"
echo funksiya asosiy() { >> "%PROJECT_DIR%\src\asosiy.uzpp"
echo     yozish ^<^< "Salom, Dunyo!" ^<^< qator_oxiri; >> "%PROJECT_DIR%\src\asosiy.uzpp"
echo     qaytarish 0; >> "%PROJECT_DIR%\src\asosiy.uzpp"
echo } >> "%PROJECT_DIR%\src\asosiy.uzpp"

echo nom = "MeningUzppLoyiham" > "%PROJECT_DIR%\uzpp.toml"
echo versiya = "1.0.0" >> "%PROJECT_DIR%\uzpp.toml"

echo.
echo ==============================================================
echo TAYYOR! Dasturlashni boshlashingiz mumkin.
echo ==============================================================
echo Dasturni ishga tushirish uchun VS Code terminalida quyidagicha yozing:
echo    uzpp ishga-tushirish
echo.
echo Davom etish uchun ixtiyoriy tugmani bosing va VS Code ochiladi...
pause >nul

cd /d "%PROJECT_DIR%"
code .