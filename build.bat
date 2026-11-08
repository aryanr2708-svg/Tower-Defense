@echo off
rem ============================================================
rem  Build script for the tower defense game (Windows + raylib)
rem
rem    build.bat        -> compile only
rem    build.bat run    -> compile, then start the game
rem
rem  If you installed raylib with the official installer, the
rem  defaults below already match. Otherwise edit the two paths.
rem ============================================================
setlocal

set RAYLIB_DIR=C:\raylib\raylib
set COMPILER_DIR=C:\raylib\w64devkit\bin

if exist "%COMPILER_DIR%\gcc.exe" set "PATH=%COMPILER_DIR%;%PATH%"

where gcc >nul 2>nul
if errorlevel 1 goto nogcc

set INC=
set LIB=
if exist "%RAYLIB_DIR%\src\raylib.h" set INC=-I"%RAYLIB_DIR%\src"
if exist "%RAYLIB_DIR%\src\libraylib.a" set LIB=-L"%RAYLIB_DIR%\src"

echo Building...
gcc -std=c99 -Wall -Wextra -O2 main.c game.c enemy.c tower.c projectile.c maps.c path.c ui.c -o towerdef.exe %INC% %LIB% -lraylib -lopengl32 -lgdi32 -lwinmm
if errorlevel 1 goto failed

echo Build OK: towerdef.exe
if /i "%1"=="run" towerdef.exe
goto end

:nogcc
echo [error] gcc was not found.
echo Install raylib with its Windows installer (it bundles w64devkit),
echo or edit COMPILER_DIR and RAYLIB_DIR at the top of build.bat.
pause
goto end

:failed
echo.
echo [error] Build failed. Copy the messages above and send them to me.
pause

:end
endlocal
