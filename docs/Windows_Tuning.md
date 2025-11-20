# Windows 10/11 Tuning — QoS + MMCSS

- Assign MMCSS priority to inference thread; reduce jitter.
- Use WFP user-mode QoS (DSCP & queue pacing) for game/WebRTC flows.
- Timer resolution to 1 ms only if really needed; verify with ETW.
- Validate with p95 latency histograms and spike count (>25 ms per 10 min).

