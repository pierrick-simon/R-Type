cmake_minimum_required(VERSION 4.4)
project(r-type_server CXX)

set(CMAKE_CXX_STANDARD 26)
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
find_library(CRITERION_LIBRARY criterion)
find_path(CRITERION_INCLUDE_DIR criterion/criterion.h)

if (CRITERION_LIBRARY AND CRITERION_INCLUDE_DIR)
    add_library(criterion UNKNOWN IMPORTED)
    set_target_properties(criterion PROPERTIES
            IMPORTED_LOCATION ${CRITERION_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${CRITERION_INCLUDE_DIR}
    )
else ()
    if (NOT DEFINED CRITERION_VERSION)
        set(CRITERION_VERSION v2.4.2)
    endif ()
    include(FetchContent)
    FetchContent_Declare(
            criterion
            GIT_REPOSITORY https://github.com/Snaipe/Criterion.git
            GIT_TAG ${CRITERION_VERSION}
    )
    FetchContent_MakeAvailable(criterion)
endif ()

set(SRCS_TESTS
        tests/tests_init.cpp
)

add_executable(server_tests_run_bin EXCLUDE_FROM_ALL ${SRCS_TESTS} ${SRCS})
target_include_directories(server_tests_run_bin PRIVATE include)
target_compile_options(server_tests_run_bin PRIVATE --coverage)
target_link_options(server_tests_run_bin PRIVATE --coverage)
target_link_libraries(server_tests_run_bin PRIVATE criterion shared)

add_custom_target(server_tests_run
        COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
        COMMAND ./server_tests_run_bin
        DEPENDS server_tests_run_bin
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
