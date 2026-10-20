cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MAIN
        src/Main.cpp
)

set(SRCS
        src/Enemies/EnemyManager.cpp
        src/Enemies/EnemySystems.cpp
)

set(SRCS_GFX
        src/Enemies/EnemyVisuals.cpp
)

set(GAME_SRCS
        src/Assets.cpp
        src/Background.cpp
        src/Bullet.cpp
        src/Player.cpp
        src/Setup.cpp
        src/Texture.cpp
)

add_executable(r-type_server ${MAIN} ${SRCS} ${GAME_SRCS} ${SRCS_GFX})
target_include_directories(r-type_server PRIVATE include)
target_link_libraries(r-type_server PRIVATE engine-system engine-graphical)

if (TARGET criterion AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_init.cpp")
    set(SRCS_TESTS tests/tests_init.cpp)

    add_executable(server_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS})
    target_include_directories(server_tests_run_bin PRIVATE include)
    target_compile_options(server_tests_run_bin PRIVATE --coverage)
    target_link_options(server_tests_run_bin PRIVATE --coverage)
    target_link_libraries(server_tests_run_bin PRIVATE criterion engine-system engine-network engine-graphical)

    add_custom_target(server_tests_run
            COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
            COMMAND $<TARGET_FILE:server_tests_run_bin>
            DEPENDS server_tests_run_bin
            WORKING_DIRECTORY $<TARGET_FILE_DIR:server_tests_run_bin>
    )
endif ()
