@echo off
setlocal
cd /d "%~dp0"

if not exist "externals\glew\include\GL\glew.h" goto missing_libraries
if not exist "externals\glfw\include\GLFW\glfw3.h" goto missing_libraries
if not exist "externals\glm\glm\glm.hpp" goto missing_libraries

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto missing_compiler
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
if not defined VSROOT goto missing_compiler
call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 goto missing_compiler

set "GLFW_LIB=lib-vc2022"
if "%VisualStudioVersion:~0,2%"=="18" set "GLFW_LIB=lib-vc2026"
if not exist "externals\glfw\%GLFW_LIB%\glfw3dll.lib" goto missing_libraries
if not exist "bin" mkdir "bin"
echo Building with MSVC %VisualStudioVersion% / x64...
cl /nologo /EHsc /std:c++17 /utf-8 /DGLFW_DLL main.cpp /I"externals\glew\include" /I"externals\glfw\include" /I"externals\glm" /Fo:bin\main.obj /Fe:bin\OpenGLStudy.exe /link /LIBPATH:"externals\glew\lib\Release\x64" /LIBPATH:"externals\glfw\%GLFW_LIB%" opengl32.lib glew32.lib glfw3dll.lib
if errorlevel 1 exit /b 1
copy /Y "externals\glew\bin\Release\x64\glew32.dll" "bin\" >nul
if errorlevel 1 exit /b 1
copy /Y "externals\glfw\%GLFW_LIB%\glfw3.dll" "bin\" >nul
if errorlevel 1 exit /b 1
echo Build OK. Run: .\run.bat
exit /b 0

:missing_libraries
echo ERROR: externals is missing or nested incorrectly.
echo Expected: externals\glew, externals\glfw, externals\glm
echo See README.md for the download link.
exit /b 1

:missing_compiler
echo ERROR: MSVC C++ x64 tools were not found.
echo Install Visual Studio 2022/2026 or Build Tools with Desktop development with C++.
echo See README.md for instructions.
exit /b 1
