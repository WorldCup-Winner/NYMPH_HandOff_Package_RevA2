/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 KV-Pinning Implementation
 * 
 * KV Cache Region Manager for LLM inference optimization
 * Stub implementation - simulates KV cache behavior
 */

#include "kvpin.hpp"
#include "logger.hpp"
#include <sstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <random>
#include <cstdio>

namespace nymph {
namespace kv {

/* Global KV Cache Manager instance */
static std::unique_ptr<KVCacheManager> g_kv_manager = nullptr;

KVCacheManager& get_kv_cache_manager() {
    if (!g_kv_manager) {
        g_kv_manager = std::make_unique<KVCacheManager>();
        g_kv_manager->initialize();
    }
    return *g_kv_manager;
}

KVCacheManager::KVCacheManager()
    : initialized_(false)
    , total_size_kb_(0)
    , used_size_kb_(0)
    , next_base_address_(0x100000)  // Start at 1MB
{
}

KVCacheManager::~KVCacheManager() {
    // Cleanup if needed
}

uint64_t KVCacheManager::get_current_time() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

bool KVCacheManager::initialize(uint64_t total_cache_size_kb) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }

    log::info("Initializing KV Cache Manager with " + 
              std::to_string(total_cache_size_kb / 1024) + " MB");
    
    total_size_kb_ = total_cache_size_kb;
    used_size_kb_ = 0;
    next_base_address_ = 0x100000;
    regions_.clear();
    
    initialized_ = true;
    log::info("KV Cache Manager initialized (stub mode)");
    return true;
}

bool KVCacheManager::allocate_space(uint64_t size_kb, uint64_t& base_address) {
    // Check if we have enough space
    if (used_size_kb_ + size_kb > total_size_kb_) {
        return false;
    }
    
    base_address = next_base_address_;
    next_base_address_ += size_kb * 1024;  // Convert to bytes for address
    used_size_kb_ += size_kb;
    
    return true;
}

KVPinResult KVCacheManager::pin_region(const KVPinRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    KVPinResult result;
    result.success = false;
    result.region_name = request.region;
    result.region_size_kb = request.size_kb;
    
    if (!initialized_) {
        result.error_message = "KV Cache Manager not initialized";
        return result;
    }

    log::info("Pinning KV region: " + request.region + 
              " (" + std::to_string(request.size_kb) + " KB)");

    // Check if region already exists
    auto it = regions_.find(request.region);
    if (it != regions_.end()) {
        // Region exists, update it
        KVRegion& region = it->second;
        
        if (region.is_pinned) {
            // Already pinned, just update stats
            region.access_count++;
            region.hit_count++;
            region.last_access_time = get_current_time();
            
            result.success = true;
            result.hit_rate = (region.access_count > 0) 
                ? static_cast<double>(region.hit_count) / region.access_count 
                : 0.0;
            result.stats["existing_region"] = 1.0;
            result.stats["access_count"] = static_cast<double>(region.access_count);
            
            log::info("Region already pinned, hit_rate: " + 
                      std::to_string(result.hit_rate));
            return result;
        }
        
        // Re-pin the region
        region.is_pinned = true;
        region.pin_time = get_current_time();
        region.access_count++;
        region.hit_count++;
        region.last_access_time = get_current_time();
        
        result.success = true;
        result.hit_rate = (region.access_count > 0) 
            ? static_cast<double>(region.hit_count) / region.access_count 
            : 0.0;
        result.stats["repinned"] = 1.0;
        
        return result;
    }
    
    // New region - allocate space
    uint64_t base_address;
    if (!allocate_space(request.size_kb, base_address)) {
        // Try to evict if force is set
        if (request.force) {
            uint64_t freed = evict_lru(request.size_kb);
            if (freed >= request.size_kb) {
                if (!allocate_space(request.size_kb, base_address)) {
                    result.error_message = "Failed to allocate space after eviction";
                    return result;
                }
            } else {
                result.error_message = "Insufficient space after eviction";
                return result;
            }
        } else {
            result.error_message = "Insufficient cache space";
            return result;
        }
    }
    
    // Create new region
    KVRegion region;
    region.name = request.region;
    region.size_kb = request.size_kb;
    region.base_address = base_address;
    region.is_pinned = true;
    region.access_count = 1;
    region.hit_count = 1;
    region.miss_count = 0;
    region.last_access_time = get_current_time();
    region.pin_time = get_current_time();
    
    regions_[request.region] = region;
    
    // Simulate realistic hit rate based on region characteristics
    // New regions start with high hit rate (warm cache)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.75, 0.95);  // 75-95% hit rate
    
    result.success = true;
    result.hit_rate = dis(gen);
    result.stats["new_region"] = 1.0;
    result.stats["base_address"] = static_cast<double>(base_address);
    result.stats["total_used_kb"] = static_cast<double>(used_size_kb_);
    result.stats["total_free_kb"] = static_cast<double>(total_size_kb_ - used_size_kb_);
    
    log::info("Region pinned successfully, hit_rate: " + 
              std::to_string(result.hit_rate));
    
    return result;
}

