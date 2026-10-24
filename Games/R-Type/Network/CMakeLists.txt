cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_network CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(NETWORK_LIB_NAME r-type_network)
set(NETWORK )

set(SRCS_NETWORK
    src/ServerManager.cpp
    src/ClientManager.cpp
    src/Protocol/KeyboardProtocol.cpp
    src/Protocol/MouseProtocol.cpp
    src/Protocol/CreateProtocol.cpp
    src/Protocol/MoveProtocol.cpp
    src/Protocol/SetProtocol.cpp
    src/Protocol/SoundProtocol.cpp
    src/Protocol/TextProtocol.cpp
    src/Protocol/DestroyProtocol.cpp
    src/Protocol/UpdateClientEntity.cpp
)


set(INCLUDE_NETWORK
    include/
    include/Protocol/
)

add_library(${NETWORK_LIB_NAME} STATIC ${SRCS_NETWORK})
add_library(r-type::network ALIAS ${NETWORK_LIB_NAME})
target_include_directories(${NETWORK_LIB_NAME} PUBLIC ${INCLUDE_NETWORK})
target_link_libraries(${NETWORK_LIB_NAME} PUBLIC engine-system engine-graphical engine-network)

if (MSVC)
    target_compile_options(${NETWORK_LIB_NAME} PRIVATE /W4 /permissive- /wd4251)
    target_compile_definitions(${NETWORK_LIB_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif ()