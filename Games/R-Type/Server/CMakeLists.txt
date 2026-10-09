cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAIN
        src/Main.cpp
)

set(SRCS
)

add_executable(r-type_server ${MAIN} ${SRCS})
target_include_directories(r-type_server PRIVATE include)
target_link_libraries(r-type_server PRIVATE shared)

# criterion
if (TARGET criterion)
    set(SRCS_TESTS tests/tests_init.cpp)

    add_executable(server_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS})
    target_include_directories(server_tests_run_bin PRIVATE include)
    target_compile_options(server_tests_run_bin PRIVATE --coverage)
    target_link_options(server_tests_run_bin PRIVATE --coverage)
    target_link_libraries(server_tests_run_bin PRIVATE criterion shared)

    add_custom_target(server_tests_run
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            COMMAND $<TARGET_FILE:server_tests_run_bin>
            DEPENDS server_tests_run_bin
            WORKING_DIRECTORY $<TARGET_FILE_DIR:server_tests_run_bin>
    )
endif ()
