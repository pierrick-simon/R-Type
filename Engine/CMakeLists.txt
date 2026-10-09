cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_engine CXX)

include(FetchContent)
find_package(Threads REQUIRED)

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-34-2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
add_library(asio::asio ALIAS asio)
target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_link_libraries(asio INTERFACE Threads::Threads)
if (WIN32)
    target_compile_definitions(asio INTERFACE _WIN32_WINNT=0x0A00)
    target_link_libraries(asio INTERFACE ws2_32 mswsock)
endif ()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
    FIND_PACKAGE_ARGS 5.5 CONFIG
)
FetchContent_MakeAvailable(raylib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compile Shared File

set(INCLUDE_SHARED ${CMAKE_CURRENT_SOURCE_DIR}/Shared/include)

set(SRCS_SHARED
)

add_library(shared INTERFACE)
target_include_directories(shared INTERFACE ${INCLUDE_SHARED})

# Create System engine lib

set(SYSTEM_LIB_NAME engine-system)
set(SYSTEM System)

set(SRCS_SYSTEM
    ${SYSTEM}/src/Empty.cpp
)

add_library(${SYSTEM_LIB_NAME} SHARED ${SRCS_SYSTEM} ${SRCS_SHARED})
target_include_directories(${SYSTEM_LIB_NAME} PRIVATE ${SYSTEM}/include)
target_link_libraries(${SYSTEM_LIB_NAME} PRIVATE shared)

set_target_properties(${SYSTEM_LIB_NAME} PROPERTIES PREFIX "")

set_target_properties(${SYSTEM_LIB_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Create Network engine lib

set(NETWORK_LIB_NAME engine-network)
set(NETWORK Network)

set(SRCS_NETWORK
    ${NETWORK}/src/Empty.cpp
)

add_library(${NETWORK_LIB_NAME} SHARED ${SRCS_NETWORK} ${SRCS_SHARED})
target_include_directories(${NETWORK_LIB_NAME} PRIVATE ${NETWORK}/include)
target_link_libraries(${NETWORK_LIB_NAME} PRIVATE shared asio::asio)

set_target_properties(${NETWORK_LIB_NAME} PROPERTIES PREFIX "")

set_target_properties(${NETWORK_LIB_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Create Graphical engine lib

set(GRAPHICAL_LIB_NAME engine-graphical)
set(GRAPHICAL Graphical)

set(SRCS_GRAPHICAL
    ${GRAPHICAL}/src/Empty.cpp
)

add_library(${GRAPHICAL_LIB_NAME} SHARED ${SRCS_GRAPHICAL} ${SRCS_SHARED})
target_include_directories(${GRAPHICAL_LIB_NAME} PRIVATE ${GRAPHICAL}/include)
target_link_libraries(${GRAPHICAL_LIB_NAME} PRIVATE shared raylib)

set_target_properties(${GRAPHICAL_LIB_NAME} PROPERTIES PREFIX "")

set_target_properties(${GRAPHICAL_LIB_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
