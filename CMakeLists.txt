cmake_minimum_required(VERSION 3.16)

# ─────────────────────────────────────────────────────────
# Project
# ─────────────────────────────────────────────────────────
project(git-lfs-transfer
    VERSION 1.0.0
    DESCRIPTION "Git LFS custom transfer agent implementing the pkt-line protocol"
    LANGUAGES CXX
)

# ─────────────────────────────────────────────────────────
# Include helper modules
# ─────────────────────────────────────────────────────────
include(cmake/StandardSettings.cmake)
include(cmake/CompilerWarnings.cmake)

# ─────────────────────────────────────────────────────────
# Linker — static link GCC & C++ runtime (no .so dependency)
# ─────────────────────────────────────────────────────────
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# ─────────────────────────────────────────────────────────
# Output directories — all runtime binaries go to bin/
# ─────────────────────────────────────────────────────────
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ─────────────────────────────────────────────────────────
# Options
# ─────────────────────────────────────────────────────────
option(ENABLE_LOG "Enable debug logging to /tmp/git-lfs-transfer.log" OFF)
option(BUILD_TESTS  "Build unit tests" ON)

# ─────────────────────────────────────────────────────────
# Sub-directories
# ─────────────────────────────────────────────────────────
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ─────────────────────────────────────────────────────────
# Install
# ─────────────────────────────────────────────────────────
install(TARGETS git-lfs-transfer RUNTIME DESTINATION bin)
