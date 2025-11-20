/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 ZLTA-2 Fabric Implementation
 * 
 * Stub implementation - interfaces with /dev/pcie_nymph driver
 */

#include "fabric_zlta.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <stdexcept>

// IOCTL definitions (matching kernel driver)
#define PCIE_NYMPH_DEVICE_NAME "/dev/pcie_nymph"
#define PCIE_NYMPH_IOC_MAGIC 'N'

#define _IOC(dir, type, nr, size) \
    (((dir) << 30) | ((type) << 8) | (nr) | ((size) << 16))
#define _IOR(type, nr, size) _IOC(2, type, nr, size)
#define _IOW(type, nr, size) _IOC(1, type, nr, size)
#define _IOWR(type, nr, size) _IOC(3, type, nr, size)

#define NYMPH_IOC_SUBMIT_DMA _IOWR(PCIE_NYMPH_IOC_MAGIC, 1, sizeof(nymph::fabric::DMADescriptor))
#define NYMPH_IOC_GET_STATUS _IOR(PCIE_NYMPH_IOC_MAGIC, 2, sizeof(nymph::fabric::FabricStatus))
#define NYMPH_IOC_SETUP_RING _IOW(PCIE_NYMPH_IOC_MAGIC, 3, sizeof(nymph::fabric::DMARing))
#define NYMPH_IOC_RESET _IOC(0, PCIE_NYMPH_IOC_MAGIC, 5, 0)

namespace nymph {
namespace fabric {

ZLTA2Fabric::ZLTA2Fabric() : device_fd_(-1), initialized_(false) {
    memset(&ring_, 0, sizeof(ring_));
}

ZLTA2Fabric::~ZLTA2Fabric() {
    if (device_fd_ >= 0) {
        close(device_fd_);
    }
}

bool ZLTA2Fabric::initialize(uint32_t ring_size) {
    if (initialized_) {
        return true;
    }

    // Open device
    device_fd_ = open(PCIE_NYMPH_DEVICE_NAME, O_RDWR);
    if (device_fd_ < 0) {
        // In stub mode, continue without device
        // Real implementation would return false
        return false;
    }

    // Setup ring
    ring_.ring_size = ring_size;
    ring_.head = 0;
    ring_.tail = 0;
    ring_.ring_addr = 0x1000000;  // Stub address

    if (ioctl(device_fd_, NYMPH_IOC_SETUP_RING, &ring_) < 0) {
        close(device_fd_);
        device_fd_ = -1;
        return false;
    }

    initialized_ = true;
    return true;
}

bool ZLTA2Fabric::submit_dma(const DMADescriptor& desc) {
    if (!initialized_ || device_fd_ < 0) {
        // Stub mode: just return success
        return true;
    }

    DMADescriptor desc_copy = desc;
    if (ioctl(device_fd_, NYMPH_IOC_SUBMIT_DMA, &desc_copy) < 0) {
        return false;
    }

    return true;
}

bool ZLTA2Fabric::get_status(FabricStatus& status) {
    if (!initialized_ || device_fd_ < 0) {
        // Stub mode: return fake status
        status.dma_bytes = 0;
        status.ring_hash.resize(32, 0xAA);
        status.ring_size = ring_.ring_size;
        status.active_descriptors = 0;
        return true;
    }

    // Note: This structure needs to match kernel's struct nymph_fabric_status
    struct {
        uint64_t dma_bytes;
        uint8_t ring_hash[32];
        uint32_t ring_size;
        uint32_t active_descriptors;
    } kernel_status;

    if (ioctl(device_fd_, NYMPH_IOC_GET_STATUS, &kernel_status) < 0) {
        return false;
    }

    status.dma_bytes = kernel_status.dma_bytes;
    status.ring_hash.assign(kernel_status.ring_hash, kernel_status.ring_hash + 32);
    status.ring_size = kernel_status.ring_size;
    status.active_descriptors = kernel_status.active_descriptors;

    return true;
}

bool ZLTA2Fabric::reset() {
    if (!initialized_ || device_fd_ < 0) {
        return true;
    }

    if (ioctl(device_fd_, NYMPH_IOC_RESET, 0) < 0) {
        return false;
    }

    return true;
}

FabricStatus get_fabric_verify_status() {
    ZLTA2Fabric fabric;
    FabricStatus status;

    // Initialize if needed
    if (!fabric.is_initialized()) {
        fabric.initialize();
    }

    // Get status
    fabric.get_status(status);

    return status;
}

} // namespace fabric
} // namespace nymph