bool KVCacheManager::unpin_region(const std::string& region_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = regions_.find(region_name);
    if (it == regions_.end()) {
        return false;
    }
    
    it->second.is_pinned = false;
    log::info("Region unpinned: " + region_name);
    return true;
}

bool KVCacheManager::access_region(const std::string& region_name, bool is_read) {
    (void)is_read;  // Could be used for write-through cache logic
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = regions_.find(region_name);
    if (it == regions_.end()) {
        return false;
    }
    
    KVRegion& region = it->second;
    region.access_count++;
    region.last_access_time = get_current_time();
    
    // Simulate hit/miss based on pinned status
    if (region.is_pinned) {
        // Pinned regions have high hit rate (90-99%)
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        if (dis(gen) < 0.95) {  // 95% hit rate for pinned regions
            region.hit_count++;
        } else {
            region.miss_count++;
        }
    } else {
        // Unpinned regions have lower hit rate (50-70%)
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        if (dis(gen) < 0.60) {  // 60% hit rate for unpinned regions
            region.hit_count++;
        } else {
            region.miss_count++;
        }
    }
    
    return true;
}

bool KVCacheManager::get_region(const std::string& region_name, KVRegion& region) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = regions_.find(region_name);
    if (it == regions_.end()) {
        return false;
    }
    
    region = it->second;
    return true;
}

KVCacheStats KVCacheManager::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    KVCacheStats stats;
    stats.total_size_kb = total_size_kb_;
    stats.used_size_kb = used_size_kb_;
    stats.free_size_kb = total_size_kb_ - used_size_kb_;
    stats.pinned_regions = 0;
    stats.total_accesses = 0;
    stats.total_hits = 0;
    stats.total_misses = 0;
    
    for (const auto& pair : regions_) {
        const KVRegion& region = pair.second;
        if (region.is_pinned) {
            stats.pinned_regions++;
            stats.pinned_region_names.push_back(region.name);
        }
        stats.total_accesses += region.access_count;
        stats.total_hits += region.hit_count;
        stats.total_misses += region.miss_count;
    }
    
    stats.overall_hit_rate = (stats.total_accesses > 0) 
        ? static_cast<double>(stats.total_hits) / stats.total_accesses 
        : 0.0;
    
    return stats;
}

std::vector<KVRegion> KVCacheManager::list_regions() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<KVRegion> result;
    for (const auto& pair : regions_) {
        result.push_back(pair.second);
    }
    return result;
}

uint64_t KVCacheManager::evict_lru(uint64_t required_kb) {
    // Already holding lock from caller
    
    // Sort regions by last access time (oldest first)
    std::vector<std::pair<std::string, uint64_t>> candidates;
    for (const auto& pair : regions_) {
        if (!pair.second.is_pinned) {  // Only evict unpinned regions
            candidates.push_back({pair.first, pair.second.last_access_time});
        }
    }
    
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    uint64_t freed = 0;
    for (const auto& candidate : candidates) {
        if (freed >= required_kb) break;
        
        auto it = regions_.find(candidate.first);
        if (it != regions_.end()) {
            freed += it->second.size_kb;
            used_size_kb_ -= it->second.size_kb;
            log::info("Evicting region: " + candidate.first);
            regions_.erase(it);
        }
    }
    
    return freed;
}

void KVCacheManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    regions_.clear();
    used_size_kb_ = 0;
    next_base_address_ = 0x100000;
    log::info("KV Cache cleared");
}

/* Helper function to parse KV pin request from JSON */
KVPinRequest parse_kvpin_request(const std::string& json_body) {
    KVPinRequest request;
    request.force = false;
    request.priority = 0;
    
    // Simple JSON parsing
    auto find_string_field = [&json_body](const std::string& field) -> std::string {
        std::string search = "\"" + field + "\"";
        size_t pos = json_body.find(search);
        if (pos == std::string::npos) return "";
        
        pos = json_body.find(":", pos);
        if (pos == std::string::npos) return "";
        pos++;
        
        while (pos < json_body.length() && (json_body[pos] == ' ' || json_body[pos] == '\t')) {
            pos++;
        }
        
        if (pos >= json_body.length()) return "";
        
        if (json_body[pos] == '"') {
            pos++;
            size_t end = pos;
            while (end < json_body.length() && json_body[end] != '"') {
                end++;
            }
            return json_body.substr(pos, end - pos);
        }
        
        return "";
    };
    
    auto find_number_field = [&json_body](const std::string& field) -> uint64_t {
        std::string search = "\"" + field + "\"";
        size_t pos = json_body.find(search);
        if (pos == std::string::npos) return 0;
        
        pos = json_body.find(":", pos);
        if (pos == std::string::npos) return 0;
        pos++;
        
        while (pos < json_body.length() && (json_body[pos] == ' ' || json_body[pos] == '\t')) {
            pos++;
        }
        
        if (pos >= json_body.length()) return 0;
        
        size_t end = pos;
        while (end < json_body.length() && 
               (json_body[end] >= '0' && json_body[end] <= '9')) {
            end++;
        }
        
        if (end > pos) {
            return std::stoull(json_body.substr(pos, end - pos));
        }
        
        return 0;
    };
    
    auto find_bool_field = [&json_body](const std::string& field) -> bool {
        std::string search = "\"" + field + "\"";
        size_t pos = json_body.find(search);
        if (pos == std::string::npos) return false;
        
        pos = json_body.find(":", pos);
        if (pos == std::string::npos) return false;
        pos++;
        
        while (pos < json_body.length() && (json_body[pos] == ' ' || json_body[pos] == '\t')) {
            pos++;
        }
        
        if (pos >= json_body.length()) return false;
        
        return (json_body.substr(pos, 4) == "true");
    };
    
    request.region = find_string_field("region");
    request.size_kb = find_number_field("size_kb");
    request.force = find_bool_field("force");
    request.priority = static_cast<int>(find_number_field("priority"));
    
    // Defaults
    if (request.region.empty()) {
        request.region = "default";
    }
    if (request.size_kb == 0) {
        request.size_kb = 256;  // Default 256 KB
    }
    
    return request;
}

/* Helper function to format KV pin result as JSON */
std::string format_kvpin_result(const KVPinResult& result) {
    std::stringstream json;
    json << std::fixed << std::setprecision(4);
    
    json << "{";
    json << "\"hit_rate\":" << result.hit_rate;
    
    if (result.success) {
        json << ",\"region\":\"" << result.region_name << "\"";
        json << ",\"size_kb\":" << result.region_size_kb;
        
        if (!result.stats.empty()) {
            json << ",\"stats\":{";
            bool first = true;
            for (const auto& pair : result.stats) {
                if (!first) json << ",";
                json << "\"" << pair.first << "\":" << pair.second;
                first = false;
            }
            json << "}";
        }
    } else {
        json << ",\"success\":false";
        if (!result.error_message.empty()) {
            json << ",\"error\":\"" << result.error_message << "\"";
        }
    }
    
    json << "}";
    
    return json.str();
}

} // namespace kv
} // namespace nymph

