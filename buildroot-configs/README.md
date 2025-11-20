# Buildroot Configuration for NYMPH 1.1

This directory contains Buildroot configuration files and rootfs overlays for building the NYMPH 1.1 firmware image.

## Files

### Configuration Files

- `nymph-rk3588_defconfig` - Main Buildroot defconfig for RK3588 SoC
- `buildroot_setup.sh` - Setup script to initialize Buildroot
- `build-image.sh` - Complete image build script
- `dts-integration.sh` - DTS overlay integration script

### Root Filesystem Overlay

- `rootfs-overlay/` - Files to overlay onto root filesystem
  - `etc/systemd/system/nymph-acceld.service` - Systemd service file
  - `usr/local/bin/nymph-acceld` - Daemon launcher (stub)
  - `boot/overlays/` - DTS overlay files

## Quick Start

1. **Download Buildroot** (if not already done):
   ```bash
   cd ..
   mkdir -p buildroot-workspace
   cd buildroot-workspace
   wget https://buildroot.org/downloads/buildroot-2024.02.tar.gz
   tar xzf buildroot-2024.02.tar.gz
   ```

2. **Setup Buildroot**:
   ```bash
   cd ../../NYMPH_HandOff_Package_RevA2
   export BUILDROOT_DIR=../buildroot-workspace/buildroot-2024.02
   ./buildroot-configs/buildroot_setup.sh
   ```

3. **Build Image**:
   ```bash
   ./buildroot-configs/build-image.sh
   ```

## Configuration Details

### Defconfig Settings

- **Architecture**: aarch64 (ARM64)
- **CPU**: Cortex-A76
- **Init System**: systemd
- **Packages**: Python3, OpenSSL, libcurl, BusyBox
- **Rootfs Overlay**: Automatically included from `rootfs-overlay/`

### Rootfs Overlay Structure

```
rootfs-overlay/
├── etc/
│   └── systemd/
│       └── system/
│           └── nymph-acceld.service
├── usr/
│   └── local/
│       └── bin/
│           └── nymph-acceld
└── boot/
    └── overlays/
        ├── rk3588-nymph.dts
        ├── switchtec-overlay.dtsi
        ├── stm32-mcufan.dtsi
        └── pmbus-tps53667.dtsi
```

## DTS Integration

Device tree overlays are automatically copied to `/boot/overlays/` in the rootfs. For kernel integration:

1. Copy DTS files to kernel source tree
2. Update kernel defconfig
3. Rebuild kernel with DTS

See `docs/Build_Pipeline.md` for detailed instructions.

## Service Configuration

The daemon service (`nymph-acceld.service`) is configured to:
- Start after network is available
- Restart automatically on failure
- Log to systemd journal
- Run with security restrictions

## Troubleshooting

### Buildroot Not Found

Set `BUILDROOT_DIR` environment variable:
```bash
export BUILDROOT_DIR=/path/to/buildroot-2024.02
```

### Service Not Starting

Check systemd logs:
```bash
journalctl -u nymph-acceld -f
```

### DTS Compilation Errors

Verify DTS syntax and includes. Check kernel compatibility.

## Next Steps

1. Integrate actual daemon binary into build
2. Add kernel DTS compilation to build process
3. Test image on hardware
4. Add OTA update mechanism

