cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_SHARED ${CMAKE_CURRENT_SOURCE_DIR}/Shared/include)

set(SRCS_SHARED
)

add_library(shared INTERFACE)
target_include_directories(shared INTERFACE ${INCLUDE_SHARED})

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
