# Firmware Architecture — Tiger Runtime MAX v1.1

## Overview

A layered architecture that separates **drivers (C)** from the **daemon (C++17)** and exposes a stable REST/WS/WT API to apps and the browser.

```
[Hardware]
RK3588 + Switchtec PM40100-B0 + 4× KL730 + NVMe + STM32 + EEPROM
↓
[Drivers (C)]
pcie_nymph  |  nymph_thermal  |  boardid_eeprom  |  rgb_status
↓
[Daemon C++17: nymph-acceld]
ZLTA-2   |   TAITO/TAPIM   |   KV-Pinning   |   S-Quantum/EDF
SAIR     |   Vault/OTA     |   HTTP/WS/WT API
↓
[Bridges]
Browser (WebTransport/WebSocket) | Windows (WFP QoS/MMCSS)
```

## Subsystems

### 1) ZLTA-2 Fabric

- **Purpose**: DMA zero-copy path with integrity hashing.
- **Responsibilities**: IOCTL to `/dev/pcie_nymph`; ring descriptors; `/fabric/verify`.
- **API**: `GET /fabric/verify` → `{ ring_hash, dma_bytes }`
- **KPI**: lower I/O latency; verifiable transfers.

### 2) TAITO / TAPIM (Thermal)

- **Purpose**: Predict throttling and adjust DVFS/fan curves.
- **Responsibilities**: Read PMBus/NTC; control PWM/TACH (MCU); `/thermal/schedule`.
- **API**: `POST /thermal/schedule` → `{ ok }`
- **KPI**: no hard throttling; ΔT stable over 30 min.

### 3) KV-Pinning

- **Purpose**: Hot KV cache for LLMs (Paged-KV, MQA).
- **API**: `POST /kv/pin` → `{ hit_rate }`
- **KPI**: higher tokens/s; faster first token.

### 4) S-Quantum / EDF

- **Purpose**: Deterministic quantum-inspired optimization.
- **Responsibilities**: superpose/interfere/collapse; seeded reproducibility + witness log.
- **API**: `POST /squantum/run` → `{ best_score, trace }`
- **KPI**: 2–4× faster time-to-good-solution (sampling/search).

### 5) SAIR (Security)

- **Purpose**: Attest binary/model/config before execution.
- **API**: `POST /capsule/run` → `{ verified: true|false, result: {...} }`
- **KPI**: reject modified/unknown artifacts.

### 6) Vault / OTA

- **Purpose**: Signed update & rollback (Board-ID bound).
- **APIs**: `POST /vault/update`, `POST /ota/rollback`
- **KPI**: safe updates; quick recovery.

