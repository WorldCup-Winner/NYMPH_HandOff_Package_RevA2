# API Specification (Base URL: https://localhost:8443)

## Endpoints

### GET /status

Returns system status and telemetry.

**Response**:
```json
{
  "uptime_s": 12345,
  "temp_c": 55.0,
  "board_id": "xx:xx:.."
}
```

### GET /fabric/verify

Returns DMA fabric verification status.

**Response**:
```json
{
  "ring_hash": "hex",
  "dma_bytes": 1024000
}
```

### POST /infer

Execute AI inference.

**Request**:
```json
{
  "model": "llm-7b-int4",
  "input": "...",
  "profile": "edge-llm-turbo"
}
```

**Response**:
```json
{
  "latency_ms": 120.5,
  "output": "...",
  "energy_wh": 0.05
}
```

### POST /kv/pin

Pin KV cache region.

**Request**:
```json
{
  "region": "chat_ctx",
  "size_kb": 256
}
```

**Response**:
```json
{
  "hit_rate": 0.85
}
```

### POST /squantum/run

Run quantum-inspired optimization.

**Request**:
```json
{
  "population": 256,
  "seed": 1234,
  "objective": "..."
}
```

**Response**:
```json
{
  "best_score": 0.95,
  "trace": [...]
}
```

### POST /thermal/schedule

Set thermal policy.

**Request**:
```json
{
  "policy": "predictive",
  "target_temp_c": 72
}
```

**Response**:
```json
{
  "ok": true
}
```

### POST /capsule/run

Execute attested capsule.

**Request**:
```json
{
  "id": "caps#1",
  "args": {...}
}
```

**Response**:
```json
{
  "verified": true,
  "result": {...}
}
```

### POST /vault/update

Apply firmware update (multipart).

**Response**:
```json
{
  "applied": true,
  "version": "v1.1"
}
```

### POST /ota/rollback

Rollback to previous version.

**Response**:
```json
{
  "rolled_back": true,
  "version": "v1.0"
}
```

## Error Codes

- `400` - Invalid request
- `401` - Unauthorized
- `409` - Verification failed
- `500` - Runtime error

