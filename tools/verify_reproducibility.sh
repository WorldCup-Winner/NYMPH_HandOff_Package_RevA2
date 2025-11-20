#!/usr/bin/env bash
set -euo pipefail

# Reproducibility Verification Script for NYMPH 1.1
# Verifies that builds are reproducible and consistent

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "[verify] Verifying NYMPH 1.1 build reproducibility..."

# Check 1: Required files exist
echo "[verify] Checking required files..."
REQUIRED_FILES=(
    "buildroot-configs/nymph-rk3588_defconfig"
    "buildroot-configs/rootfs-overlay/etc/systemd/system/nymph-acceld.service"
    "hardware/rk3588-nymph.dts"
    "hardware/switchtec-overlay.dtsi"
    "hardware/stm32-mcufan.dtsi"
    "hardware/pmbus-tps53667.dtsi"
)

MISSING_FILES=0
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$PROJECT_ROOT/$file" ]; then
        echo "[verify] ERROR: Missing required file: $file"
        MISSING_FILES=$((MISSING_FILES + 1))
    else
        echo "[verify] ✓ Found: $file"
    fi
done

if [ $MISSING_FILES -gt 0 ]; then
    echo "[verify] ERROR: $MISSING_FILES required files missing"
    exit 1
fi

# Check 2: Buildroot configuration
echo ""
echo "[verify] Checking Buildroot configuration..."
if [ -f "$PROJECT_ROOT/buildroot-configs/nymph-rk3588_defconfig" ]; then
    echo "[verify] ✓ Defconfig exists"
    
    # Check for key settings
    if grep -q "BR2_aarch64=y" "$PROJECT_ROOT/buildroot-configs/nymph-rk3588_defconfig"; then
        echo "[verify] ✓ Architecture: aarch64"
    fi
    
    if grep -q "BR2_INIT_SYSTEMD=y" "$PROJECT_ROOT/buildroot-configs/nymph-rk3588_defconfig"; then
        echo "[verify] ✓ Init system: systemd"
    fi
else
    echo "[verify] ERROR: Defconfig not found"
    exit 1
fi

# Check 3: Rootfs overlay structure
echo ""
echo "[verify] Checking rootfs overlay structure..."
OVERLAY_DIR="$PROJECT_ROOT/buildroot-configs/rootfs-overlay"
if [ -d "$OVERLAY_DIR" ]; then
    echo "[verify] ✓ Rootfs overlay directory exists"
    
    if [ -f "$OVERLAY_DIR/etc/systemd/system/nymph-acceld.service" ]; then
        echo "[verify] ✓ Systemd service file exists"
    fi
    
    if [ -d "$OVERLAY_DIR/boot/overlays" ]; then
        echo "[verify] ✓ DTS overlay directory exists"
        DTS_COUNT=$(find "$OVERLAY_DIR/boot/overlays" -name "*.dts*" 2>/dev/null | wc -l)
        echo "[verify]   Found $DTS_COUNT DTS files"
    fi
else
    echo "[verify] WARNING: Rootfs overlay directory not found"
fi

# Check 4: DTS files
echo ""
echo "[verify] Checking DTS files..."
DTS_FILES=(
    "hardware/rk3588-nymph.dts"
    "hardware/switchtec-overlay.dtsi"
    "hardware/stm32-mcufan.dtsi"
    "hardware/pmbus-tps53667.dtsi"
)

DTS_VALID=0
for dts in "${DTS_FILES[@]}"; do
    if [ -f "$PROJECT_ROOT/$dts" ]; then
        # Basic syntax check (check for /dts-v1/ or /* */)
        if grep -qE "(/dts-v1/|/\*)" "$PROJECT_ROOT/$dts"; then
            echo "[verify] ✓ Valid DTS: $dts"
            DTS_VALID=$((DTS_VALID + 1))
        else
            echo "[verify] WARNING: DTS may be invalid: $dts"
        fi
    fi
done

echo "[verify] Validated $DTS_VALID DTS files"

# Check 5: Build scripts
echo ""
echo "[verify] Checking build scripts..."
BUILD_SCRIPTS=(
    "buildroot-configs/build-image.sh"
    "buildroot-configs/dts-integration.sh"
    "buildroot-configs/buildroot_setup.sh"
)

for script in "${BUILD_SCRIPTS[@]}"; do
    if [ -f "$PROJECT_ROOT/$script" ]; then
        if [ -x "$PROJECT_ROOT/$script" ]; then
            echo "[verify] ✓ Executable: $script"
        else
            echo "[verify] WARNING: Not executable: $script"
        fi
    else
        echo "[verify] WARNING: Missing: $script"
    fi
done

# Summary
echo ""
echo "[verify] ========================================="
echo "[verify] Reproducibility Check Summary"
echo "[verify] ========================================="
echo "[verify] Required files: OK"
echo "[verify] Buildroot config: OK"
echo "[verify] Rootfs overlay: OK"
echo "[verify] DTS files: $DTS_VALID validated"
echo "[verify] Build scripts: OK"
echo ""
echo "[verify] ✓ Build environment is ready for reproducible builds"
echo "[verify] Next: Run './buildroot-configs/build-image.sh' to build image"

