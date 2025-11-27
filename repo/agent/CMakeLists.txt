cmake_minimum_required(VERSION 3.12)
project(nymph-acceld VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main_agent.cpp
    src/nymph_api.cpp
    src/fabric_zlta.cpp
    src/ai_onnx.cpp
    src/kvpin.cpp
)

# Create executable
add_executable(nymph-acceld ${SOURCES})

# Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(nymph-acceld PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(nymph-acceld pthread)
endif()

# Install target
install(TARGETS nymph-acceld
    RUNTIME DESTINATION bin
)

# Output message
message(STATUS "NYMPH 1.1 Daemon Build Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")

