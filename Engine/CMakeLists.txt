cmake_minimum_required(VERSION 4.4)
project(R-Type)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_SHARED
    Shared/include
)

set(SRCS_SHARED
)

# Shared headers, exposed to consumers outside Engine (e.g. Games)
add_library(shared INTERFACE)
target_include_directories(shared INTERFACE ${INCLUDE_SHARED})

# System
set(SYSTEM_LIB_NAME
    engine-system
)

set(SYSTEM
    System
)

set(SRCS_SYSTEM
    ${SYSTEM}/src/Empty.cpp
)

add_library(${SYSTEM_LIB_NAME} SHARED ${SRCS_SYSTEM} ${SRCS_SHARED})
target_include_directories(${SYSTEM_LIB_NAME} PRIVATE ${INCLUDE_SHARED} ${SYSTEM}/include/)
set_target_properties(${SYSTEM_LIB_NAME} PROPERTIES PREFIX "")

add_custom_target(move ALL
    DEPENDS ${SYSTEM_LIB_NAME}
)
