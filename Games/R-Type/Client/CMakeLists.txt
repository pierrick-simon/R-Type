cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_client CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT TARGET r-type_shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Shared ${CMAKE_BINARY_DIR}/Shared)
endif ()

set(MAIN
        src/Main.cpp
)

set(SRCS
    src/Setup.cpp
)

add_executable(r-type_client ${MAIN} ${SRCS})
target_include_directories(r-type_client PRIVATE include)
target_link_libraries(r-type_client PRIVATE r-type::shared engine-system engine-network engine-graphical)

# criterion
if (TARGET criterion)
    set(SRCS_TESTS tests/tests_init.cpp)

    add_executable(client_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS})
    target_include_directories(client_tests_run_bin PRIVATE include)
    target_link_libraries(client_tests_run_bin PRIVATE criterion r-type::shared engine-system engine-network engine-graphical)

    # --coverage and `find` are GCC/Clang + Unix only
    if (NOT MSVC)
        target_compile_options(client_tests_run_bin PRIVATE --coverage)
        target_link_options(client_tests_run_bin PRIVATE --coverage)
    endif ()

    if (WIN32)
        set(_CLEAN_GCDA COMMAND ${CMAKE_COMMAND} -E echo "skipping gcda cleanup on Windows")
    else ()
        set(_CLEAN_GCDA COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete)
    endif ()

    add_custom_target(client_tests_run
            ${_CLEAN_GCDA}
            COMMAND $<TARGET_FILE:client_tests_run_bin>
            DEPENDS client_tests_run_bin
            WORKING_DIRECTORY $<TARGET_FILE_DIR:client_tests_run_bin>
    )
endif ()