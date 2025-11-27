/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 Thermal System Implementation (TAITO/TAPIM)
 * 
 * Stub implementation - simulates thermal management behavior
 * Real implementation will interface with kernel drivers and MCU
 */

#include "thermal_stdio.hpp"
#include "logger.hpp"
#include <sstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <random>
#include <cmath>
#include <fstream>

namespace nymph {
namespace thermal {

/* Global Thermal Manager instance */
static std::unique_ptr<ThermalManager> g_thermal_manager = nullptr;

ThermalManager& get_thermal_manager() {
    if (!g_thermal_manager) {
        g_thermal_manager = std::make_unique<ThermalManager>();
        g_thermal_manager->initialize();
    }
    return *g_thermal_manager;
}

ThermalManager::ThermalManager()
    : initialized_(false)
    , current_policy_(ThermalPolicy::PREDICTIVE)
    , target_temp_c_(72.0)
    , max_temp_c_(85.0)
{
    // Initialize fan status
    fan_status_.pwm_duty = 128;
    fan_status_.rpm = 3000;
    fan_status_.target_rpm = 3000;
    fan_status_.tach_valid = true;
    fan_status_.stall_detected = false;
    
    // Initialize MCU status
    mcu_status_.connected = true;
    mcu_status_.firmware_version = "1.0.0-stub";
    mcu_status_.uptime_s = 0;
    mcu_status_.fan = fan_status_;
    mcu_status_.led_state = {0x00, 0xFF, 0x00};  // Green
    
    // Initialize stats
    stats_.min_temp_c = 100.0;
    stats_.max_temp_c = 0.0;
    stats_.avg_temp_c = 0.0;
    stats_.throttle_count = 0;
    stats_.throttle_time_ms = 0;
    stats_.power_total_w = 0.0;
    stats_.sample_count = 0;
}

ThermalManager::~ThermalManager() {
    // Cleanup if needed
}

uint64_t ThermalManager::get_current_time() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

std::string ThermalManager::thermal_zone_name(ThermalZone zone) const {
    switch (zone) {
        case ThermalZone::SOC: return "SoC";
        case ThermalZone::VRM: return "VRM";
        case ThermalZone::NPU: return "NPU";
        case ThermalZone::NVME: return "NVMe";
        case ThermalZone::AMBIENT: return "Ambient";
        default: return "Unknown";
    }
}

ThermalZone ThermalManager::thermal_zone_from_name(const std::string& name) const {
    if (name == "SoC" || name == "soc") return ThermalZone::SOC;
    if (name == "VRM" || name == "vrm") return ThermalZone::VRM;
    if (name == "NPU" || name == "npu") return ThermalZone::NPU;
    if (name == "NVMe" || name == "nvme") return ThermalZone::NVME;
    if (name == "Ambient" || name == "ambient") return ThermalZone::AMBIENT;
    return ThermalZone::SOC;
}

double ThermalManager::ntc_resistance_to_temp(double resistance_ohm) const {
    // Steinhart-Hart equation for EPCOS B57560G104F (100kΩ NTC)
    // Simplified: T = 1 / (A + B*ln(R) + C*ln(R)^3) - 273.15
    // Using typical coefficients
    const double A = 0.001129148;
    const double B = 0.000234125;
    const double C = 0.0000000876741;
    
    double ln_r = std::log(resistance_ohm);
    double temp_k = 1.0 / (A + B * ln_r + C * ln_r * ln_r * ln_r);
    return temp_k - 273.15;
}

uint8_t ThermalManager::calculate_fan_pwm(double current_temp, double target_temp) const {
    // Simple proportional control with hysteresis
    double error = current_temp - target_temp;
    
    // Base PWM
    uint8_t base_pwm = 80;  // ~30% minimum
    
    if (error <= -5.0) {
        // Well below target - minimum fan
        return base_pwm;
    } else if (error >= 10.0) {
        // Well above target - maximum fan
        return 255;
    } else {
        // Proportional zone
        double scale = (error + 5.0) / 15.0;  // 0.0 to 1.0
        return static_cast<uint8_t>(base_pwm + scale * (255 - base_pwm));
    }
}

void ThermalManager::update_stats(double temp) {
    stats_.sample_count++;
    
    if (temp < stats_.min_temp_c) stats_.min_temp_c = temp;
    if (temp > stats_.max_temp_c) stats_.max_temp_c = temp;
    
    // Running average
    double n = static_cast<double>(stats_.sample_count);
    stats_.avg_temp_c = stats_.avg_temp_c * ((n - 1) / n) + temp / n;
    
    // Keep history (last 60 samples)
    stats_.temp_history.push_back(temp);
    if (stats_.temp_history.size() > 60) {
        stats_.temp_history.erase(stats_.temp_history.begin());
    }
}

bool ThermalManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return true;
    }

