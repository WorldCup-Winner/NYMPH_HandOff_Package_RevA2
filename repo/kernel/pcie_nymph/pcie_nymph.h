/* SPDX-License-Identifier: GPL-2.0 */
/*
 * NYMPH 1.1 PCIe Driver - Header File
 * 
 * Provides IOCTL interface for DMA operations and ZLTA-2 fabric
 */

#ifndef _PCIE_NYMPH_H_
#define _PCIE_NYMPH_H_

#include <linux/ioctl.h>
#include <linux/types.h>

/* Device name and major number */
#define PCIE_NYMPH_DEVICE_NAME "pcie_nymph"
#define PCIE_NYMPH_MAJOR 0  /* Dynamic allocation */
#define PCIE_NYMPH_MINOR 0

/* IOCTL magic number */
#define PCIE_NYMPH_IOC_MAGIC 'N'

/* DMA descriptor structure for ZLTA-2 */
struct nymph_dma_desc {
	__u64 src_addr;		/* Source address */
	__u64 dst_addr;		/* Destination address */
	__u32 length;		/* Transfer length in bytes */
	__u32 flags;		/* Transfer flags */
	__u64 cookie;		/* User cookie for completion tracking */
};

/* DMA ring buffer structure */
struct nymph_dma_ring {
	__u32 ring_size;	/* Number of descriptors in ring */
	__u32 head;		/* Producer index */
	__u32 tail;		/* Consumer index */
	__u32 reserved;
	__u64 ring_addr;	/* Physical address of ring buffer */
};

/* Fabric status structure */
struct nymph_fabric_status {
	__u64 dma_bytes;	/* Total bytes transferred */
	__u8 ring_hash[32];	/* BLAKE3 hash of ring state (256 bits) */
	__u32 ring_size;
	__u32 active_descriptors;
};

/* IOCTL commands */
#define NYMPH_IOC_SUBMIT_DMA	_IOWR(PCIE_NYMPH_IOC_MAGIC, 1, struct nymph_dma_desc)
#define NYMPH_IOC_GET_STATUS	_IOR(PCIE_NYMPH_IOC_MAGIC, 2, struct nymph_fabric_status)
#define NYMPH_IOC_SETUP_RING	_IOW(PCIE_NYMPH_IOC_MAGIC, 3, struct nymph_dma_ring)
#define NYMPH_IOC_GET_RING	_IOR(PCIE_NYMPH_IOC_MAGIC, 4, struct nymph_dma_ring)
#define NYMPH_IOC_RESET		_IO(PCIE_NYMPH_IOC_MAGIC, 5)

#define PCIE_NYMPH_IOC_MAXNR 5

/* DMA descriptor flags */
#define NYMPH_DMA_FLAG_ZERO_COPY	(1 << 0)
#define NYMPH_DMA_FLAG_VERIFY_HASH	(1 << 1)
#define NYMPH_DMA_FLAG_COMPLETE_SYNC	(1 << 2)

/* PCIe device IDs (stub - will be updated with real IDs) */
#define PCI_VENDOR_ID_NYMPH		0x1234
#define PCI_DEVICE_ID_NYMPH_SWITCHTEC	0x5678

#endif /* _PCIE_NYMPH_H_ */

