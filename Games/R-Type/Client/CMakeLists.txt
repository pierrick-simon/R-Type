cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_client CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAIN
        src/Main.cpp
)

set(SRCS
)

add_executable(r-type_client ${MAIN} ${SRCS})
target_include_directories(r-type_client PRIVATE include)
target_link_libraries(r-type_client PRIVATE shared)

# criterion
if (TARGET criterion)
    set(SRCS_TESTS tests/tests_init.cpp)

    add_executable(client_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS})
    target_include_directories(client_tests_run_bin PRIVATE include)
    target_compile_options(client_tests_run_bin PRIVATE --coverage)
    target_link_options(client_tests_run_bin PRIVATE --coverage)
    target_link_libraries(client_tests_run_bin PRIVATE criterion shared)

    add_custom_target(client_tests_run
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            COMMAND $<TARGET_FILE:client_tests_run_bin>
            DEPENDS client_tests_run_bin
            WORKING_DIRECTORY $<TARGET_FILE_DIR:client_tests_run_bin>
    )
endif ()
