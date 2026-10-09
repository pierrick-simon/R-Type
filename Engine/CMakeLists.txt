cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_engine CXX)

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
target_link_libraries(${NETWORK_LIB_NAME} PRIVATE shared)

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
target_link_libraries(${GRAPHICAL_LIB_NAME} PRIVATE shared)

set_target_properties(${GRAPHICAL_LIB_NAME} PROPERTIES PREFIX "")

set_target_properties(${GRAPHICAL_LIB_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
