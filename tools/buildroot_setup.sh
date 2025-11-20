#!/usr/bin/env bash
set -euo pipefail

# Buildroot Setup Script for NYMPH 1.1

BUILDROOT_DIR="${BUILDROOT_DIR:-../buildroot-workspace/buildroot-2024.02}"
DEFCONFIG="nymph-rk3588"

echo "[buildroot] Setting up Buildroot for NYMPH 1.1"

if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "[buildroot] ERROR: Buildroot directory not found at $BUILDROOT_DIR"
    echo "[buildroot] Please download Buildroot first:"
    echo "[buildroot]   cd .. && mkdir -p buildroot-workspace && cd buildroot-workspace"
    echo "[buildroot]   wget https://buildroot.org/downloads/buildroot-2024.02.tar.gz"
    echo "[buildroot]   tar xzf buildroot-2024.02.tar.gz"
    exit 1
fi

cd "$BUILDROOT_DIR"

# Copy defconfig if it exists
if [ -f "../NYMPH_HandOff_Package_RevA2/buildroot-configs/${DEFCONFIG}_defconfig" ]; then
    cp "../NYMPH_HandOff_Package_RevA2/buildroot-configs/${DEFCONFIG}_defconfig" "configs/${DEFCONFIG}_defconfig"
    echo "[buildroot] Defconfig copied"
fi

echo "[buildroot] Setup complete"
echo "[buildroot] To configure: cd $BUILDROOT_DIR && make ${DEFCONFIG}_defconfig"

