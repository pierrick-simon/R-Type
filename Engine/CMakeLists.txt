cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_engine CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL ON)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(GNUInstallDirs)

# ---- Options (engine-system is always built) ----
option(RTYPE_FETCH_MISSING_DEPS "Download dependencies that are not found on the system" ON)
option(RTYPE_BUILD_GRAPHICAL2D  "Build engine-graphical2D" ON)
option(RTYPE_BUILD_GRAPHICAL3D  "Build engine-graphical3D (requires GRAPHICAL2D)" ON)
option(RTYPE_BUILD_NETWORK      "Build engine-network" ON)
option(RTYPE_BUILD_AUDIO        "Build engine-audio" ON)
option(RTYPE_BUILD_GAME         "Build the game binary (requires GRAPHICAL2D, GRAPHICAL3D, AUDIO)" ON)

# ---- Resolve inter-module dependencies (order matters: Game first, then 3D) ----
if (RTYPE_BUILD_GAME)
    foreach (dep GRAPHICAL2D GRAPHICAL3D AUDIO)
        if (NOT RTYPE_BUILD_${dep})
            message(STATUS "RTYPE_BUILD_GAME needs RTYPE_BUILD_${dep}: enabling it")
            set(RTYPE_BUILD_${dep} ON)
        endif ()
    endforeach ()
endif ()

if (RTYPE_BUILD_GRAPHICAL3D AND NOT RTYPE_BUILD_GRAPHICAL2D)
    message(STATUS "RTYPE_BUILD_GRAPHICAL3D needs RTYPE_BUILD_GRAPHICAL2D: enabling it")
    set(RTYPE_BUILD_GRAPHICAL2D ON)
endif ()

# ---- Windows: put every DLL next to the executables ----
if (WIN32 AND PROJECT_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif ()

# ---- Modules ----
add_subdirectory(System)

if (RTYPE_BUILD_GRAPHICAL2D)
    add_subdirectory(Graphical2D)
endif ()
if (RTYPE_BUILD_GRAPHICAL3D)
    add_subdirectory(Graphical3D)
endif ()
if (RTYPE_BUILD_NETWORK)
    add_subdirectory(Network)
endif ()
if (RTYPE_BUILD_AUDIO)
    add_subdirectory(Audio)
endif ()
if (RTYPE_BUILD_GAME)
    add_subdirectory(Game)
endif ()

message(STATUS "r-type_engine: system=ON 2D=${RTYPE_BUILD_GRAPHICAL2D} 3D=${RTYPE_BUILD_GRAPHICAL3D} network=${RTYPE_BUILD_NETWORK} audio=${RTYPE_BUILD_AUDIO} game=${RTYPE_BUILD_GAME}")