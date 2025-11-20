# Hardware Specification — NYMPH 1.1 (Rev-A2)

## Overview (Fixed)

- **Host**: PCIe Gen4 ×8 (mechanical ×16), ≤ 75 W from slot
- **SoC / Root Complex**: Rockchip RK3588 (PCIe RC ×4 + I²C/UART/PMBus)
- **PCIe Switch**: Microchip Switchtec PFX PM40100-B0 (uplink ×8 → downlinks: RK3588 ×4, NVMe ×4, 4× M.2 Key-E ×2)
- **NPU Modules**: 4 × Kneron KL730 (M.2 Key-E, Gen3 ×2 each) → ~100 TOPS INT8 aggregate
- **Storage**: NVMe M.2 Key-M 2280 (Gen3 ×4), 512 GB (model cache)
- **MCU / Management**: STM32F030F4P6 (Fan PWM/TACH, LED, NTC sensing, PMBus bridge)
- **EEPROM (UID)**: Microchip 24AA02E48 (Board ID for SAIR attestation)
- **Thermistors**: EPCOS B57560G104F (100 kΩ) at SoC / VRM / NPU zones
- **Cooling**: Sunon MB50101V2-A99 (50 mm PWM blower)
- **PCB**: 190 × 100 mm, 8 layers FR-4 TG170, ENIG; Zdiff 85 Ω ±10 %, refclk 100 MHz diff

## Lane Map (Logical)

Host (Gen4 ×8) → Switchtec PFX (PM40100-B0)

**Uplink**: PCIE_HOST_TX/RX[0..7]_P/N (x8)

**Downlinks**:
- DL0 (x4) → RK3588 (Gen4 ×4)
- DL1 (x4) → NVMe (Gen3 ×4)
- DL2 (x2) → M.2-E #1
- DL3 (x2) → M.2-E #2
- DL4 (x2) → M.2-E #3
- DL5 (x2) → M.2-E #4

## Control Signals (Per Endpoint)

- PERST#, CLKREQ#, WAKE#, REFCLK± (100 MHz differential)

## Routing Constraints (PCIe Gen4)

- Differential impedance 85 Ω ±10 %
- Intra-pair skew ≤ 5 mil; inter-pair skew ≤ 25 mil
- Refclk 100 MHz diff: match ±5 ps (~30–40 mil)

## Power Rails & Decoupling (Guidelines)

- 12 V slot input → smart VRMs (MPS MP86992) => 5 V / 3.3 V / 1.8 V
- LDOs: 1.2 V / 1.0 V / 0.7 V (PLL) with close-in MLCCs
- PDN: dense MLCC arrays (100 nF 0402) near BGA edges; bulk MLCCs 4.7–22 µF around rails
- PMBus (TPS53667) for telemetry; tie sense lines and keep short current paths
- Ground stitching vias around high-speed regions (≤ 1.5 mm pitch)

## Mechanical & Thermal Notes

- Dual-slot cooler keep-out; via-in-pad on BGAs; heatsink studs outside BGA shadow
- Target ΔT ≤ 25 °C sustained load; reserve airflow corridor across heatsink fins

