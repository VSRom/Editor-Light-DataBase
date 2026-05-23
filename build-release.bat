@echo off
setlocal EnableDelayedExpansion

:: Путь к твоему Qt Kit
set "QT_ROOT=F:\QT\6.11.0\msvc2022_64"

echo ========================================
echo   Сборка Editor-Light-DataBase
echo ========================================
echo.

cd /d "%~dp0"

echo [1/5] Очистка старых файлов...
if exist "out" rmdir /s /q "out"
if exist "build" rmdir /s /q "build"

echo [2/5] Конфигурация CMake...
cmake --preset=Qt-Release
if !errorlevel! neq 0 ( echo [ОШИБКА] cmake --preset & pause & exit /b 1 )

echo [3/5] Компиляция (копирует драйверы и внешние DLL)...
cmake --build "out/build/release" --config Release
if !errorlevel! neq 0 ( echo [ОШИБКА] cmake --build & pause & exit /b 1 )

echo [4/5] Сборка библиотек Qt (windeployqt)...
cd /d "out/build/release"
:: ИСПРАВЛЕНО: --no-compiler -> --no-compiler-runtime (синтаксис Qt 6)
call "%QT_ROOT%\bin\windeployqt.exe" Editor-Light-DataBase.exe --no-compiler-runtime --no-translations
if !errorlevel! neq 0 ( echo [ОШИБКА] windeployqt & pause & exit /b 1 )

echo [5/5] Создание готовой папки dist...
cd /d "%~dp0"
if exist "dist" rmdir /s /q "dist"
mkdir "dist\Editor-Light-DataBase"
xcopy /E /Y /I "out\build\release\*" "dist\Editor-Light-DataBase\" >nul

echo.
echo ========================================
echo   СБОРКА ЗАВЕРШЕНА!
echo   Запускай: dist\Editor-Light-DataBase\Editor-Light-DataBase.exe
echo ========================================
pause
endlocal