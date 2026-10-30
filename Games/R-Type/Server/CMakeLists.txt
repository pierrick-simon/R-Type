cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT TARGET r-type_shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Shared ${CMAKE_BINARY_DIR}/Shared)
endif ()

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
        src/Animation.cpp
        src/Background.cpp
        src/Bullet.cpp
        src/Player.cpp
        src/Setup.cpp
        src/Terrain.cpp
        src/Texture.cpp
)

add_executable(r-type_server ${MAIN} ${SRCS} ${GAME_SRCS} ${SRCS_GFX})
target_include_directories(r-type_server PRIVATE include)
target_link_libraries(r-type_server PRIVATE r-type::shared engine-system engine-graphical engine-network)

if (TARGET criterion)
    set(SRCS_TESTS tests/tests_init.cpp)

    add_executable(server_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS} ${GAME_SRCS} ${SRCS_GFX})
    target_include_directories(server_tests_run_bin PRIVATE include)
    target_link_libraries(server_tests_run_bin PRIVATE criterion r-type::shared engine-system engine-network engine-graphical)

    # --coverage and `find` are GCC/Clang + Unix only
    if (NOT MSVC)
        target_compile_options(server_tests_run_bin PRIVATE --coverage)
        target_link_options(server_tests_run_bin PRIVATE --coverage)
    endif ()

    if (WIN32)
        set(_CLEAN_GCDA COMMAND ${CMAKE_COMMAND} -E echo "skipping gcda cleanup on Windows")
    else ()
        set(_CLEAN_GCDA COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete)
    endif ()

    add_custom_target(server_tests_run
            ${_CLEAN_GCDA}
            COMMAND $<TARGET_FILE:server_tests_run_bin>
            DEPENDS server_tests_run_bin
            WORKING_DIRECTORY $<TARGET_FILE_DIR:server_tests_run_bin>
    )
endif ()