cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_common CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Game logic shared by the client and the server.
# No graphics here: it only depends on the ECS (engine-system).
set(SRCS
    src/EnemyFactory.cpp
    src/EnemySystems.cpp
)

add_library(rtype-common STATIC ${SRCS})
target_include_directories(rtype-common PUBLIC include)
target_link_libraries(rtype-common PUBLIC engine-system)

# criterion
if (TARGET criterion)
    set(SRCS_TESTS tests/test_enemies.cpp)

    add_executable(common_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS})
    target_include_directories(common_tests_run_bin PRIVATE include)
    target_compile_options(common_tests_run_bin PRIVATE --coverage)
    target_link_options(common_tests_run_bin PRIVATE --coverage)
    target_link_libraries(common_tests_run_bin PRIVATE criterion engine-system)

    add_custom_target(common_tests_run
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            COMMAND $<TARGET_FILE:common_tests_run_bin>
            DEPENDS common_tests_run_bin
            WORKING_DIRECTORY $<TARGET_FILE_DIR:common_tests_run_bin>
    )
endif ()
