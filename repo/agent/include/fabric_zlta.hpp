/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 ZLTA-2 Fabric Interface
 * 
 * DMA zero-copy fabric with integrity hashing
 */

#ifndef NYMPH_FABRIC_ZLTA_HPP
#define NYMPH_FABRIC_ZLTA_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace nymph {
namespace fabric {

/* DMA descriptor structure (matches kernel driver) */
struct DMADescriptor {
    uint64_t src_addr;
    uint64_t dst_addr;
    uint32_t length;
    uint32_t flags;
    uint64_t cookie;
};

/* DMA ring configuration */
struct DMARing {
    uint32_t ring_size;
    uint32_t head;
    uint32_t tail;
    uint64_t ring_addr;
};

/* Fabric status */
struct FabricStatus {
    uint64_t dma_bytes;
    std::vector<uint8_t> ring_hash;  // 32 bytes (BLAKE3)
    uint32_t ring_size;
    uint32_t active_descriptors;
};

/* ZLTA-2 Fabric Interface */
class ZLTA2Fabric {
public:
    ZLTA2Fabric();
    ~ZLTA2Fabric();

    /* Initialize fabric (open device, setup ring) */
    bool initialize(uint32_t ring_size = 256);

    /* Submit DMA descriptor */
    bool submit_dma(const DMADescriptor& desc);

    /* Get fabric status and hash */
    bool get_status(FabricStatus& status);

    /* Reset fabric state */
    bool reset();

    /* Check if initialized */
    bool is_initialized() const { return initialized_; }

private:
    int device_fd_;
    bool initialized_;
    DMARing ring_;
};

/* Helper function to get fabric verification status (for /fabric/verify endpoint) */
FabricStatus get_fabric_verify_status();

} // namespace fabric
} // namespace nymph

#endif // NYMPH_FABRIC_ZLTA_HPP

