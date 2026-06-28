# ─────────────────────────────────────────────────────────
# Library (objects) — internal, static
# ─────────────────────────────────────────────────────────
add_library(git-lfs-transfer_obj
    OBJECT
        log.cpp
        protocol.cpp
        lfs_object.cpp
        repository.cpp
)

target_include_directories(git-lfs-transfer_obj
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(git-lfs-transfer_obj PUBLIC cxx_std_17)

if(ENABLE_LOG)
    target_compile_definitions(git-lfs-transfer_obj PRIVATE ENABLE_LOG)
endif()

set_project_warnings(git-lfs-transfer_obj)

# ─────────────────────────────────────────────────────────
# Executable
# ─────────────────────────────────────────────────────────
add_executable(git-lfs-transfer
    main.cpp
)

target_link_libraries(git-lfs-transfer
    PRIVATE
        git-lfs-transfer_obj
)

set_project_warnings(git-lfs-transfer)

# ─────────────────────────────────────────────────────────
# Install
# ─────────────────────────────────────────────────────────
install(TARGETS git-lfs-transfer RUNTIME DESTINATION bin)
