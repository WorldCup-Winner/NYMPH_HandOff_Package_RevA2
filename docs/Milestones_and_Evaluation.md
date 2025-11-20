# Milestones & Evaluation (6 Parts)

## Milestone 1: Reproducible Base & Repository

- **Scope**: repo structure; Docker/Buildroot image; `/status` endpoint.
- **Acceptance**: clean build from scratch; image boots; `/status` responds.

## Milestone 2: PCIe Driver & ZLTA-2 (DMA & Verification)

- **Scope**: char driver (IOCTL/mmap); DMA rings; BLAKE3 hash; `/fabric/verify`.
- **Acceptance**: `/dev/pcie_nymph` functional; `dma_vs_copy.py` PASS; `/fabric/verify` prints metrics.

## Milestone 3: Runtime Daemon + Local AI (ONNX) + KV-Pinning

- **Scope**: `nymph-acceld` (HTTP/WS/WT); ONNX/TFLite or vendor SDK; `/infer`; `/kv/pin`.
- **Acceptance**: valid JSON outputs; `hit_rate > 0`; `bench.json` + `trace.perfetto` created.

## Milestone 4: Thermal TAITO/TAPIM + MCU

- **Scope**: PMBus & NTC reads; fan PWM/TACH; `/thermal/schedule`.
- **Acceptance**: `thermal_stress.sh` stable ΔT; no hard throttling; telemetry visible.

## Milestone 5: Security SAIR + Vault/OTA

- **Scope**: attestation; signed update; rollback.
- **Acceptance**: `/capsule/run` requires `verified:true`; `ota_update.sh` apply & rollback; `NYMPH_IP_Manifest.json`.

## Milestone 6: Bridges (Browser/Windows) + Final Bench

- **Scope**: web SDK (WT/WS), Windows QoS demo; packaging; docs.
- **Acceptance**: web demo calls `/infer`; QoS logs present; all tests reproducible from clean build.

