/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 Main Daemon Entry Point
 * 
 * nymph-acceld - Main acceleration daemon
 */

#include "nymph_api.hpp"
#include "logger.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <signal.h>
#include <cstring>

// Simple HTTP server implementation (stub - will use cpp-httplib or similar)
// For now, we'll create a minimal HTTP server

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace {
    std::atomic<bool> g_running{true};
    const int PORT = 8443;
    const std::string HOST = "0.0.0.0";
}

void signal_handler(int sig) {
    nymph::log::info("Received signal " + std::to_string(sig) + ", shutting down...");
    g_running = false;
}

std::string parse_json_field(const std::string& json, const std::string& field) {
    size_t pos = json.find("\"" + field + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.length()) return "";
    
    if (json[pos] == '"') {
        pos++;
        size_t end = json.find("\"", pos);
        if (end != std::string::npos) {
            return json.substr(pos, end - pos);
        }
    } else {
        size_t end = pos;
        while (end < json.length() && json[end] != ',' && json[end] != '}' && json[end] != '\n') end++;
        std::string val = json.substr(pos, end - pos);
        // Trim whitespace
        while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
        return val;
    }
    return "";
}

nymph::api::APIRequest parse_request(const std::string& http_request) {
    nymph::api::APIRequest req;
    
    // Parse method and path
    size_t first_line_end = http_request.find("\r\n");
    if (first_line_end == std::string::npos) first_line_end = http_request.find("\n");
    
    if (first_line_end != std::string::npos) {
        std::string first_line = http_request.substr(0, first_line_end);
        size_t space1 = first_line.find(' ');
        size_t space2 = first_line.find(' ', space1 + 1);
        
        if (space1 != std::string::npos && space2 != std::string::npos) {
            req.method = first_line.substr(0, space1);
            req.path = first_line.substr(space1 + 1, space2 - space1 - 1);
        }
    }
    
    // Parse body (if POST)
    size_t body_start = http_request.find("\r\n\r\n");
    if (body_start == std::string::npos) body_start = http_request.find("\n\n");
    if (body_start != std::string::npos) {
        req.body = http_request.substr(body_start + 4);
    }
    
    return req;
}

std::string build_response(const nymph::api::APIResponse& api_resp) {
    std::stringstream http;
    http << "HTTP/1.1 " << api_resp.status_code << " OK\r\n";
    http << "Content-Type: " << api_resp.content_type << "\r\n";
    http << "Content-Length: " << api_resp.body.length() << "\r\n";
    http << "Access-Control-Allow-Origin: *\r\n";
    http << "\r\n";
    http << api_resp.body;
    return http.str();
}

nymph::api::APIResponse route_request(const nymph::api::APIRequest& req) {
    if (req.path == "/status" && req.method == "GET") {
        return nymph::api::api_status(req);
    } else if (req.path == "/fabric/verify" && req.method == "GET") {
        return nymph::api::api_fabric_verify(req);
    } else if (req.path == "/infer" && req.method == "POST") {
        return nymph::api::api_infer(req);
    } else if (req.path == "/kv/pin" && req.method == "POST") {
        return nymph::api::api_kvpin(req);
    } else if (req.path == "/squantum/run" && req.method == "POST") {
        return nymph::api::api_squantum_run(req);
    } else if (req.path == "/thermal/schedule" && req.method == "POST") {
        return nymph::api::api_thermal_schedule(req);
    } else if (req.path == "/capsule/run" && req.method == "POST") {
        return nymph::api::api_capsule_run(req);
    } else if (req.path == "/vault/update" && req.method == "POST") {
        return nymph::api::api_vault_update(req);
    } else if (req.path == "/ota/rollback" && req.method == "POST") {
        return nymph::api::api_ota_rollback(req);
    } else {
        std::stringstream json;
        json << "{\"error\": \"Not found\", \"path\": \"" << req.path << "\"}";
        return nymph::api::APIResponse(404, "application/json", json.str());
    }
}

void handle_client(int client_fd) {
    char buffer[4096];
#ifdef _WIN32
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
#else
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
#endif
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
    std::string request_str(buffer);
    
    nymph::api::APIRequest req = parse_request(request_str);
    nymph::api::APIResponse resp = route_request(req);
    std::string http_response = build_response(resp);
    
    send(client_fd, http_response.c_str(), http_response.length(), 0);
    close(client_fd);
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    std::cout << "NYMPH 1.1 Acceleration Daemon (nymph-acceld)" << std::endl;
    std::cout << "Version: 0.1.0-stub" << std::endl;
    std::cout << "Starting server on " << HOST << ":" << PORT << std::endl;
    
    // Setup signal handlers
#ifndef _WIN32
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#endif
    
    // Initialize logger
    nymph::log::Logger::instance().set_level(nymph::log::Level::INFO);
    nymph::log::info("NYMPH daemon starting...");
    
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        nymph::log::error("Failed to create socket");
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind socket
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        nymph::log::error("Failed to bind socket to port " + std::to_string(PORT));
        close(server_fd);
        return 1;
    }
    
    // Listen
    if (listen(server_fd, 10) < 0) {
        nymph::log::error("Failed to listen on socket");
        close(server_fd);
        return 1;
    }
    
    nymph::log::info("Server listening on http://" + HOST + ":" + std::to_string(PORT));
    nymph::log::info("API endpoints available:");
    nymph::log::info("  GET  /status");
    nymph::log::info("  GET  /fabric/verify");
    nymph::log::info("  POST /infer");
    nymph::log::info("  POST /kv/pin");
    nymph::log::info("  POST /squantum/run");
    nymph::log::info("  POST /thermal/schedule");
    nymph::log::info("  POST /capsule/run");
    nymph::log::info("  POST /vault/update");
    nymph::log::info("  POST /ota/rollback");
    
    // Main loop
    while (g_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (g_running) {
                nymph::log::warn("Failed to accept client connection");
            }
            continue;
        }
        
        // Handle client in thread
        std::thread(handle_client, client_fd).detach();
    }
    
    nymph::log::info("Shutting down server...");
    close(server_fd);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    nymph::log::info("Server stopped");
    
    return 0;
}

