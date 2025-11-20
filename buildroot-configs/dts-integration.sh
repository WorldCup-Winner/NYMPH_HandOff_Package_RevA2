#!/usr/bin/env bash
set -euo pipefail

# DTS Overlay Integration Script for NYMPH 1.1
# Integrates device tree overlays into Buildroot build

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILDROOT_DIR="${BUILDROOT_DIR:-../buildroot-workspace/buildroot-2024.02}"
KERNEL_DIR="${KERNEL_DIR:-$BUILDROOT_DIR/output/build/linux-custom}"

echo "[dts] Integrating NYMPH 1.1 DTS overlays"

# Check if Buildroot output exists
if [ ! -d "$BUILDROOT_DIR/output" ]; then
    echo "[dts] WARNING: Buildroot output not found. Run 'make' in Buildroot first."
    echo "[dts] This script will prepare DTS files for integration."
fi

# Create DTS overlay directory in rootfs
OVERLAY_DIR="$PROJECT_ROOT/buildroot-configs/rootfs-overlay/boot/overlays"
mkdir -p "$OVERLAY_DIR"

# Copy DTS files to overlay directory
echo "[dts] Copying DTS overlays to rootfs overlay..."
cp "$PROJECT_ROOT/hardware/rk3588-nymph.dts" "$OVERLAY_DIR/"
cp "$PROJECT_ROOT/hardware/switchtec-overlay.dtsi" "$OVERLAY_DIR/"
cp "$PROJECT_ROOT/hardware/stm32-mcufan.dtsi" "$OVERLAY_DIR/"
cp "$PROJECT_ROOT/hardware/pmbus-tps53667.dtsi" "$OVERLAY_DIR/"

echo "[dts] DTS overlays copied to $OVERLAY_DIR"
echo "[dts] These will be included in the rootfs at /boot/overlays/"

# If kernel directory exists, create integration instructions
if [ -d "$KERNEL_DIR" ]; then
    echo "[dts] Kernel directory found: $KERNEL_DIR"
    echo "[dts] To integrate DTS files into kernel build:"
    echo "[dts]   1. Copy DTS files to $KERNEL_DIR/arch/arm64/boot/dts/rockchip/"
    echo "[dts]   2. Update kernel defconfig to include DTS files"
    echo "[dts]   3. Rebuild kernel"
else
    echo "[dts] Kernel directory not found. DTS files prepared for manual integration."
fi

echo "[dts] Integration complete"

