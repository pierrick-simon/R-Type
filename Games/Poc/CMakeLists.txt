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
)

add_executable(r-type_poc ${SRCS})
target_include_directories(r-type_poc PRIVATE
        include
        $<TARGET_PROPERTY:engine-system,INCLUDE_DIRECTORIES>
)
target_link_libraries(r-type_poc PRIVATE engine-system shared plf-colony)

# if (POC_SANITIZE AND NOT MSVC)
#     target_compile_options(r-type_poc PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
#     target_link_options(r-type_poc PRIVATE -fsanitize=address,undefined)
# endif ()