    log::info("Initializing Thermal Manager (TAITO/TAPIM)");
    
    // Initialize zone readings with simulated values
    uint64_t now = get_current_time();
    
    NTCReading soc_reading;
    soc_reading.zone = ThermalZone::SOC;
    soc_reading.temp_c = 55.0;
    soc_reading.resistance_ohm = 47000.0;
    soc_reading.timestamp = now;
    soc_reading.valid = true;
    zone_readings_[ThermalZone::SOC] = soc_reading;
    
    NTCReading vrm_reading;
    vrm_reading.zone = ThermalZone::VRM;
    vrm_reading.temp_c = 58.0;
    vrm_reading.resistance_ohm = 42000.0;
    vrm_reading.timestamp = now;
    vrm_reading.valid = true;
    zone_readings_[ThermalZone::VRM] = vrm_reading;
    
    NTCReading npu_reading;
    npu_reading.zone = ThermalZone::NPU;
    npu_reading.temp_c = 52.0;
    npu_reading.resistance_ohm = 52000.0;
    npu_reading.timestamp = now;
    npu_reading.valid = true;
    zone_readings_[ThermalZone::NPU] = npu_reading;
    
    NTCReading nvme_reading;
    nvme_reading.zone = ThermalZone::NVME;
    nvme_reading.temp_c = 45.0;
    nvme_reading.resistance_ohm = 65000.0;
    nvme_reading.timestamp = now;
    nvme_reading.valid = true;
    zone_readings_[ThermalZone::NVME] = nvme_reading;
    
    NTCReading ambient_reading;
    ambient_reading.zone = ThermalZone::AMBIENT;
    ambient_reading.temp_c = 35.0;
    ambient_reading.resistance_ohm = 100000.0;
    ambient_reading.timestamp = now;
    ambient_reading.valid = true;
    zone_readings_[ThermalZone::AMBIENT] = ambient_reading;
    
    // Initialize PMBus rails
    pmbus_rails_.clear();
    
    PMBusRail rail_5v;
    rail_5v.name = "5V0";
    rail_5v.voltage_v = 5.05;
    rail_5v.current_a = 2.5;
    rail_5v.power_w = 12.625;
    rail_5v.temp_c = 50.0;
    rail_5v.status_ok = true;
    pmbus_rails_.push_back(rail_5v);
    
    PMBusRail rail_3v3;
    rail_3v3.name = "3V3";
    rail_3v3.voltage_v = 3.32;
    rail_3v3.current_a = 3.0;
    rail_3v3.power_w = 9.96;
    rail_3v3.temp_c = 48.0;
    rail_3v3.status_ok = true;
    pmbus_rails_.push_back(rail_3v3);
    
    PMBusRail rail_1v8;
    rail_1v8.name = "1V8";
    rail_1v8.voltage_v = 1.81;
    rail_1v8.current_a = 5.0;
    rail_1v8.power_w = 9.05;
    rail_1v8.temp_c = 55.0;
    rail_1v8.status_ok = true;
    pmbus_rails_.push_back(rail_1v8);
    
    PMBusRail rail_1v0;
    rail_1v0.name = "1V0";
    rail_1v0.voltage_v = 1.01;
    rail_1v0.current_a = 15.0;
    rail_1v0.power_w = 15.15;
    rail_1v0.temp_c = 60.0;
    rail_1v0.status_ok = true;
    pmbus_rails_.push_back(rail_1v0);
    
    initialized_ = true;
    log::info("Thermal Manager initialized (stub mode)");
    return true;
}

