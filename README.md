# NYMPH 1.1 — Firmware Engineering Hand-off Package (Rev-A2)

This package provides a complete C/C++ firmware skeleton, documentation, device-tree templates, test scripts, and demo clients for the NYMPH 1.1 (Freeway) PCIe AI accelerator. You can build and validate without hardware using stubs, then swap in the real drivers when the board arrives.

## Quick Start

1. **Build**: `./tools/build.sh all` → generates `/dist` artifacts.
2. **Run daemon (local stub)**: `./tools/run_local.sh` → https://localhost:8443
3. **Validate**:
   - `./tools/dma_vs_copy.py` → PASS if throughput >= memcpy & hash printed
   - `./tools/bench-run.sh` → creates `dist/bench.json` + `dist/trace.perfetto`
   - `./tools/thermal_stress.sh` → creates `dist/thermal.log` (stable ΔT)

## Subsystems (High Value)

- **ZLTA-2**: DMA zero-copy + integrity hashing
- **TAITO/TAPIM**: Predictive thermal + DVFS + fan control (MCU)
- **KV-Pinning**: Hot KV cache w/ Paged-KV, Multi-Query Attention
- **S-Quantum/EDF**: Deterministic optimization (superpose/interfere/collapse + witness log)
- **SAIR**: Self-attesting runtime (binary/model/config)
- **Vault/OTA**: Signed updates + rollback; Board-ID bound (24AA02E48)

## Directory Layout

- `docs/` - Complete documentation
- `hardware/` - Device tree templates
- `repo/kernel/` - Linux kernel drivers (C)
- `repo/agent/` - Main daemon (C++17)
- `repo/bridge/` - Web SDK and Windows integration
- `tools/` - Build and validation scripts
- `security/` - Signing and verification utilities
- `demo/` - Demo clients (web and Python)
- `dist/` - Build artifacts (generated)

## Build Requirements

- Linux / WSL2, Docker, Git, Python 3
- CMake ≥ 3.12, C++17 toolchain
- Buildroot (for embedded image)

## Milestones

See `docs/Milestones_and_Evaluation.md` for detailed milestone breakdown and acceptance criteria.

## Development Status

**Current Milestone**: 1 - Repository Setup & Build Infrastructure (Day 1)

## License

[Specify license]

