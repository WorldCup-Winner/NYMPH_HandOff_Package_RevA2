#!/usr/bin/env bash
set -euo pipefail

# NYMPH 1.1 Clean Script
# Removes build artifacts

echo "[clean] Cleaning build artifacts..."

rm -rf dist || true
mkdir -p dist

echo "[clean] Build artifacts cleaned."