void ThermalManager::update_readings() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) return;
    
    uint64_t now = get_current_time();
    
    // Simulate temperature variations
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> temp_var(0.0, 0.5);  // ±0.5°C variation
    
    // Update each zone with realistic variation
    for (auto& pair : zone_readings_) {
        NTCReading& reading = pair.second;
        
        // Add small random variation
        reading.temp_c += temp_var(gen);
        
        // Apply thermal dynamics (slow drift toward ambient + load heating)
        double ambient = zone_readings_[ThermalZone::AMBIENT].temp_c;
        double load_heat = 0.0;
        
        switch (pair.first) {
            case ThermalZone::SOC:
                load_heat = 20.0;  // SoC runs hot
                break;
            case ThermalZone::VRM:
                load_heat = 23.0;  // VRM efficiency losses
                break;
            case ThermalZone::NPU:
                load_heat = 17.0;  // NPU during inference
                break;
            case ThermalZone::NVME:
                load_heat = 10.0;  // NVMe during access
                break;
            case ThermalZone::AMBIENT:
                load_heat = 0.0;   // Ambient is reference
                break;
        }
        
        // Fan effect
        double fan_cooling = (fan_status_.pwm_duty / 255.0) * 15.0;  // Up to 15°C cooling
        
        // Target temperature under current conditions
        double target = ambient + load_heat - fan_cooling;
        
        // Slowly move toward target (thermal inertia)
        reading.temp_c = reading.temp_c * 0.95 + target * 0.05;
        
        // Clamp to reasonable range
        reading.temp_c = std::max(25.0, std::min(95.0, reading.temp_c));
        
        reading.timestamp = now;
    }
    
    // Update MCU uptime
    mcu_status_.uptime_s++;
    mcu_status_.fan = fan_status_;
    
    // Update total power
    stats_.power_total_w = 0.0;
    for (const auto& rail : pmbus_rails_) {
        stats_.power_total_w += rail.power_w;
    }
    
    // Get hottest zone and update stats
    double hottest = 0.0;
    for (const auto& pair : zone_readings_) {
        if (pair.second.temp_c > hottest) {
            hottest = pair.second.temp_c;
        }
    }
    update_stats(hottest);
    
    // Check for throttling
    if (hottest > max_temp_c_) {
        stats_.throttle_count++;
        stats_.throttle_time_ms += 1000;  // Assume 1s update interval
    }
}

ThermalScheduleResult ThermalManager::set_schedule(const ThermalScheduleRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ThermalScheduleResult result;
    result.ok = false;
    
    if (!initialized_) {
        result.message = "Thermal Manager not initialized";
        return result;
    }

    log::info("Setting thermal policy: " + policy_to_string(request.policy) + 
              ", target: " + std::to_string(request.target_temp_c) + "°C");

    // Update policy
    current_policy_ = request.policy;
    target_temp_c_ = request.target_temp_c;
    max_temp_c_ = request.max_temp_c;
    
    // Get current hottest temperature
    double hottest = 0.0;
    for (const auto& pair : zone_readings_) {
        if (pair.second.temp_c > hottest) {
            hottest = pair.second.temp_c;
        }
    }
    
    // Calculate new fan PWM based on policy
    uint8_t new_pwm;
    switch (current_policy_) {
        case ThermalPolicy::PASSIVE:
            new_pwm = request.fan_min_pwm;
            break;
        case ThermalPolicy::ACTIVE:
            new_pwm = calculate_fan_pwm(hottest, target_temp_c_);
            break;
        case ThermalPolicy::PREDICTIVE:
            // TAITO: Use prediction to set fan proactively
            {
                double predicted = predict_temperature(5000);  // 5 seconds ahead
                new_pwm = calculate_fan_pwm(predicted, target_temp_c_);
            }
            break;
        case ThermalPolicy::AGGRESSIVE:
            new_pwm = request.fan_max_pwm;
            break;
        case ThermalPolicy::QUIET:
            new_pwm = std::min(request.fan_min_pwm, static_cast<uint8_t>(100));
            break;
        default:
            new_pwm = 128;
    }
    
    // Apply fan PWM
    fan_status_.pwm_duty = new_pwm;
    fan_status_.target_rpm = (new_pwm * 5000) / 255;  // 0-5000 RPM range
    fan_status_.rpm = fan_status_.target_rpm;  // Instant in stub mode
    
    // Build result
    result.ok = true;
    result.active_policy = current_policy_;
    result.current_temp_c = hottest;
    result.target_temp_c = target_temp_c_;
    result.fan_pwm = fan_status_.pwm_duty;
    result.message = "Thermal schedule applied";
    
    for (const auto& pair : zone_readings_) {
        result.zone_temps[thermal_zone_name(pair.first)] = pair.second.temp_c;
    }
    
    log::info("Thermal schedule applied, fan PWM: " + std::to_string(fan_status_.pwm_duty));
    
    return result;
}

