@echo off
echo [build] Starting NYMPH 1.1 build process...

if not exist dist mkdir dist

echo { > dist\NYMPH_IP_Manifest.json
echo   "build": "NYMPH_HandOff_Package_RevA2", >> dist\NYMPH_IP_Manifest.json
echo   "version": "0.1.0-stub", >> dist\NYMPH_IP_Manifest.json
echo   "build_date": "%date% %time%", >> dist\NYMPH_IP_Manifest.json
echo   "features": { >> dist\NYMPH_IP_Manifest.json
echo     "ZLTA2": "ON", >> dist\NYMPH_IP_Manifest.json
echo     "TAITO": "ON", >> dist\NYMPH_IP_Manifest.json
echo     "TAPIM": "ON", >> dist\NYMPH_IP_Manifest.json
echo     "KV_PINNING": "ON", >> dist\NYMPH_IP_Manifest.json
echo     "S_QUANTUM_EDF": "ON", >> dist\NYMPH_IP_Manifest.json
echo     "SAIR": "ON", >> dist\NYMPH_IP_Manifest.json
echo     "VAULT_OTA": "ON" >> dist\NYMPH_IP_Manifest.json
echo   }, >> dist\NYMPH_IP_Manifest.json
echo   "evidence": { >> dist\NYMPH_IP_Manifest.json
echo     "fabric": "dist/fabric_status.json", >> dist\NYMPH_IP_Manifest.json
echo     "bench": "dist/bench.json", >> dist\NYMPH_IP_Manifest.json
echo     "thermal": "dist/thermal.log", >> dist\NYMPH_IP_Manifest.json
echo     "squantum_trace": "dist/squantum_trace.json", >> dist\NYMPH_IP_Manifest.json
echo     "attestation": "dist/attestation.log", >> dist\NYMPH_IP_Manifest.json
echo     "ota": "dist/ota.log" >> dist\NYMPH_IP_Manifest.json
echo   } >> dist\NYMPH_IP_Manifest.json
echo } >> dist\NYMPH_IP_Manifest.json

echo [build] (stub) Build completed. Artifacts in .\dist
echo [build] Manifest: dist\NYMPH_IP_Manifest.json

