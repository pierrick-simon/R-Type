cmake_minimum_required(VERSION 3.25...4.0)
project(R-Type CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RTYPE_OUT ${CMAKE_SOURCE_DIR}/Games)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RTYPE_OUT})
foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${RTYPE_OUT})
endforeach()

if (MSVC)
    add_compile_options(/utf-8 /permissive- /Zc:__cplusplus)
endif ()

add_subdirectory(Engine)
add_subdirectory(Games)
