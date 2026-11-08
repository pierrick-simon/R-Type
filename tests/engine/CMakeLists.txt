option(ENGINE_TESTS_SANITIZE "Build the engine tests with AddressSanitizer and UBSan" ON)

set(SRCS_TESTS
    tests_edge_cases.cpp
    tests_duplicates.cpp
    tests_simulation.cpp
    tests_systems.cpp
    tests_multi_systems.cpp
    tests_views.cpp
    tests_view_vectors.cpp
    tests_frame_systems.cpp
    tests_gm_systems.cpp
    tests_recycling.cpp
    tests_tricky.cpp
)

add_executable(engine_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS})
target_link_libraries(engine_tests_run_bin PRIVATE criterion engine-system)

if (ENGINE_TESTS_SANITIZE AND NOT MSVC)
    target_compile_options(engine_tests_run_bin PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(engine_tests_run_bin PRIVATE -fsanitize=address,undefined)
endif ()

add_custom_target(engine_tests_run
        COMMAND $<TARGET_FILE:engine_tests_run_bin>
        DEPENDS engine_tests_run_bin
        WORKING_DIRECTORY $<TARGET_FILE_DIR:engine_tests_run_bin>
)
