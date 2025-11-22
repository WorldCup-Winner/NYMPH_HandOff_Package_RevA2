@echo off
REM NYMPH 1.1 Local Daemon Runner (Stub Mode) - Windows
REM Starts the nymph-acceld daemon in local/stub mode

setlocal

set DAEMON_BIN=repo\agent\nymph-acceld.exe
set BUILD_DIR=repo\agent\build

echo [run] Starting nymph-acceld (stub mode)

REM Check if daemon is built
if not exist "%DAEMON_BIN%" (
    echo [run] Daemon not found. Building...
    
    REM Create build directory
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cd "%BUILD_DIR%"
    
    REM Configure and build
    cmake ..
    if errorlevel 1 (
        echo [run] ERROR: CMake configuration failed
        cd ..\..
        exit /b 1
    )
    
    cmake --build .
    if errorlevel 1 (
        echo [run] ERROR: Build failed
        cd ..\..
        exit /b 1
    )
    
    REM Copy binary to agent directory
    if exist "Debug\nymph-acceld.exe" (
        copy "Debug\nymph-acceld.exe" "..\nymph-acceld.exe"
    ) else if exist "Release\nymph-acceld.exe" (
        copy "Release\nymph-acceld.exe" "..\nymph-acceld.exe"
    ) else if exist "nymph-acceld.exe" (
        copy "nymph-acceld.exe" "..\nymph-acceld.exe"
    )
    
    cd ..\..
)

REM Check if daemon exists
if not exist "%DAEMON_BIN%" (
    echo [run] ERROR: Daemon binary not found at %DAEMON_BIN%
    echo [run] Please build the daemon first:
    echo   cd repo\agent ^&^& mkdir build ^&^& cd build ^&^& cmake .. ^&^& cmake --build .
    exit /b 1
)

echo [run] Service will be available at: http://localhost:8443
echo [run] API endpoints:
echo   GET  /status
echo   GET  /fabric/verify
echo   POST /infer
echo.
echo [run] Starting daemon...
echo [run] Press Ctrl+C to stop
echo.

REM Run daemon
"%DAEMON_BIN%"

endlocal
