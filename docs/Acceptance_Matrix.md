# Acceptance Matrix

| Milestone | Evidence | PASS Condition |
|-----------|----------|----------------|
| 1 | dist/nymph.img, /status | Clean build; /status returns JSON |
| 2 | dma_vs_copy.py, /fabric/verify | Valid hash; throughput ≥ memcpy |
| 3 | bench.json, trace.perfetto | Valid outputs; hit_rate > 0 |
| 4 | thermal.log | Stable ΔT; no hard throttling |
| 5 | attestation.log, ota.log | verified:true; OTA apply & rollback OK |
| 6 | web demo (/infer), QoS logs | Browser calls /infer; QoS/MMCSS logs |

