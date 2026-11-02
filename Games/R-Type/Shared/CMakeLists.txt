cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_shared CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SHARED_LIB_NAME r-type_shared)
set(SHARED )

set(SRCS_NETWORK
    src/Network/ServerManager.cpp
    src/Network/ClientManager.cpp
    src/Network/Protocol/KeyboardProtocol.cpp
    src/Network/Protocol/MouseProtocol.cpp
    src/Network/Protocol/CreateProtocol.cpp
    src/Network/Protocol/CreatedProtocol.cpp
    src/Network/Protocol/MoveProtocol.cpp
    src/Network/Protocol/SoundProtocol.cpp
    src/Network/Protocol/TextProtocol.cpp
    src/Network/Protocol/DestroyProtocol.cpp
    src/Network/Protocol/Update/SetProtocol.cpp
    src/Network/Protocol/Update/UpdateClientEntity.cpp
    src/Network/Protocol/Update/UpdatePosition2D.cpp
    src/Network/Protocol/Update/UpdateSprite2D.cpp
    src/Network/Protocol/Update/UpdateSpriteSheet2D.cpp
    src/Network/Protocol/Update/UpdateText2D.cpp
)

set(SRCS_ASSETS
    src/Assets/Assets.cpp
)

set(INCLUDE_SHARED
    include/
    include/Network
    include/Network/Protocol/
    include/Network/Protocol/Update/
    include/Assets
)

add_library(${SHARED_LIB_NAME} STATIC ${SRCS_NETWORK} ${SRCS_ASSETS})
add_library(r-type::shared ALIAS ${SHARED_LIB_NAME})
target_include_directories(${SHARED_LIB_NAME} PUBLIC ${INCLUDE_SHARED})
target_link_libraries(${SHARED_LIB_NAME} PUBLIC engine-system engine-graphical engine-network)

if (MSVC)
    target_compile_options(${SHARED_LIB_NAME} PRIVATE /W4 /permissive- /wd4251)
    target_compile_definitions(${SHARED_LIB_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif ()