# ─────────────────────────────────────────────────────────
# Unit tests (requires BUILD_TESTS=ON)
# ─────────────────────────────────────────────────────────
find_package(GTest QUIET)

if(GTest_FOUND)
    # Enable all the Google Tests
    enable_testing()

    # ── test_protocol ─────────────────────────────────
    add_executable(test_protocol
        test_protocol.cpp
    )
    target_link_libraries(test_protocol
        PRIVATE
            git-lfs-transfer_obj
            GTest::GTest
            GTest::Main
    )
    target_compile_features(test_protocol PUBLIC cxx_std_17)
    set_project_warnings(test_protocol)
    add_test(NAME test_protocol COMMAND test_protocol)

    # ── test_lfs_object ───────────────────────────────
    add_executable(test_lfs_object
        test_lfs_object.cpp
    )
    target_link_libraries(test_lfs_object
        PRIVATE
            git-lfs-transfer_obj
            GTest::GTest
            GTest::Main
    )
    target_compile_features(test_lfs_object PUBLIC cxx_std_17)
    set_project_warnings(test_lfs_object)
    add_test(NAME test_lfs_object COMMAND test_lfs_object)

    # ── test_repository ───────────────────────────────
    add_executable(test_repository
        test_repository.cpp
    )
    target_link_libraries(test_repository
        PRIVATE
            git-lfs-transfer_obj
            GTest::GTest
            GTest::Main
    )
    target_compile_features(test_repository PUBLIC cxx_std_17)
    set_project_warnings(test_repository)
    add_test(NAME test_repository COMMAND test_repository)

else()
    message(STATUS "GTest not found — skipping unit tests. "
                   "Install googletest or set BUILD_TESTS=OFF")
endif()
