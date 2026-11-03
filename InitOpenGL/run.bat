@echo off
setlocal
cd /d "%~dp0"
if not exist "bin\OpenGLStudy.exe" (
    echo ERROR: Run .\build.bat first.
    exit /b 1
)
"bin\OpenGLStudy.exe"
exit /b %errorlevel%
