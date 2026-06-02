@echo off
setlocal

if "%~2"=="" (
  echo Usage: build_vrfextract.bat ^<SolutionDir^> ^<OutDir^>
  exit /b 1
)

set "SOLUTION_DIR=%~1"
if not "%SOLUTION_DIR:~-1%"=="\" set "SOLUTION_DIR=%SOLUTION_DIR%\"

set "OUT_DIR=%~2"
if not "%OUT_DIR:~-1%"=="\" set "OUT_DIR=%OUT_DIR%\"

set "PROJ=%SOLUTION_DIR%src\external\VrfExtract\VrfExtract.csproj"
set "STAGE=%OUT_DIR%VrfPublish"

where dotnet >nul 2>&1 || (echo error: dotnet SDK not found & exit /b 1)

dotnet publish "%PROJ%" -c Release -r win-x64 --self-contained true -o "%STAGE%"
if errorlevel 1 exit /b 1

if not exist "%STAGE%\VrfExtract.exe" (
  echo error: VrfExtract.exe not produced
  exit /b 1
)

copy /Y "%STAGE%\VrfExtract.exe" "%OUT_DIR%VrfExtract.exe" >nul
if errorlevel 1 exit /b 1

rmdir /S /Q "%STAGE%" 2>nul

del /Q "%OUT_DIR%ValveResourceFormat*.dll" 2>nul
del /Q "%OUT_DIR%SharpGLTF*.dll" 2>nul
del /Q "%OUT_DIR%SkiaSharp*.dll" 2>nul
del /Q "%OUT_DIR%TinyBCSharp*.dll" 2>nul
del /Q "%OUT_DIR%ZstdSharp*.dll" 2>nul
del /Q "%OUT_DIR%K4os*.dll" 2>nul
del /Q "%OUT_DIR%System.*.dll" 2>nul
del /Q "%OUT_DIR%Microsoft.*.dll" 2>nul
del /Q "%OUT_DIR%Newtonsoft.Json.dll" 2>nul
del /Q "%OUT_DIR%VrfExtract.pdb" 2>nul
del /Q "%OUT_DIR%libSkiaSharp.pdb" 2>nul

exit /b 0
