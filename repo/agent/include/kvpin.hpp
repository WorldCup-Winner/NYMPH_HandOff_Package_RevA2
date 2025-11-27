/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 KV-Pinning Interface
 * 
 * Hot KV cache management for LLMs with Paged-KV and Multi-Query Attention support
 */

#ifndef NYMPH_KVPIN_HPP
#define NYMPH_KVPIN_HPP

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>

namespace nymph {
namespace kv {

/* KV Region configuration */
struct KVRegion {
    std::string name;           // Region name (e.g., "chat_ctx", "model_cache")
    uint64_t size_kb;           // Size in kilobytes
    uint64_t base_address;      // Base address in KV cache memory
    bool is_pinned;             // Whether region is currently pinned
    uint64_t access_count;      // Number of accesses
    uint64_t hit_count;         // Number of cache hits
    uint64_t miss_count;        // Number of cache misses
    uint64_t last_access_time;  // Timestamp of last access
    uint64_t pin_time;          // Timestamp when pinned
};

/* KV Pin request */
struct KVPinRequest {
    std::string region;         // Region name
    uint64_t size_kb;           // Size in KB
    bool force;                 // Force eviction if needed
    int priority;               // Priority (higher = more important)
};

/* KV Pin result */
struct KVPinResult {
    bool success;
    double hit_rate;            // Hit rate (0.0 - 1.0)
    uint64_t region_size_kb;    // Actual region size
    std::string region_name;    // Region name
    std::string error_message;  // Error if not successful
    std::map<std::string, double> stats;  // Additional statistics
};

/* KV Cache statistics */
struct KVCacheStats {
    uint64_t total_size_kb;     // Total cache size
    uint64_t used_size_kb;      // Currently used
    uint64_t free_size_kb;      // Available
    uint64_t pinned_regions;    // Number of pinned regions
    double overall_hit_rate;    // Overall hit rate
    uint64_t total_accesses;    // Total access count
    uint64_t total_hits;        // Total hits
    uint64_t total_misses;      // Total misses
    std::vector<std::string> pinned_region_names;  // Names of pinned regions
};

/* KV Cache Region Manager */
class KVCacheManager {
public:
    KVCacheManager();
    ~KVCacheManager();

    /* Initialize the cache manager */
    bool initialize(uint64_t total_cache_size_kb = 1024 * 1024);  // Default 1GB

    /* Pin a region in the KV cache */
    KVPinResult pin_region(const KVPinRequest& request);

    /* Unpin a region */
    bool unpin_region(const std::string& region_name);

    /* Access a region (updates hit/miss stats) */
    bool access_region(const std::string& region_name, bool is_read = true);

    /* Get region info */
    bool get_region(const std::string& region_name, KVRegion& region) const;

    /* Get cache statistics */
    KVCacheStats get_stats() const;

    /* List all regions */
    std::vector<KVRegion> list_regions() const;

    /* Evict LRU regions to free space */
    uint64_t evict_lru(uint64_t required_kb);

    /* Clear all regions */
    void clear();

    /* Check if initialized */
    bool is_initialized() const { return initialized_; }

private:
    bool initialized_;
    uint64_t total_size_kb_;
    uint64_t used_size_kb_;
    uint64_t next_base_address_;
    
    std::map<std::string, KVRegion> regions_;
    mutable std::mutex mutex_;

    /* Internal helpers */
    uint64_t get_current_time() const;
    bool allocate_space(uint64_t size_kb, uint64_t& base_address);
    void update_hit_stats(const std::string& region_name, bool is_hit);
};

/* Global KV Cache Manager instance */
KVCacheManager& get_kv_cache_manager();

/* Helper functions for API integration */
KVPinRequest parse_kvpin_request(const std::string& json_body);
std::string format_kvpin_result(const KVPinResult& result);

} // namespace kv
} // namespace nymph

#endif // NYMPH_KVPIN_HPP

