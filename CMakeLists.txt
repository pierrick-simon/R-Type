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

# Criterion (optional, Unix only): defined once for the whole project
if (NOT WIN32 AND NOT TARGET criterion)
    find_library(CRITERION_LIBRARY criterion)
    find_path(CRITERION_INCLUDE_DIR criterion/criterion.h)

    if (CRITERION_LIBRARY AND CRITERION_INCLUDE_DIR)
        add_library(criterion UNKNOWN IMPORTED GLOBAL)
        set_target_properties(criterion PROPERTIES
                IMPORTED_LOCATION ${CRITERION_LIBRARY}
                INTERFACE_INCLUDE_DIRECTORIES ${CRITERION_INCLUDE_DIR})
    else ()
        message(STATUS "Criterion not found or unsupported: tests disabled")
    endif ()
endif ()

add_subdirectory(Engine)
add_subdirectory(Games)
