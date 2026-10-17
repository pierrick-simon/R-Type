cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_poc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(POC_SANITIZE "Build the Poc with AddressSanitizer and UBSan" ON)

set(SRCS
        src/Main.cpp
        src/Runner.cpp
        src/EdgeCases.cpp
        src/Simulation.cpp
        src/Systems.cpp
        src/MultiSystems.cpp
        src/Views.cpp
        src/Recycling.cpp
        src/Tricky.cpp
        src/ThrowTrace.cpp
)

add_executable(r-type_poc ${SRCS})
target_include_directories(r-type_poc PRIVATE
        include
        $<TARGET_PROPERTY:engine-system,INCLUDE_DIRECTORIES>
)
target_link_libraries(r-type_poc PRIVATE engine-system engine-system plf-colony)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
        AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
    set_target_properties(r-type_poc PROPERTIES CXX_STANDARD 23)
    target_compile_definitions(r-type_poc PRIVATE
            POC_STACKTRACE
            POC_SOURCE_ROOT="${CMAKE_SOURCE_DIR}/"
    )
    target_compile_options(r-type_poc PRIVATE -g)
    target_link_libraries(r-type_poc PRIVATE stdc++exp ${CMAKE_DL_LIBS})
endif ()

if (POC_SANITIZE AND NOT MSVC)
    target_compile_options(r-type_poc PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(r-type_poc PRIVATE -fsanitize=address,undefined)
endif ()
