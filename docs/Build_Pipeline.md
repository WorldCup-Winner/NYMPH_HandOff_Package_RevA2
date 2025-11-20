# Build Pipeline Documentation — NYMPH 1.1

## Overview

The NYMPH 1.1 firmware build pipeline uses Buildroot to create a complete embedded Linux image with:
- Custom device tree overlays (DTS)
- Systemd service for daemon startup
- Root filesystem overlay with configuration files
- Reproducible builds

## Prerequisites

1. **Buildroot 2024.02** (or compatible version)
   ```bash
   cd ..
   mkdir -p buildroot-workspace
   cd buildroot-workspace
   wget https://buildroot.org/downloads/buildroot-2024.02.tar.gz
   tar xzf buildroot-2024.02.tar.gz
   ```

2. **Development Tools**
   - Linux/WSL2 environment
   - Docker (optional, for containerized builds)
   - Git

## Build Process

### Step 1: Setup Buildroot

```bash
cd buildroot-workspace/buildroot-2024.02
export BUILDROOT_DIR=$(pwd)
cd ../../NYMPH_HandOff_Package_RevA2
./buildroot-configs/buildroot_setup.sh
```

### Step 2: Configure Buildroot

```bash
cd $BUILDROOT_DIR
make nymph-rk3588_defconfig
make menuconfig  # Optional: customize settings
```

### Step 3: Integrate DTS Overlays

```bash
cd ../../NYMPH_HandOff_Package_RevA2
./buildroot-configs/dts-integration.sh
```

This script:
- Copies DTS files to rootfs overlay (`/boot/overlays/`)
- Prepares DTS files for kernel integration
- Creates integration instructions

### Step 4: Build Image

**Option A: Using build script (recommended)**
```bash
cd ../../NYMPH_HandOff_Package_RevA2
./buildroot-configs/build-image.sh
```

**Option B: Manual build**
```bash
cd $BUILDROOT_DIR
make
```

### Step 5: Verify Output

```bash
cd ../../NYMPH_HandOff_Package_RevA2
ls -lh dist/
```

Expected outputs:
- `nymph-rootfs.ext2` - Root filesystem image
- `nymph-rootfs.tar` - Root filesystem tarball
- `*.dtb` - Device tree binary files
- `nymph-image-info.txt` - Build information

## Root Filesystem Overlay

The rootfs overlay (`buildroot-configs/rootfs-overlay/`) includes:

### Systemd Service
- `/etc/systemd/system/nymph-acceld.service` - Daemon service file
- Auto-starts on boot
- Restarts on failure

### DTS Overlays
- `/boot/overlays/` - Device tree source files
- Available for runtime overlay or kernel integration

### Daemon Binary
- `/usr/local/bin/nymph-acceld` - Daemon launcher (stub)
- Will be replaced with actual binary during build

## DTS Integration

### Current Status (Stub Mode)

DTS files are placed in rootfs overlay for manual integration:
- `rk3588-nymph.dts` - Main device tree
- `switchtec-overlay.dtsi` - PCIe switch overlay
- `stm32-mcufan.dtsi` - MCU and fan control
- `pmbus-tps53667.dtsi` - PMBus power management

### Integration with Kernel (Future)

When RK3588 BSP is available:

1. Copy DTS files to kernel source:
   ```bash
   cp hardware/*.dts* $KERNEL_DIR/arch/arm64/boot/dts/rockchip/
   ```

2. Update kernel defconfig:
   ```bash
   # Add to kernel defconfig
   CONFIG_ARCH_ROCKCHIP=y
   CONFIG_ROCKCHIP_RK3588=y
   ```

3. Build kernel with DTS:
   ```bash
   make ARCH=arm64 rk3588_nymph_defconfig
   make ARCH=arm64 dtbs
   ```

## Reproducibility

### Build Environment

To ensure reproducible builds:

1. **Use Docker** (recommended):
   ```bash
   docker-compose build nymph-dev
   docker-compose run nymph-dev bash
   # Build inside container
   ```

2. **Document Versions**:
   - Buildroot version: 2024.02
   - Toolchain: Buildroot internal
   - All dependencies pinned in defconfig

### Build Artifacts

All build artifacts are stored in:
- `dist/` - Final images and binaries
- `$BUILDROOT_DIR/output/` - Buildroot build output

### Verification

Run reproducibility check:
```bash
./tools/verify_reproducibility.sh
```

## Troubleshooting

### Buildroot Not Found

If `BUILDROOT_DIR` is not set:
```bash
export BUILDROOT_DIR=/path/to/buildroot-2024.02
```

### DTS Compilation Errors

- Ensure DTS syntax is correct
- Check for missing includes
- Verify compatible strings match kernel

### Service Not Starting

- Check systemd logs: `journalctl -u nymph-acceld`
- Verify binary exists: `ls -l /usr/local/bin/nymph-acceld`
- Check service file: `cat /etc/systemd/system/nymph-acceld.service`

## Next Steps

1. **Kernel Integration**: Integrate DTS files into kernel build
2. **Daemon Build**: Build actual daemon binary and include in image
3. **Hardware Testing**: Test image on actual hardware
4. **OTA Integration**: Add OTA update mechanism to image

