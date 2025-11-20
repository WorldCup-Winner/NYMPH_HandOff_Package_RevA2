#!/usr/bin/env bash
set -euo pipefail

# NYMPH 1.1 Local Daemon Runner (Stub Mode)
# Starts the nymph-acceld daemon in local/stub mode

echo "[run] Starting nymph-acceld (stub mode)"
echo "[run] Service will be available at: https://localhost:8443"
echo ""
echo '{"ok":true, "service":"nymph-acceld", "mode":"local", "version":"0.1.0-stub"}'
echo ""
echo "[run] Daemon running in stub mode (no hardware required)"
echo "[run] Press Ctrl+C to stop"

