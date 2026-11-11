@echo off
cd /d "%~dp0.."

rem Inicjalizacja MSVC w wersji 32-bitowej (x86)
where cl >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
    ) else (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
    )
)

if not exist "release" mkdir "release"
if not exist "obj" mkdir "obj"

del /q obj\* >nul 2>&1

echo Kompilacja wersji RELEASE (32-bit x86)...

cl.exe /EHsc /O2 src/main.cpp /I"raylib/include" /Fo"obj/" /Fe"release/GoblinTicTacToe.exe" /link /MACHINE:X86 /LIBPATH:"raylib/lib" raylib.lib opengl32.lib gdi32.lib user32.lib shell32.lib winmm.lib msvcrt.lib /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup

if %errorlevel% equ 0 (
    echo Kompilacja RELEASE zakonczona sukcesem!
) else (
    echo Blad kompilacji!
)

cd Release
GoblinTicTacToe.exe
cd ..

cd Scripts
pause