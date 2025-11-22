/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 API Handler Declarations
 * 
 * Defines all API endpoint handler signatures
 */

#ifndef NYMPH_API_HPP
#define NYMPH_API_HPP

#include <string>
#include <map>

namespace nymph {
namespace api {

/* Response structure for API handlers */
struct APIResponse {
    int status_code;
    std::string content_type;
    std::string body;
    
    APIResponse(int code = 200, const std::string& type = "application/json", const std::string& b = "")
        : status_code(code), content_type(type), body(b) {}
};

/* Request structure */
struct APIRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> params;
};

/* API Handler Functions */

/* GET /status - System status and telemetry */
APIResponse api_status(const APIRequest& req);

/* GET /fabric/verify - DMA fabric verification status */
APIResponse api_fabric_verify(const APIRequest& req);

/* POST /infer - AI inference */
APIResponse api_infer(const APIRequest& req);

/* POST /kv/pin - KV cache pinning */
APIResponse api_kvpin(const APIRequest& req);

/* POST /squantum/run - Quantum-inspired optimization */
APIResponse api_squantum_run(const APIRequest& req);

/* POST /thermal/schedule - Thermal policy */
APIResponse api_thermal_schedule(const APIRequest& req);

/* POST /capsule/run - Attested capsule execution */
APIResponse api_capsule_run(const APIRequest& req);

/* POST /vault/update - Firmware update */
APIResponse api_vault_update(const APIRequest& req);

/* POST /ota/rollback - OTA rollback */
APIResponse api_ota_rollback(const APIRequest& req);

} // namespace api
} // namespace nymph

#endif // NYMPH_API_HPP