ThermalScheduleResult ThermalManager::get_status() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ThermalScheduleResult result;
    result.ok = initialized_;
    result.active_policy = current_policy_;
    result.target_temp_c = target_temp_c_;
    result.fan_pwm = fan_status_.pwm_duty;
    
    // Get current hottest temperature
    double hottest = 0.0;
    for (const auto& pair : zone_readings_) {
        if (pair.second.temp_c > hottest) {
            hottest = pair.second.temp_c;
        }
        result.zone_temps[thermal_zone_name(pair.first)] = pair.second.temp_c;
    }
    result.current_temp_c = hottest;
    
    result.message = initialized_ ? "Thermal system operational" : "Not initialized";
    
    return result;
}

std::vector<PMBusRail> ThermalManager::read_pmbus_rails() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pmbus_rails_;
}

std::vector<NTCReading> ThermalManager::read_ntc_sensors() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<NTCReading> readings;
    for (const auto& pair : zone_readings_) {
        readings.push_back(pair.second);
    }
    return readings;
}

FanStatus ThermalManager::get_fan_status() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return fan_status_;
}

bool ThermalManager::set_fan_pwm(uint8_t pwm_duty) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) return false;
    
    fan_status_.pwm_duty = pwm_duty;
    fan_status_.target_rpm = (pwm_duty * 5000) / 255;
    fan_status_.rpm = fan_status_.target_rpm;
    
    log::info("Fan PWM set to: " + std::to_string(pwm_duty));
    return true;
}

MCUStatus ThermalManager::get_mcu_status() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return mcu_status_;
}

ThermalStats ThermalManager::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

double ThermalManager::predict_temperature(uint64_t time_ahead_ms) const {
    // TAITO: Simple linear prediction based on recent trend
    if (stats_.temp_history.size() < 5) {
        // Not enough data, return current temp
        double hottest = 0.0;
        for (const auto& pair : zone_readings_) {
            if (pair.second.temp_c > hottest) {
                hottest = pair.second.temp_c;
            }
        }
        return hottest;
    }
    
    // Calculate trend from last 5 samples (assume 1s intervals)
    size_t n = stats_.temp_history.size();
    double recent_avg = 0.0;
    double old_avg = 0.0;
    
    for (size_t i = n - 5; i < n; i++) {
        recent_avg += stats_.temp_history[i];
    }
    recent_avg /= 5.0;
    
    for (size_t i = n - 10; i < n - 5 && i < n; i++) {
        old_avg += stats_.temp_history[i];
    }
    old_avg /= 5.0;
    
    double trend = (recent_avg - old_avg) / 5.0;  // °C per sample
    double predicted = recent_avg + trend * (time_ahead_ms / 1000.0);
    
    // Clamp to reasonable range
    return std::max(25.0, std::min(100.0, predicted));
}

bool ThermalManager::is_throttling() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& pair : zone_readings_) {
        if (pair.second.temp_c > max_temp_c_) {
            return true;
        }
    }
    return false;
}

