/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 SAIR & Vault/OTA Interface
 * 
 * SAIR: Self-Attesting Runtime
 * - Attest binary/model/config before execution
 * - Board-ID bound (24AA02E48 EEPROM)
 * 
 * Vault/OTA: Signed Update & Rollback
 * - Signed firmware updates
 * - Version management
 * - Rollback capability
 */

#ifndef NYMPH_SAIR_VAULT_HPP
#define NYMPH_SAIR_VAULT_HPP

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>
#include <functional>

namespace nymph {
namespace security {

/* Artifact types for attestation */
enum class ArtifactType {
    BINARY,     // Executable binary
    MODEL,      // AI model file
    CONFIG,     // Configuration file
    FIRMWARE    // Firmware image
};

/* Attestation result */
struct AttestationResult {
    bool verified;              // Verification status
    std::string artifact_id;   // Artifact identifier
    ArtifactType type;          // Artifact type
    std::string hash;           // Computed hash (hex)
    std::string expected_hash;  // Expected hash from manifest
    std::string board_id;       // Board ID used for binding
    std::string error_message;  // Error if verification failed
    std::map<std::string, std::string> metadata;  // Additional metadata
};

/* Capsule run request */
struct CapsuleRunRequest {
    std::string id;             // Capsule identifier
    std::map<std::string, std::string> args;  // Execution arguments
    std::string artifact_path; // Path to artifact
    ArtifactType artifact_type; // Type of artifact
    bool require_verification;  // Require verification before run
};

/* Capsule run result */
struct CapsuleRunResult {
    bool verified;              // Verification status
    bool executed;              // Execution status
    std::string result_data;   // Execution result
    std::string error_message; // Error if failed
    std::map<std::string, std::string> metadata;
};

/* OTA update request */
struct OTAUpdateRequest {
    std::string version;        // Target version
    std::string update_path;   // Path to update file
    std::string signature_path;// Path to signature file
    std::string board_id;      // Board ID for binding
    bool force;                 // Force update even if same version
};

/* OTA update result */
struct OTAUpdateResult {
    bool applied;               // Update applied
    bool verified;              // Signature verified
    std::string version;        // Applied version
    std::string previous_version; // Previous version
    std::string error_message;  // Error if failed
    std::map<std::string, std::string> metadata;
};

/* OTA rollback result */
struct OTARollbackResult {
    bool rolled_back;           // Rollback successful
    std::string version;        // Current version after rollback
    std::string previous_version; // Version rolled back from
    std::string error_message;  // Error if failed
};

/* Version information */
struct VersionInfo {
    std::string current_version;
    std::string previous_version;
    std::string build_date;
    std::string build_id;
    std::string board_id;
    bool can_rollback;
};

/* SAIR Manager */
class SAIRManager {
public:
    SAIRManager();
    ~SAIRManager();

    /* Initialize SAIR system */
    bool initialize();

    /* Attest an artifact */
    AttestationResult attest_artifact(const std::string& artifact_path, 
                                      ArtifactType type,
                                      const std::string& expected_hash = "");

    /* Run a capsule with attestation */
    CapsuleRunResult run_capsule(const CapsuleRunRequest& request);

    /* Get board ID from EEPROM */
    std::string get_board_id() const;

    /* Verify signature */
    bool verify_signature(const std::string& data_path, 
                         const std::string& signature_path) const;

    /* Check if initialized */
    bool is_initialized() const { return initialized_; }

private:
    bool initialized_;
    std::string board_id_;
    std::map<std::string, AttestationResult> attestation_cache_;
    mutable std::mutex mutex_;

    /* Internal helpers */
    std::string compute_hash(const std::string& filepath) const;
    std::string read_board_id() const;
    bool check_manifest(const std::string& artifact_id, 
                       const std::string& hash) const;
};

/* Vault Manager */
class VaultManager {
public:
    VaultManager();
    ~VaultManager();

    /* Initialize vault system */
    bool initialize();

    /* Apply OTA update */
    OTAUpdateResult apply_update(const OTAUpdateRequest& request);

    /* Rollback to previous version */
    OTARollbackResult rollback();

    /* Get version information */
    VersionInfo get_version_info() const;

    /* Check if update available */
    bool is_update_available(const std::string& version) const;

    /* Check if initialized */
    bool is_initialized() const { return initialized_; }

private:
    bool initialized_;
    std::string current_version_;
    std::string previous_version_;
    std::string board_id_;
    mutable std::mutex mutex_;

    /* Internal helpers */
    bool save_version(const std::string& version);
    bool load_version();
    std::string read_board_id() const;
    bool verify_update_signature(const std::string& update_path,
                                const std::string& signature_path) const;
};

/* Global instances */
SAIRManager& get_sair_manager();
VaultManager& get_vault_manager();

/* Helper functions for API integration */
CapsuleRunRequest parse_capsule_request(const std::string& json_body);
std::string format_capsule_result(const CapsuleRunResult& result);

OTAUpdateRequest parse_update_request(const std::string& json_body);
std::string format_update_result(const OTAUpdateResult& result);

std::string format_rollback_result(const OTARollbackResult& result);

} // namespace security
} // namespace nymph

#endif // NYMPH_SAIR_VAULT_HPP

