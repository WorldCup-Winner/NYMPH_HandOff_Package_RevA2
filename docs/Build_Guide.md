# Build Guide

## Requirements

- Linux / WSL2, Docker, Git, Python 3
- CMake ≥ 3.12, C++17 toolchain

## Build (Stub Mode)

```bash
./tools/build.sh all
```

Artifacts appear in `/dist/`.

## Run (Skeleton)

```bash
./tools/run_local.sh
```

## Benchmarks & Logs

```bash
./tools/bench-run.sh       # → dist/bench.json + dist/trace.perfetto
./tools/dma_vs_copy.py     # → throughput + hash
./tools/thermal_stress.sh  # → dist/thermal.log
```

## Switching to Real Hardware

- Replace DMA stub with IOCTL to `/dev/pcie_nymph` (ZLTA-2)
- Enable PMBus/NTC + MCU PWM/TACH in thermal back-end
- Keep API contracts unchanged; re-run the same scripts

## DTS / Overlays

- Update `hardware/*.dts(i)` (RK3588 RC, Switchtec ports, M.2, PMBus, EEPROM, MCU)

