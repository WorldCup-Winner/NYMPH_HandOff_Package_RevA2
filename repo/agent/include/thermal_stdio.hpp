/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 Thermal System Interface (TAITO/TAPIM)
 * 
 * Predictive thermal management with DVFS and fan control
 * - TAITO: Thermal-Aware Inference Timing Optimization
 * - TAPIM: Thermal-Aware Power and Inference Management
 */

#ifndef NYMPH_THERMAL_STDIO_HPP
#define NYMPH_THERMAL_STDIO_HPP

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>

namespace nymph {
namespace thermal {

/* Thermal zone identifiers */
enum class ThermalZone {
    SOC,        // RK3588 SoC
    VRM,        // Voltage Regulator Module
    NPU,        // KL730 NPU modules
    NVME,       // NVMe storage
    AMBIENT     // Board ambient
};

/* Thermal policy modes */
enum class ThermalPolicy {
    PASSIVE,        // Reduce performance to lower temp
    ACTIVE,         // Use fan to maintain temp
    PREDICTIVE,     // Predict and preemptively adjust (TAITO)
    AGGRESSIVE,     // Maximum cooling, performance priority
    QUIET           // Minimum fan noise, temp priority
};

/* PMBus rail status */
struct PMBusRail {
    std::string name;           // Rail name (e.g., "5V0", "3V3", "1V8")
    double voltage_v;           // Current voltage
    double current_a;           // Current draw
    double power_w;             // Power consumption
    double temp_c;              // Rail temperature
    bool status_ok;             // Status flag
};

/* NTC thermistor reading */
struct NTCReading {
    ThermalZone zone;
    double temp_c;              // Temperature in Celsius
    double resistance_ohm;      // Raw resistance
    uint64_t timestamp;         // Reading timestamp
    bool valid;                 // Reading validity
};

/* Fan status */
struct FanStatus {
    uint8_t pwm_duty;           // PWM duty cycle (0-255)
    uint16_t rpm;               // Current RPM
    uint16_t target_rpm;        // Target RPM
    bool tach_valid;            // TACH signal valid
    bool stall_detected;        // Stall condition
};

/* MCU status */
struct MCUStatus {
    bool connected;             // MCU communication OK
    std::string firmware_version;
    uint32_t uptime_s;
    FanStatus fan;
    std::vector<uint8_t> led_state;  // LED pattern state
};

/* Thermal schedule request */
struct ThermalScheduleRequest {
    ThermalPolicy policy;
    double target_temp_c;       // Target temperature
    double max_temp_c;          // Maximum allowed temperature
    uint8_t fan_min_pwm;        // Minimum fan PWM
    uint8_t fan_max_pwm;        // Maximum fan PWM
    bool enable_dvfs;           // Enable DVFS adjustment
    bool enable_throttle;       // Enable thermal throttling
};

/* Thermal schedule result */
struct ThermalScheduleResult {
    bool ok;
    ThermalPolicy active_policy;
    double current_temp_c;      // Current temperature (hottest zone)
    double target_temp_c;       // Target temperature
    uint8_t fan_pwm;            // Current fan PWM
    std::string message;
    std::map<std::string, double> zone_temps;  // Per-zone temperatures
};

/* Thermal statistics */
struct ThermalStats {
    double min_temp_c;
    double max_temp_c;
    double avg_temp_c;
    uint64_t throttle_count;    // Times throttled
    uint64_t throttle_time_ms;  // Total throttle time
    double power_total_w;       // Total power consumption
    uint64_t sample_count;
    std::vector<double> temp_history;  // Recent temperature readings
};

/* Thermal Manager (TAITO/TAPIM) */
class ThermalManager {
public:
    ThermalManager();
    ~ThermalManager();

    /* Initialize thermal management */
    bool initialize();

    /* Set thermal policy/schedule */
    ThermalScheduleResult set_schedule(const ThermalScheduleRequest& request);

    /* Get current thermal status */
    ThermalScheduleResult get_status() const;

    /* Read PMBus rails */
    std::vector<PMBusRail> read_pmbus_rails() const;

    /* Read NTC thermistors */
    std::vector<NTCReading> read_ntc_sensors() const;

    /* Get fan status */
    FanStatus get_fan_status() const;

    /* Set fan PWM directly */
    bool set_fan_pwm(uint8_t pwm_duty);

    /* Get MCU status */
    MCUStatus get_mcu_status() const;

    /* Get thermal statistics */
    ThermalStats get_stats() const;

    /* TAITO: Predict thermal trajectory */
    double predict_temperature(uint64_t time_ahead_ms) const;

    /* TAPIM: Check if throttling needed */
    bool is_throttling() const;

    /* Force thermal reading update */
    void update_readings();

    /* Log thermal data to file */
    bool log_thermal_data(const std::string& filepath) const;

    /* Check if initialized */
    bool is_initialized() const { return initialized_; }

private:
    bool initialized_;
    ThermalPolicy current_policy_;
    double target_temp_c_;
    double max_temp_c_;
    
    // Current readings
    std::map<ThermalZone, NTCReading> zone_readings_;
    std::vector<PMBusRail> pmbus_rails_;
    FanStatus fan_status_;
    MCUStatus mcu_status_;
    
    // Statistics
    ThermalStats stats_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Internal helpers
    uint64_t get_current_time() const;
    double ntc_resistance_to_temp(double resistance_ohm) const;
    uint8_t calculate_fan_pwm(double current_temp, double target_temp) const;
    void update_stats(double temp);
    std::string thermal_zone_name(ThermalZone zone) const;
    ThermalZone thermal_zone_from_name(const std::string& name) const;
};

/* Global Thermal Manager instance */
ThermalManager& get_thermal_manager();

/* Helper functions for API integration */
ThermalScheduleRequest parse_thermal_request(const std::string& json_body);
std::string format_thermal_result(const ThermalScheduleResult& result);

/* Policy name conversions */
std::string policy_to_string(ThermalPolicy policy);
ThermalPolicy policy_from_string(const std::string& name);

} // namespace thermal
} // namespace nymph

#endif // NYMPH_THERMAL_STDIO_HPP

