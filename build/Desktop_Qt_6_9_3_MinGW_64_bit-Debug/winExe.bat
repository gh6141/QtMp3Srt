@echo off
REM 引数が無ければ終了
if "%1"=="" (
    echo Usage: %0 YourApp.exe
    exit /b
)

REM バッチファイルと同じディレクトリのexeをフルパスで取得
set EXE_FILE=%~dp0%1

"C:\Qt\6.9.3\mingw_64\bin\windeployqt.exe" "%EXE_FILE%"
pause