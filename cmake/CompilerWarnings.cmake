# ─────────────────────────────────────────────────────────
# Compiler warnings — enabled for all targets in this project
# Modern CMake approach: target-based, not global.
# ─────────────────────────────────────────────────────────

function(set_project_warnings target)
    if(NOT MSVC)
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    else()
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /utf-8
        )
    endif()
endfunction()
