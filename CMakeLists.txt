cmake_minimum_required(VERSION 3.20)
project(VPNrel)

set(CMAKE_CXX_STANDARD 17)

# ---------------------------
# Include headers
# ---------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

# ---------------------------
# Gather ALL project sources
# ---------------------------
file(GLOB_RECURSE SRC_FILES
        ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# ---------------------------
# vpn_server_test executable
# ---------------------------
add_executable(vpn_server_test
        tests/test_vpn_server.cpp
        ${SRC_FILES}
)

target_include_directories(vpn_server_test PRIVATE include)

if (WIN32)
    target_link_libraries(vpn_server_test PRIVATE ws2_32)
endif()

# ---------------------------
# vpn_client_test executable
# ---------------------------
add_executable(vpn_client_test
        tests/test_vpn_client.cpp
        ${SRC_FILES}
)

target_include_directories(vpn_client_test PRIVATE include)

if (WIN32)
    target_link_libraries(vpn_client_test PRIVATE ws2_32)
endif()

# ---------------------------
# Optional Encryption Test
# ---------------------------
add_executable(test_encryption
        tests/test_encryption.cpp
        src/core/encryption/encryption.cpp
)

target_include_directories(test_encryption PRIVATE include)

if (WIN32)
    target_link_libraries(test_encryption PRIVATE ws2_32)
endif()
