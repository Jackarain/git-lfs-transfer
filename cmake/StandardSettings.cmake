# ─────────────────────────────────────────────────────────
# Standard build settings shared by all targets
# ─────────────────────────────────────────────────────────

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Build type default ----
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ---- Position-independent code ----
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---- Export compile commands (for IDE / clang-tidy) ----
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- RPATH handling ----
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
