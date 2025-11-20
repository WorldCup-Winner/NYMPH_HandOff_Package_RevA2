# Driver Development Guide — NYMPH 1.1

## Overview

This document describes the kernel driver development for NYMPH 1.1, focusing on the PCIe driver (`pcie_nymph`) that implements the ZLTA-2 DMA fabric interface.

## Driver Architecture

### pcie_nymph Driver

**Purpose**: Char device driver for DMA operations and PCIe communication.

**Features**:
- Char device interface (`/dev/pcie_nymph`)
- IOCTL interface for DMA operations
- PCIe endpoint enumeration
- DMA ring buffer management
- Integrity hash verification (BLAKE3)

**Current Status**: v0.1.0 (Stub Implementation)

## File Structure

```
repo/kernel/pcie_nymph/
├── pcie_nymph.h      - Header with IOCTL definitions
├── pcie_nymph.c      - Main driver implementation
├── Makefile          - Standalone build
├── Kbuild            - Kernel tree build
├── README.md         - Driver documentation
└── test_ioctl.c      - IOCTL test program
```

## Building the Driver

### Standalone Build

```bash
cd repo/kernel/pcie_nymph
make
```

**Requirements**:
- Kernel headers: `/lib/modules/$(uname -r)/build`
- Build tools: gcc, make

### Kernel Tree Build

Add to kernel `.config`:
```
CONFIG_PCIE_NYMPH=m
```

Build:
```bash
make modules
```

## Installation

```bash
# Build
cd repo/kernel/pcie_nymph
make

# Load module
sudo insmod pcie_nymph.ko

# Create device node
MAJOR=$(grep pcie_nymph /proc/devices | awk '{print $1}')
sudo mknod /dev/pcie_nymph c $MAJOR 0
sudo chmod 666 /dev/pcie_nymph
```

Or use Makefile:
```bash
sudo make install
```

## Testing

### Load/Unload Validation

```bash
sudo tools/validate_driver.sh
```

### Full Driver Test

```bash
sudo tools/test_pcie_driver.sh
```

### IOCTL Test

```bash
cd repo/kernel/pcie_nymph
gcc -o test_ioctl test_ioctl.c
sudo ./test_ioctl
```

## IOCTL Interface

### NYMPH_IOC_SUBMIT

Submit a DMA transfer descriptor.

**Input**: `struct nymph_dma_desc`
**Output**: Updated descriptor with status and hash

### NYMPH_IOC_GET_STATUS

Get fabric status and statistics.

**Output**: `struct nymph_fabric_status`

### NYMPH_IOC_SETUP_RING

Setup DMA ring buffer.

**Input**: `struct nymph_dma_ring`

### NYMPH_IOC_ENUM_ENDPOINTS

Enumerate all PCIe endpoints.

**Output**: Array of `struct nymph_endpoint_info`

### NYMPH_IOC_GET_ENDPOINT

Get information about a specific endpoint.

**Input/Output**: `struct nymph_endpoint_info`

## Stub Implementation

The current stub implementation:

1. **Simulates 6 PCIe endpoints**:
   - Switchtec PM40100-B0
   - RK3588 Root Complex
   - NVMe storage
   - 4× Kneron KL730 NPUs

2. **Accepts DMA descriptors** and returns success

3. **Generates stub hash values** (0x1234567890abcdef)

4. **Tracks transfer statistics**

## Real Hardware Integration

When hardware is available, the following need to be implemented:

1. **PCIe Device Detection**:
   - Probe for Switchtec PM40100-B0
   - Enumerate downstream endpoints
   - Map BARs and configure devices

2. **DMA Engine**:
   - Initialize DMA controller
   - Setup ring buffers
   - Handle DMA transfers

3. **BLAKE3 Hashing**:
   - Integrate BLAKE3 library
   - Compute integrity hashes
   - Verify transfer integrity

4. **Interrupt Handling**:
   - Setup MSI/MSI-X interrupts
   - Handle transfer completion
   - Error handling

5. **Zero-Copy Memory**:
   - Allocate DMA-coherent memory
   - Map user-space buffers
   - Manage memory regions

## Debugging

### Check Module Status

```bash
lsmod | grep pcie_nymph
dmesg | grep nymph
```

### Check Device Node

```bash
ls -l /dev/pcie_nymph
cat /proc/devices | grep pcie_nymph
```

### Kernel Logs

```bash
dmesg | tail -50
journalctl -k | grep nymph
```

## Troubleshooting

### Module Won't Load

- Check kernel version compatibility
- Verify kernel headers are installed
- Check dmesg for errors

### Device Node Not Created

- Verify module loaded successfully
- Check major number in `/proc/devices`
- Ensure permissions for mknod

### IOCTL Fails

- Verify device node exists and is accessible
- Check IOCTL command numbers match
- Verify structure sizes match

## Next Steps

1. **Day 4**: Implement DMA engine with BLAKE3 hashing
2. **Day 5**: Integrate with daemon (fabric_zlta.cpp)
3. **Hardware**: Real PCIe device detection and enumeration

## References

- Linux Kernel Documentation: `Documentation/driver-api/`
- PCIe Subsystem: `Documentation/PCI/`
- Char Devices: `Documentation/driver-api/char_dev.rst`

