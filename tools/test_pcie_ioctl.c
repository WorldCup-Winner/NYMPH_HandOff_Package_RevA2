/*
 * Test program for pcie_nymph driver IOCTL interface
 * Compile: gcc -o test_pcie_ioctl test_pcie_ioctl.c
 * Run: sudo ./test_pcie_ioctl
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

/* Include driver header definitions */
#define PCIE_NYMPH_IOC_MAGIC 'N'

struct nymph_dma_desc {
	unsigned long long src_addr;
	unsigned long long dst_addr;
	unsigned int length;
	unsigned int flags;
	unsigned long long cookie;
};

struct nymph_dma_ring {
	unsigned int ring_size;
	unsigned int head;
	unsigned int tail;
	unsigned int reserved;
	unsigned long long ring_addr;
};

struct nymph_fabric_status {
	unsigned long long dma_bytes;
	unsigned char ring_hash[32];
	unsigned int ring_size;
	unsigned int active_descriptors;
};

#define NYMPH_IOC_SUBMIT_DMA	_IOWR(PCIE_NYMPH_IOC_MAGIC, 1, struct nymph_dma_desc)
#define NYMPH_IOC_GET_STATUS	_IOR(PCIE_NYMPH_IOC_MAGIC, 2, struct nymph_fabric_status)
#define NYMPH_IOC_SETUP_RING	_IOW(PCIE_NYMPH_IOC_MAGIC, 3, struct nymph_dma_ring)
#define NYMPH_IOC_GET_RING	_IOR(PCIE_NYMPH_IOC_MAGIC, 4, struct nymph_dma_ring)
#define NYMPH_IOC_RESET		_IO(PCIE_NYMPH_IOC_MAGIC, 5)

#define DEVICE "/dev/pcie_nymph"

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	struct nymph_dma_ring ring;
	struct nymph_fabric_status status;
	struct nymph_dma_desc desc;

	printf("[test] Opening device: %s\n", DEVICE);
	fd = open(DEVICE, O_RDWR);
	if (fd < 0) {
		perror("open");
		printf("[test] ERROR: Cannot open device (is driver loaded?)\n");
		return 1;
	}
	printf("[test] ✓ Device opened successfully\n");

	/* Test 1: Setup ring */
	printf("\n[test] Test 1: Setting up DMA ring...\n");
	memset(&ring, 0, sizeof(ring));
	ring.ring_size = 256;
	ring.head = 0;
	ring.tail = 0;
	ring.ring_addr = 0x1000000; /* Stub address */

	ret = ioctl(fd, NYMPH_IOC_SETUP_RING, &ring);
	if (ret < 0) {
		perror("ioctl SETUP_RING");
		close(fd);
		return 1;
	}
	printf("[test] ✓ Ring setup successful\n");

	/* Test 2: Get ring */
	printf("\n[test] Test 2: Getting ring configuration...\n");
	memset(&ring, 0, sizeof(ring));
	ret = ioctl(fd, NYMPH_IOC_GET_RING, &ring);
	if (ret < 0) {
		perror("ioctl GET_RING");
		close(fd);
		return 1;
	}
	printf("[test] ✓ Ring size: %u, addr: 0x%llx\n", ring.ring_size, ring.ring_addr);

	/* Test 3: Submit DMA */
	printf("\n[test] Test 3: Submitting DMA descriptor...\n");
	memset(&desc, 0, sizeof(desc));
	desc.src_addr = 0x2000000;
	desc.dst_addr = 0x3000000;
	desc.length = 4096;
	desc.flags = 0;
	desc.cookie = 0x12345678;

	ret = ioctl(fd, NYMPH_IOC_SUBMIT_DMA, &desc);
	if (ret < 0) {
		perror("ioctl SUBMIT_DMA");
		close(fd);
		return 1;
	}
	printf("[test] ✓ DMA descriptor submitted\n");

	/* Test 4: Get status */
	printf("\n[test] Test 4: Getting fabric status...\n");
	memset(&status, 0, sizeof(status));
	ret = ioctl(fd, NYMPH_IOC_GET_STATUS, &status);
	if (ret < 0) {
		perror("ioctl GET_STATUS");
		close(fd);
		return 1;
	}
	printf("[test] ✓ Status retrieved:\n");
	printf("    DMA bytes: %llu\n", status.dma_bytes);
	printf("    Ring size: %u\n", status.ring_size);
	printf("    Active descriptors: %u\n", status.active_descriptors);
	printf("    Ring hash: ");
	for (int i = 0; i < 8; i++) {
		printf("%02x", status.ring_hash[i]);
	}
	printf("...\n");

	/* Test 5: Reset */
	printf("\n[test] Test 5: Resetting driver...\n");
	ret = ioctl(fd, NYMPH_IOC_RESET, 0);
	if (ret < 0) {
		perror("ioctl RESET");
		close(fd);
		return 1;
	}
	printf("[test] ✓ Driver reset successful\n");

	close(fd);
	printf("\n[test] ✓ All IOCTL tests passed!\n");
	return 0;
}

