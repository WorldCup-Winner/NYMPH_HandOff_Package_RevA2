# Change Tolerance (Hardware Substitutions)

| Change | Impact | Notes |
|--------|--------|-------|
| Thermistor model (100 kΩ) | Low | Update thermal conversion table only |
| Blower model (PWM) | Low | Adjust PWM curve in MCU |
| EEPROM vendor (UID) | Low | I²C address/ID change |
| NPU swap (same PCIe Gen3 ×2 per slot) | Medium | Replace ONNX/SDK bridge; same APIs |
| PCIe switch variant | Medium | Update DTS overlay & enumeration |
| Root SoC change | High | New BSP; userland APIs remain stable |

