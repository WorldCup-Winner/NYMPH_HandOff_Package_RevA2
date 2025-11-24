/* SPDX-License-Identifier: MIT */
/*
 * NYMPH 1.1 ONNX Runtime Interface
 * 
 * Provides AI inference using ONNX Runtime (or stub implementation)
 */

#ifndef NYMPH_AI_ONNX_HPP
#define NYMPH_AI_ONNX_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace nymph {
namespace ai {

/* Inference request structure */
struct InferenceRequest {
    std::string model_name;      // e.g., "llm-7b-int4"
    std::string input_text;       // Input text/data
    std::string profile;          // e.g., "edge-llm-turbo"
    std::map<std::string, std::string> options;  // Additional options
};

/* Inference result structure */
struct InferenceResult {
    bool success;
    double latency_ms;           // Inference latency in milliseconds
    std::string output;          // Output text/data
    double energy_wh;            // Energy consumption in watt-hours
    std::string error_message;   // Error message if success == false
    std::map<std::string, double> metrics;  // Additional metrics (tokens/s, etc.)
};

/* ONNX Runtime Interface */
class ONNXRuntime {
public:
    ONNXRuntime();
    ~ONNXRuntime();

    /* Initialize the runtime */
    bool initialize(const std::string& model_path = "");

    /* Load a model */
    bool load_model(const std::string& model_name, const std::string& model_path);

    /* Run inference */
    InferenceResult run_inference(const InferenceRequest& request);

    /* Check if runtime is initialized */
    bool is_initialized() const { return initialized_; }

    /* Get available models */
    std::vector<std::string> list_models() const;

    /* Get model info */
    std::map<std::string, std::string> get_model_info(const std::string& model_name) const;

    /* Set execution provider (CPU, CUDA, TensorRT, etc.) */
    void set_execution_provider(const std::string& provider);

private:
    bool initialized_;
    std::string execution_provider_;
    std::map<std::string, std::string> loaded_models_;  // model_name -> model_path
    
    /* Stub mode: simulate inference */
    InferenceResult run_inference_stub(const InferenceRequest& request);
    
    /* Real mode: call ONNX Runtime (when implemented) */
    InferenceResult run_inference_real(const InferenceRequest& request);
};

/* Helper function to parse inference request from JSON */
InferenceRequest parse_inference_request(const std::string& json_body);

/* Helper function to format inference result as JSON */
std::string format_inference_result(const InferenceResult& result);

} // namespace ai
} // namespace nymph

#endif // NYMPH_AI_ONNX_HPP

