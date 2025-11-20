# NYMPH 1.1 PCIe Driver (pcie_nymph)

Kernel char device driver for PCIe DMA operations and ZLTA-2 fabric.

## Overview

This driver provides:
- Char device interface (`/dev/pcie_nymph`)
- IOCTL interface for DMA operations
- DMA ring buffer management
- Fabric status and verification

## Building

### Standalone Build

```bash
cd repo/kernel/pcie_nymph
make
```

### As Part of Kernel Tree

Add to kernel config:
```
CONFIG_PCIE_NYMPH=m
```

## Loading/Unloading

```bash
# Load module
sudo insmod pcie_nymph.ko

# Check if loaded
lsmod | grep pcie_nymph

# Check device node
ls -l /dev/pcie_nymph

# Unload module
sudo rmmod pcie_nymph
```

## IOCTL Interface

See `pcie_nymph.h` for complete interface definitions.

### Commands

- `NYMPH_IOC_SETUP_RING` - Setup DMA ring buffer
- `NYMPH_IOC_SUBMIT_DMA` - Submit DMA descriptor
- `NYMPH_IOC_GET_STATUS` - Get fabric status
- `NYMPH_IOC_GET_RING` - Get ring configuration
- `NYMPH_IOC_RESET` - Reset driver state

## Stub Mode

Current implementation is a stub that:
- Creates char device successfully
- Accepts IOCTL commands
- Updates statistics
- Does not perform actual DMA (will be implemented with real hardware)

## Testing

See `tools/test_pcie_driver.sh` for test script.

## Status

**Version**: 0.1.0-stub
**Status**: Basic structure complete, ready for hardware integration