bool ThermalManager::log_thermal_data(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open()) {
        return false;
    }
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto now_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count() % 1000;
    
    // Format: timestamp,SoC,VRM,NPU,NVMe,Ambient,FanPWM,FanRPM
    file << std::put_time(std::localtime(&now_t), "%Y-%m-%d %H:%M:%S");
    file << "." << std::setfill('0') << std::setw(3) << now_ms;
    
    for (const auto& zone : {ThermalZone::SOC, ThermalZone::VRM, ThermalZone::NPU, 
                             ThermalZone::NVME, ThermalZone::AMBIENT}) {
        auto it = zone_readings_.find(zone);
        if (it != zone_readings_.end()) {
            file << "," << std::fixed << std::setprecision(1) << it->second.temp_c;
        } else {
            file << ",0.0";
        }
    }
    
    file << "," << static_cast<int>(fan_status_.pwm_duty);
    file << "," << fan_status_.rpm;
    file << "\n";
    
    file.close();
    return true;
}

/* Helper functions for API integration */
ThermalScheduleRequest parse_thermal_request(const std::string& json_body) {
    ThermalScheduleRequest request;
    request.policy = ThermalPolicy::PREDICTIVE;
    request.target_temp_c = 72.0;
    request.max_temp_c = 85.0;
    request.fan_min_pwm = 80;
    request.fan_max_pwm = 255;
    request.enable_dvfs = true;
    request.enable_throttle = true;
    
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
    
    auto find_number_field = [&json_body](const std::string& field) -> double {
        std::string search = "\"" + field + "\"";
        size_t pos = json_body.find(search);
        if (pos == std::string::npos) return -1.0;
        
        pos = json_body.find(":", pos);
        if (pos == std::string::npos) return -1.0;
        pos++;
        
        while (pos < json_body.length() && (json_body[pos] == ' ' || json_body[pos] == '\t')) {
            pos++;
        }
        
        if (pos >= json_body.length()) return -1.0;
        
        size_t end = pos;
        while (end < json_body.length() && 
               ((json_body[end] >= '0' && json_body[end] <= '9') || json_body[end] == '.')) {
            end++;
        }
        
        if (end > pos) {
            return std::stod(json_body.substr(pos, end - pos));
        }
        
        return -1.0;
    };
    
    std::string policy_str = find_string_field("policy");
    if (!policy_str.empty()) {
        request.policy = policy_from_string(policy_str);
    }
    
    double target = find_number_field("target_temp_c");
    if (target > 0) {
        request.target_temp_c = target;
    }
    
    double max_temp = find_number_field("max_temp_c");
    if (max_temp > 0) {
        request.max_temp_c = max_temp;
    }
    
    return request;
}

std::string format_thermal_result(const ThermalScheduleResult& result) {
    std::stringstream json;
    json << std::fixed << std::setprecision(1);
    
    json << "{";
    json << "\"ok\":" << (result.ok ? "true" : "false");
    json << ",\"policy\":\"" << policy_to_string(result.active_policy) << "\"";
    json << ",\"current_temp_c\":" << result.current_temp_c;
    json << ",\"target_temp_c\":" << result.target_temp_c;
    json << ",\"fan_pwm\":" << static_cast<int>(result.fan_pwm);
    
    if (!result.zone_temps.empty()) {
        json << ",\"zones\":{";
        bool first = true;
        for (const auto& pair : result.zone_temps) {
            if (!first) json << ",";
            json << "\"" << pair.first << "\":" << pair.second;
            first = false;
        }
        json << "}";
    }
    
    if (!result.message.empty()) {
        json << ",\"message\":\"" << result.message << "\"";
    }
    
    json << "}";
    
    return json.str();
}

std::string policy_to_string(ThermalPolicy policy) {
    switch (policy) {
        case ThermalPolicy::PASSIVE: return "passive";
        case ThermalPolicy::ACTIVE: return "active";
        case ThermalPolicy::PREDICTIVE: return "predictive";
        case ThermalPolicy::AGGRESSIVE: return "aggressive";
        case ThermalPolicy::QUIET: return "quiet";
        default: return "unknown";
    }
}

ThermalPolicy policy_from_string(const std::string& name) {
    if (name == "passive") return ThermalPolicy::PASSIVE;
    if (name == "active") return ThermalPolicy::ACTIVE;
    if (name == "predictive") return ThermalPolicy::PREDICTIVE;
    if (name == "aggressive") return ThermalPolicy::AGGRESSIVE;
    if (name == "quiet") return ThermalPolicy::QUIET;
    return ThermalPolicy::PREDICTIVE;  // Default
}

} // namespace thermal
} // namespace nymph

