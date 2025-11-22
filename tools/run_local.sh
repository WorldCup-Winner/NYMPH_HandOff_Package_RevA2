#!/usr/bin/env bash
set -euo pipefail

# NYMPH 1.1 Local Daemon Runner (Stub Mode)
# Starts the nymph-acceld daemon in local/stub mode

DAEMON_BIN="./repo/agent/nymph-acceld"
BUILD_DIR="./repo/agent/build"

echo "[run] Starting nymph-acceld (stub mode)"

# Check if daemon is built
if [ ! -f "$DAEMON_BIN" ]; then
    echo "[run] Daemon not found. Building..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure and build
    cmake ..
    cmake --build .
    
    # Copy binary to agent directory
    if [ -f "nymph-acceld" ]; then
        cp nymph-acceld ../nymph-acceld
        cd ../..
    else
        echo "[run] ERROR: Build failed"
        exit 1
    fi
fi

# Check if daemon exists
if [ ! -f "$DAEMON_BIN" ]; then
    echo "[run] ERROR: Daemon binary not found at $DAEMON_BIN"
    echo "[run] Please build the daemon first:"
    echo "  cd repo/agent && mkdir -p build && cd build && cmake .. && cmake --build ."
    exit 1
fi

echo "[run] Service will be available at: http://localhost:8443"
echo "[run] API endpoints:"
echo "  GET  /status"
echo "  GET  /fabric/verify"
echo "  POST /infer"
echo ""
echo "[run] Starting daemon..."
echo "[run] Press Ctrl+C to stop"
echo ""

# Run daemon
exec "$DAEMON_BIN"

