#!/usr/bin/env bash
set -euo pipefail

# Buildroot Image Build Script for NYMPH 1.1
# Builds complete firmware image with DTS overlays and daemon

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILDROOT_DIR="${BUILDROOT_DIR:-../buildroot-workspace/buildroot-2024.02}"
DEFCONFIG="nymph-rk3588"

echo "[buildroot] Building NYMPH 1.1 firmware image"

# Check if Buildroot exists
if [ ! -d "$BUILDROOT_DIR" ]; then
    echo "[buildroot] ERROR: Buildroot directory not found at $BUILDROOT_DIR"
    echo "[buildroot] Please download and extract Buildroot first"
    exit 1
fi

cd "$BUILDROOT_DIR"

# Copy defconfig if not already present
if [ ! -f "configs/${DEFCONFIG}_defconfig" ]; then
    if [ -f "$PROJECT_ROOT/buildroot-configs/${DEFCONFIG}_defconfig" ]; then
        cp "$PROJECT_ROOT/buildroot-configs/${DEFCONFIG}_defconfig" "configs/${DEFCONFIG}_defconfig"
        echo "[buildroot] Defconfig copied"
    fi
fi

# Load defconfig
if [ ! -f ".config" ]; then
    echo "[buildroot] Loading defconfig..."
    make "${DEFCONFIG}_defconfig"
fi

# Integrate DTS overlays
echo "[buildroot] Integrating DTS overlays..."
bash "$PROJECT_ROOT/buildroot-configs/dts-integration.sh"

# Build image
echo "[buildroot] Building image (this may take a while)..."
make

# Copy output to dist
echo "[buildroot] Copying output to dist/..."
mkdir -p "$PROJECT_ROOT/dist"
cp output/images/rootfs.ext2 "$PROJECT_ROOT/dist/nymph-rootfs.ext2" 2>/dev/null || true
cp output/images/rootfs.tar "$PROJECT_ROOT/dist/nymph-rootfs.tar" 2>/dev/null || true
cp output/images/*.dtb "$PROJECT_ROOT/dist/" 2>/dev/null || true

# Create image info
cat > "$PROJECT_ROOT/dist/nymph-image-info.txt" << EOF
NYMPH 1.1 Firmware Image
Build Date: $(date -Iseconds)
Buildroot Version: $(cat .br2_version 2>/dev/null || echo "unknown")
Defconfig: ${DEFCONFIG}
Output Directory: $BUILDROOT_DIR/output
EOF

echo "[buildroot] Build complete!"
echo "[buildroot] Output files in: $PROJECT_ROOT/dist/"
ls -lh "$PROJECT_ROOT/dist/" | grep -E "(nymph|\.dtb)" || true

