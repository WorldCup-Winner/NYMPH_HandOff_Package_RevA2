#!/usr/bin/env bash
set -euo pipefail

# PCIe Enumeration Test Script for NYMPH 1.1
# Lists PCIe devices and checks for Switchtec/NYMPH components

echo "[pcie] Enumerating PCIe devices for NYMPH 1.1..."

# Check if lspci is available
if ! command -v lspci &> /dev/null; then
    echo "[pcie] ERROR: lspci not found. Install pciutils."
    exit 1
fi

echo ""
echo "=== All PCIe Devices ==="
lspci

echo ""
echo "=== PCIe Tree Structure ==="
lspci -tv

echo ""
echo "=== Looking for NYMPH Components ==="

# Check for Switchtec devices
echo ""
echo "Switchtec PM40100-B0 (expected vendor 0x11f8):"
lspci -d 11f8:* -v || echo "  Not found (expected in stub mode)"

# Check for Kneron NPUs (vendor 0x1e3e)
echo ""
echo "Kneron KL730 NPUs (expected vendor 0x1e3e):"
lspci -d 1e3e:* -v || echo "  Not found (expected in stub mode)"

# Check for NVMe devices
echo ""
echo "NVMe Storage:"
lspci -d *:0108 -v || echo "  Not found (expected in stub mode)"

# Check for Rockchip devices
echo ""
echo "Rockchip RK3588 (expected vendor 0x1d87):"
lspci -d 1d87:* -v || echo "  Not found (expected in stub mode)"

echo ""
echo "=== Expected Topology ==="
echo "Host (Gen4 x8)"
echo "  └─ Switchtec PM40100-B0"
echo "      ├─ DL0: RK3588 (Gen4 x4)"
echo "      ├─ DL1: NVMe (Gen3 x4)"
echo "      ├─ DL2: M.2-E #1 / NPU 1 (Gen3 x2)"
echo "      ├─ DL3: M.2-E #2 / NPU 2 (Gen3 x2)"
echo "      ├─ DL4: M.2-E #3 / NPU 3 (Gen3 x2)"
echo "      └─ DL5: M.2-E #4 / NPU 4 (Gen3 x2)"

echo ""
echo "[pcie] Enumeration complete"
echo "[pcie] Note: In stub mode, actual devices will not be present"

