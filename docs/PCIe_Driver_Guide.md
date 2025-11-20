# PCIe Driver Guide — NYMPH 1.1

## Overview

The `pcie_nymph` driver provides a char device interface for PCIe DMA operations and the ZLTA-2 fabric. It manages communication with the Switchtec PM40100-B0 PCIe switch and provides DMA ring buffer operations.

## Driver Architecture

### Components

1. **Char Device Interface** (`/dev/pcie_nymph`)
   - User-space interface for DMA operations
   - IOCTL-based command interface

2. **PCIe Device Management**
   - Probe/remove for Switchtec PM40100-B0
   - Endpoint enumeration (DL0-DL5)
   - BAR mapping and interrupt setup (future)

3. **DMA Ring Buffer**
   - Descriptor ring for zero-copy DMA
   - Head/tail pointers for producer/consumer
   - Status tracking

4. **ZLTA-2 Fabric**
   - Integrity hashing (BLAKE3)
   - Transfer verification
   - Statistics tracking

## IOCTL Interface

### Setup Ring Buffer

```c
struct nymph_dma_ring ring = {
    .ring_size = 256,
    .head = 0,
    .tail = 0,
    .ring_addr = physical_address
};
ioctl(fd, NYMPH_IOC_SETUP_RING, &ring);
```

### Submit DMA Descriptor

```c
struct nymph_dma_desc desc = {
    .src_addr = source_address,
    .dst_addr = destination_address,
    .length = transfer_length,
    .flags = NYMPH_DMA_FLAG_ZERO_COPY,
    .cookie = user_cookie
};
ioctl(fd, NYMPH_IOC_SUBMIT_DMA, &desc);
```

### Get Fabric Status

```c
struct nymph_fabric_status status;
ioctl(fd, NYMPH_IOC_GET_STATUS, &status);
// status.dma_bytes - total bytes transferred
// status.ring_hash - BLAKE3 hash of ring state
```

### Reset Driver

```c
ioctl(fd, NYMPH_IOC_RESET, 0);
```

## PCIe Endpoint Enumeration

The driver enumerates the following endpoints via Switchtec PM40100-B0:

| Downlink | Endpoint | Lanes | Speed | Purpose |
|----------|----------|-------|-------|---------|
| DL0 | RK3588 | x4 | Gen4 | Root Complex |
| DL1 | NVMe | x4 | Gen3 | Storage |
| DL2 | M.2-E #1 | x2 | Gen3 | NPU 1 (KL730) |
| DL3 | M.2-E #2 | x2 | Gen3 | NPU 2 (KL730) |
| DL4 | M.2-E #3 | x2 | Gen3 | NPU 3 (KL730) |
| DL5 | M.2-E #4 | x2 | Gen3 | NPU 4 (KL730) |

## Building and Loading

### Build

```bash
cd repo/kernel/pcie_nymph
make
```

### Load

```bash
sudo insmod pcie_nymph.ko
```

### Verify

```bash
ls -l /dev/pcie_nymph
dmesg | grep pcie_nymph
```

### Unload

```bash
sudo rmmod pcie_nymph
```

## Testing

### Basic Driver Test

```bash
sudo ./tools/test_pcie_driver.sh
```

### IOCTL Test

```bash
cd tools
gcc -o test_pcie_ioctl test_pcie_ioctl.c
sudo ./test_pcie_ioctl
```

## Current Status (Stub Mode)

The current implementation is a stub that:
- ✅ Creates char device successfully
- ✅ Accepts all IOCTL commands
- ✅ Tracks statistics
- ✅ Manages ring buffer state
- ⏳ Does not perform actual DMA (hardware integration pending)
- ⏳ Does not enumerate real PCIe endpoints (hardware pending)

## Future Implementation

When hardware is available:

1. **PCIe Enumeration**
   - Read Switchtec configuration space
   - Enumerate all downlinks
   - Map BARs for each endpoint

2. **DMA Operations**
   - Queue descriptors to hardware
   - Handle completion interrupts
   - Implement zero-copy transfers

3. **ZLTA-2 Fabric**
   - Integrate BLAKE3 hashing
   - Verify transfer integrity
   - Report fabric status

4. **Interrupt Handling**
   - Setup MSI/MSI-X interrupts
   - Handle DMA completion
   - Handle error conditions

## Troubleshooting

### Device Node Not Created

- Check if module loaded: `lsmod | grep pcie_nymph`
- Check dmesg for errors: `dmesg | grep pcie_nymph`
- Verify udev rules (if needed)

### IOCTL Fails

- Verify device is open: `lsof /dev/pcie_nymph`
- Check permissions: `ls -l /dev/pcie_nymph`
- Verify ring is initialized before submitting DMA

### Module Won't Load

- Check kernel version compatibility
- Verify all dependencies are available
- Check dmesg for specific errors

## API Reference

See `repo/kernel/pcie_nymph/pcie_nymph.h` for complete API definitions.

