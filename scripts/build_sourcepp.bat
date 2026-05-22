@echo off
setlocal

if "%~1"=="" (
  echo Usage: build_sourcepp.bat ^<SolutionDir^>
  exit /b 1
)

set "CMAKE="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
  "%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" > "%TEMP%\cmake_path.txt" 2>nul
  if exist "%TEMP%\cmake_path.txt" set /p CMAKE=<"%TEMP%\cmake_path.txt"
  del "%TEMP%\cmake_path.txt" 2>nul
)
if not defined CMAKE where cmake >nul 2>&1 && set "CMAKE=cmake"
if not defined CMAKE (
  echo error: cmake not found
  exit /b 1
)

set "SOLUTION_DIR=%~1"
if not "%SOLUTION_DIR:~-1%"=="\" set "SOLUTION_DIR=%SOLUTION_DIR%\"

set "SOURCEPP_SRC=%SOLUTION_DIR%src\external\sourcepp"
set "BUILD_DIR=%SOLUTION_DIR%build\sourcepp"

if not exist "%BUILD_DIR%\CMakeCache.txt" goto :configure
if not exist "%BUILD_DIR%\ALL_BUILD.vcxproj" goto :configure
goto :build

:configure
if not exist "%SOURCEPP_SRC%\CMakeLists.txt" (
  echo error: sourcepp submodule not configured - missing %SOURCEPP_SRC%\CMakeLists.txt
  exit /b 1
)
echo configuring sourcepp...
"%CMAKE%" -S "%SOURCEPP_SRC%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release -DSOURCEPP_LIBS_START_ENABLED=OFF -DSOURCEPP_USE_VPKPP=ON
if errorlevel 1 exit /b 1

:build
echo building sourcepp (incremental)...
"%CMAKE%" --build "%BUILD_DIR%" --config Release
exit /b %ERRORLEVEL%
