/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 API Handler Implementations
 */

#include "nymph_api.hpp"
#include "fabric_zlta.hpp"
#include "ai_onnx.hpp"
#include "kvpin.hpp"
#include "thermal_stdio.hpp"
#include "sair_vault.hpp"
#include "logger.hpp"
#include <sstream>
#include <ctime>
#include <iomanip>
#include <random>
#include <chrono>

namespace nymph {
namespace api {

/* System uptime tracking */
static std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

/* GET /status - System status and telemetry */
APIResponse api_status(const APIRequest& req) {
    (void)req;  // Unused for GET requests
    log::info("GET /status");

    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        now - start_time).count();

    // Stub values
    double temp_c = 55.0;
    std::string board_id = "aa:bb:cc:dd:ee:ff:00:11";  // Stub board ID

    std::stringstream json;
    json << "{\n"
         << "  \"uptime_s\": " << uptime << ",\n"
         << "  \"temp_c\": " << std::fixed << std::setprecision(1) << temp_c << ",\n"
         << "  \"board_id\": \"" << board_id << "\"\n"
         << "}";

    return APIResponse(200, "application/json", json.str());
}

/* GET /fabric/verify - DMA fabric verification status */
APIResponse api_fabric_verify(const APIRequest& req) {
    (void)req;  // Unused for GET requests
    log::info("GET /fabric/verify");

    try {
        fabric::FabricStatus status = fabric::get_fabric_verify_status();

        // Convert hash to hex string
        std::stringstream hash_hex;
        for (size_t i = 0; i < status.ring_hash.size(); i++) {
            hash_hex << std::hex << std::setfill('0') << std::setw(2)
                     << static_cast<int>(status.ring_hash[i]);
        }

        std::stringstream json;
        json << "{\n"
             << "  \"ring_hash\": \"" << hash_hex.str() << "\",\n"
             << "  \"dma_bytes\": " << status.dma_bytes << "\n"
             << "}";

        return APIResponse(200, "application/json", json.str());
    } catch (const std::exception& e) {
        log::error("Failed to get fabric status: " + std::string(e.what()));
        std::stringstream json;
        json << "{\n"
             << "  \"error\": \"Failed to get fabric status\",\n"
             << "  \"message\": \"" << e.what() << "\"\n"
             << "}";
        return APIResponse(500, "application/json", json.str());
    }
}

/* Global ONNX Runtime instance */
static std::unique_ptr<nymph::ai::ONNXRuntime> g_onnx_runtime = nullptr;

static nymph::ai::ONNXRuntime& get_onnx_runtime() {
    if (!g_onnx_runtime) {
        g_onnx_runtime = std::make_unique<nymph::ai::ONNXRuntime>();
        g_onnx_runtime->initialize();
    }
    return *g_onnx_runtime;
}

/* POST /infer - AI inference */
APIResponse api_infer(const APIRequest& req) {
    log::info("POST /infer");

    try {
        // Parse inference request from JSON body
        nymph::ai::InferenceRequest inference_req = nymph::ai::parse_inference_request(req.body);
        
        log::info("Inference request - model: " + inference_req.model_name + 
                  ", profile: " + inference_req.profile);

        // Get ONNX runtime and run inference
        nymph::ai::ONNXRuntime& runtime = get_onnx_runtime();
        nymph::ai::InferenceResult result = runtime.run_inference(inference_req);

        if (!result.success) {
            std::stringstream json;
            json << "{\n"
                 << "  \"error\": \"" << result.error_message << "\"\n"
                 << "}";
            return APIResponse(500, "application/json", json.str());
        }

        // Format result as JSON
        std::string json_result = nymph::ai::format_inference_result(result);
        return APIResponse(200, "application/json", json_result);

    } catch (const std::exception& e) {
        log::error("Inference failed: " + std::string(e.what()));
        std::stringstream json;
        json << "{\n"
             << "  \"error\": \"Inference failed\",\n"
             << "  \"message\": \"" << e.what() << "\"\n"
             << "}";
        return APIResponse(500, "application/json", json.str());
    }
}

/* POST /kv/pin - KV cache pinning */
APIResponse api_kvpin(const APIRequest& req) {
    log::info("POST /kv/pin");

    try {
        // Parse KV pin request from JSON body
        nymph::kv::KVPinRequest kvpin_req = nymph::kv::parse_kvpin_request(req.body);
        
        log::info("KV pin request - region: " + kvpin_req.region + 
                  ", size_kb: " + std::to_string(kvpin_req.size_kb));

        // Get KV cache manager and pin region
        nymph::kv::KVCacheManager& manager = nymph::kv::get_kv_cache_manager();
        nymph::kv::KVPinResult result = manager.pin_region(kvpin_req);

        // Format result as JSON
        std::string json_result = nymph::kv::format_kvpin_result(result);
        
        if (!result.success) {
            return APIResponse(400, "application/json", json_result);
        }
        
        return APIResponse(200, "application/json", json_result);

    } catch (const std::exception& e) {
        log::error("KV pin failed: " + std::string(e.what()));
        std::stringstream json;
        json << "{\"error\":\"KV pin failed\",\"message\":\"" << e.what() << "\"}";
        return APIResponse(500, "application/json", json.str());
    }
}

/* POST /squantum/run - Quantum-inspired optimization (stub) */
APIResponse api_squantum_run(const APIRequest& req) {
    (void)req;  // Unused in stub mode
    log::info("POST /squantum/run");

    // Stub response
    std::stringstream json;
    json << "{\n"
         << "  \"best_score\": 0.95,\n"
         << "  \"trace\": []\n"
         << "}";

    return APIResponse(200, "application/json", json.str());
}

/* POST /thermal/schedule - Thermal policy (TAITO/TAPIM) */
APIResponse api_thermal_schedule(const APIRequest& req) {
    log::info("POST /thermal/schedule");

    try {
        // Parse thermal schedule request from JSON body
        nymph::thermal::ThermalScheduleRequest thermal_req = 
            nymph::thermal::parse_thermal_request(req.body);
        
        log::info("Thermal schedule request - policy: " + 
                  nymph::thermal::policy_to_string(thermal_req.policy) + 
                  ", target: " + std::to_string(thermal_req.target_temp_c) + "°C");

        // Get thermal manager and set schedule
        nymph::thermal::ThermalManager& manager = nymph::thermal::get_thermal_manager();
        
        // Update readings first
        manager.update_readings();
        
        // Apply schedule
        nymph::thermal::ThermalScheduleResult result = manager.set_schedule(thermal_req);

        // Format result as JSON
        std::string json_result = nymph::thermal::format_thermal_result(result);
        
        if (!result.ok) {
            return APIResponse(400, "application/json", json_result);
        }
        
        return APIResponse(200, "application/json", json_result);

    } catch (const std::exception& e) {
        log::error("Thermal schedule failed: " + std::string(e.what()));
        std::stringstream json;
        json << "{\"ok\":false,\"error\":\"" << e.what() << "\"}";
        return APIResponse(500, "application/json", json.str());
    }
}

/* POST /capsule/run - Attested capsule execution (SAIR) */
APIResponse api_capsule_run(const APIRequest& req) {
    log::info("POST /capsule/run");

    try {
        // Parse capsule run request from JSON body
        nymph::security::CapsuleRunRequest capsule_req = 
            nymph::security::parse_capsule_request(req.body);
        
        log::info("Capsule run request - id: " + capsule_req.id);

        // Get SAIR manager and run capsule
        nymph::security::SAIRManager& sair = nymph::security::get_sair_manager();
        nymph::security::CapsuleRunResult result = sair.run_capsule(capsule_req);

        // Format result as JSON
        std::string json_result = nymph::security::format_capsule_result(result);
        
        if (!result.verified) {
            return APIResponse(409, "application/json", json_result);  // 409 Verification failed
        }
        
        return APIResponse(200, "application/json", json_result);

    } catch (const std::exception& e) {
        log::error("Capsule run failed: " + std::string(e.what()));
        std::stringstream json;
        json << "{\"verified\":false,\"error\":\"" << e.what() << "\"}";
        return APIResponse(500, "application/json", json.str());
    }
}

/* POST /vault/update - Firmware update (OTA) */
APIResponse api_vault_update(const APIRequest& req) {
    log::info("POST /vault/update");

    try {
        // Parse update request
        nymph::security::OTAUpdateRequest update_req = 
            nymph::security::parse_update_request(req.body);
        
        log::info("OTA update request - version: " + update_req.version);

        // Get Vault manager and apply update
        nymph::security::VaultManager& vault = nymph::security::get_vault_manager();
        nymph::security::OTAUpdateResult result = vault.apply_update(update_req);

        // Format result as JSON
        std::string json_result = nymph::security::format_update_result(result);
        
        if (!result.applied && !result.verified) {
            return APIResponse(400, "application/json", json_result);
        }
        
        return APIResponse(200, "application/json", json_result);

    } catch (const std::exception& e) {
        log::error("OTA update failed: " + std::string(e.what()));
        std::stringstream json;
        json << "{\"applied\":false,\"error\":\"" << e.what() << "\"}";
        return APIResponse(500, "application/json", json.str());
    }
}

/* POST /ota/rollback - OTA rollback */
APIResponse api_ota_rollback(const APIRequest& req) {
    (void)req;  // No body needed for rollback
    log::info("POST /ota/rollback");

    try {
        // Get Vault manager and rollback
        nymph::security::VaultManager& vault = nymph::security::get_vault_manager();
        nymph::security::OTARollbackResult result = vault.rollback();

        // Format result as JSON
        std::string json_result = nymph::security::format_rollback_result(result);
        
        if (!result.rolled_back) {
            return APIResponse(400, "application/json", json_result);
        }
        
        return APIResponse(200, "application/json", json_result);

    } catch (const std::exception& e) {
        log::error("OTA rollback failed: " + std::string(e.what()));
        std::stringstream json;
        json << "{\"rolled_back\":false,\"error\":\"" << e.what() << "\"}";
        return APIResponse(500, "application/json", json.str());
    }
}

} // namespace api
} // namespace nymph

