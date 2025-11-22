/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 Logging Infrastructure
 */

#ifndef NYMPH_LOGGER_HPP
#define NYMPH_LOGGER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace nymph {
namespace log {

enum class Level {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void set_level(Level level) {
        level_ = level;
    }

    void set_log_file(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }
        log_file_.open(filename, std::ios::app);
    }

    void log(Level level, const std::string& message) {
        if (level < level_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        std::string level_str;
        switch (level) {
            case Level::DEBUG: level_str = "DEBUG"; break;
            case Level::INFO:  level_str = "INFO "; break;
            case Level::WARN:  level_str = "WARN "; break;
            case Level::ERROR: level_str = "ERROR"; break;
        }

        std::string log_line = "[" + ss.str() + "] [" + level_str + "] " + message;

        // Output to console
        std::cout << log_line << std::endl;

        // Output to file if open
        if (log_file_.is_open()) {
            log_file_ << log_line << std::endl;
            log_file_.flush();
        }
    }

private:
    Logger() : level_(Level::INFO) {}
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    Level level_;
    std::mutex mutex_;
    std::ofstream log_file_;
};

inline void debug(const std::string& message) {
    Logger::instance().log(Level::DEBUG, message);
}

inline void info(const std::string& message) {
    Logger::instance().log(Level::INFO, message);
}

inline void warn(const std::string& message) {
    Logger::instance().log(Level::WARN, message);
}

inline void error(const std::string& message) {
    Logger::instance().log(Level::ERROR, message);
}

} // namespace log
} // namespace nymph

#endif // NYMPH_LOGGER_HPP

