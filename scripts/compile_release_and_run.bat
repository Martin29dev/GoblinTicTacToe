@echo off
setlocal

rem ============================================================
rem  Architektura MUSI zgadzac sie z raylib\lib\raylib.lib:
rem    x86 = 32-bit raylib (msvc16_32)   x64 = 64-bit raylib (msvc16_64)
rem ============================================================
set ARCH=x86

rem Przechodzimy do folderu projektu (jeden poziom wyzej niz scripts).
cd /d "%~dp0.."
if not exist obj mkdir obj

rem --- 1. Ustawiamy srodowisko MSVC dla wybranej architektury ---
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :no_vswhere

set "VSPATH="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH goto :no_msvc

call "%VSPATH%\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul
if errorlevel 1 goto :no_msvc
goto :compile

:no_vswhere
rem Brak vswhere - probujemy uzyc cl.exe z biezacej konsoli (musi byc dla tej samej architektury!)
where cl >nul 2>nul
if errorlevel 1 goto :no_msvc
echo UWAGA: uzywam cl.exe z biezacej konsoli. Jej architektura musi byc taka sama jak raylib.lib.

:compile
rem Czyscimy stare pliki, zeby nie mieszac x86 z x64.
del /q obj\*.obj >nul 2>nul

echo Kompilacja (MSVC, Release, %ARCH%)...
cl /nologo /std:c++17 /O2 /EHsc /W3 /utf-8 /MD /DNDEBUG ^
   /Iraylib\include ^
   src\*.cpp ^
   /Fo:obj\ /Fe:GoblinTicTacToe.exe ^
   /link /LIBPATH:raylib\lib raylib.lib opengl32.lib gdi32.lib winmm.lib user32.lib shell32.lib
if errorlevel 1 goto :error

echo.
echo Uruchamianie...
GoblinTicTacToe.exe
goto :end

:no_msvc
echo.
echo Nie znaleziono MSVC. Zainstaluj w Visual Studio skladnik
echo "Programowanie aplikacji klasycznych w C++".

:error
echo.
echo BLAD - kompilacja nie powiodla sie.
pause
exit /b 1

:end
endlocal
