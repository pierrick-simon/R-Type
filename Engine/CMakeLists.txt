cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_engine CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(RTYPE_FETCH_MISSING_DEPS "Download dependencies that are not found on the system" ON)

include(FetchContent)
find_package(Threads REQUIRED)

# ---- Lua: system first, else fetch ----
find_package(Lua 5.4 QUIET)
if (Lua_FOUND OR LUA_FOUND)
    set(RTYPE_LUA_TARGET Lua::Lua)
elseif (RTYPE_FETCH_MISSING_DEPS)
    FetchContent_Declare(lua
        GIT_REPOSITORY https://github.com/walterschell/Lua.git
        GIT_TAG        v5.4.7
        GIT_SHALLOW    TRUE
    )
    set(LUA_ENABLE_SHARED  OFF CACHE BOOL "" FORCE)
    set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(LUA_BUILD_COMPILER OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(lua)
    set(RTYPE_LUA_TARGET lua_static)
else ()
    message(FATAL_ERROR "Lua not found and RTYPE_FETCH_MISSING_DEPS=OFF")
endif ()

# ---- raylib: system first, else fetch ----
find_package(raylib 5.5 QUIET CONFIG)
if (NOT raylib_FOUND AND RTYPE_FETCH_MISSING_DEPS)
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.5
        GIT_SHALLOW    TRUE
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
elseif (NOT raylib_FOUND)
    message(FATAL_ERROR "raylib not found")
endif ()

# ---- Header-only deps via Find modules ----
find_package(Asio REQUIRED)
find_package(PlfColony REQUIRED)
find_package(RaylibCpp REQUIRED)
find_package(Miniaudio REQUIRED)
find_package(Sol2 REQUIRED)

# Create System engine lib

set(SYSTEM_LIB_NAME engine-system)
set(SYSTEM System)

set(SRCS_SYSTEM
    ${SYSTEM}/src/GameManager.cpp
    ${SYSTEM}/src/Entity.cpp
    ${SYSTEM}/src/ComponentManager.cpp
    ${SYSTEM}/src/SystemManager.cpp
    ${SYSTEM}/src/BuiltIn/Clock.cpp
    ${SYSTEM}/src/BuiltIn/Velocity2D.cpp
    ${SYSTEM}/src/BuiltIn/Friction2D.cpp
    ${SYSTEM}/src/BuiltIn/GravityField2D.cpp
)

set(INCLUDE_SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include/Components
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include/System
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include/BuiltIn
)

add_library(${SYSTEM_LIB_NAME} SHARED ${SRCS_SYSTEM})
target_include_directories(${SYSTEM_LIB_NAME} PUBLIC ${INCLUDE_SYSTEM})
target_link_libraries(${SYSTEM_LIB_NAME} PUBLIC plf::colony)

# Create Graphical2D engine lib

set(GRAPHICAL2D_LIB_NAME engine-graphical2D)
set(GRAPHICAL2D Graphical2D)

set(SRCS_GRAPHICAL2D
    ${GRAPHICAL2D}/src/Window.cpp
    ${GRAPHICAL2D}/src/Text2D.cpp
    ${GRAPHICAL2D}/src/Sprite2D.cpp
    ${GRAPHICAL2D}/src/SpriteSheet2D.cpp
    ${GRAPHICAL2D}/src/Font.cpp
    ${GRAPHICAL2D}/src/Texture2D.cpp
    ${GRAPHICAL2D}/src/KeyboardManager.cpp
    ${GRAPHICAL2D}/src/MouseManager.cpp
    ${GRAPHICAL2D}/src/Keybind.cpp
)

add_library(${GRAPHICAL2D_LIB_NAME} SHARED ${SRCS_GRAPHICAL2D})
target_include_directories(${GRAPHICAL2D_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${GRAPHICAL2D}/include
)
target_link_libraries(${GRAPHICAL2D_LIB_NAME} PUBLIC engine-system raylib-cpp::raylib-cpp)

if (MSVC)
    target_compile_options(${GRAPHICAL2D_LIB_NAME} PRIVATE /wd4251)
endif ()

# Create Graphical3D engine lib

set(GRAPHICAL3D_LIB_NAME engine-graphical3D)
set(GRAPHICAL3D Graphical3D)

set(SRCS_GRAPHICAL3D
    ${GRAPHICAL3D}/src/Camera3D.cpp
    ${GRAPHICAL3D}/src/Model3D.cpp
    ${GRAPHICAL3D}/src/Sprite3D.cpp
    ${GRAPHICAL3D}/src/ModelAnimation3D.cpp
    ${GRAPHICAL3D}/src/SpriteSheet3D.cpp
    ${GRAPHICAL3D}/src/Draw3D.cpp
)

add_library(${GRAPHICAL3D_LIB_NAME} SHARED ${SRCS_GRAPHICAL3D})
target_include_directories(${GRAPHICAL3D_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${GRAPHICAL3D}/include
)
target_link_libraries(${GRAPHICAL3D_LIB_NAME} PUBLIC engine-system engine-graphical2D raylib-cpp::raylib-cpp)

if (MSVC)
    target_compile_options(${GRAPHICAL3D_LIB_NAME} PRIVATE /wd4251)
endif ()

# Create Network engine lib

set(NETWORK_LIB_NAME engine-network)
set(NETWORK Network)

set(SRCS_NETWORK
    ${NETWORK}/src/ClientUDP.cpp
    ${NETWORK}/src/ServerUDP.cpp
    ${NETWORK}/src/Utils.cpp
    ${NETWORK}/src/Lagometer.cpp
)

add_library(${NETWORK_LIB_NAME} SHARED ${SRCS_NETWORK})
target_include_directories(${NETWORK_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${NETWORK}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/${NETWORK}/include/BuiltIn
)
target_link_libraries(${NETWORK_LIB_NAME} PUBLIC engine-system asio::asio)

# Create Audio engine lib

set(AUDIO_LIB_NAME engine-audio)
set(AUDIO Audio)

set(SRCS_AUDIO
    ${AUDIO}/src/MiniaudioImpl.cpp
    ${AUDIO}/src/Listener.cpp
    ${AUDIO}/src/Sound.cpp
    ${AUDIO}/src/Decoder.cpp
)

set_source_files_properties(${AUDIO}/src/MiniaudioImpl.cpp PROPERTIES
    COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden;-w>"
)

add_library(${AUDIO_LIB_NAME} SHARED ${SRCS_AUDIO})
target_include_directories(${AUDIO_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${AUDIO}/include
)
target_link_libraries(${AUDIO_LIB_NAME} PUBLIC engine-system miniaudio::miniaudio)

# Create Game engine binary

set(GAME_BIN_NAME game)
set(GAME Game)

set(SRCS_GAME
    ${GAME}/src/Main.cpp
    ${GAME}/src/Runner.cpp
    ${GAME}/src/ConfigBuilder.cpp
    ${GAME}/src/Modules.cpp
    ${GAME}/src/Scripts.cpp
    ${GAME}/src/GameManagerBinding.cpp
    ${GAME}/src/ComponentBinding.cpp
    ${GAME}/src/SystemBinding.cpp
    ${GAME}/src/ViewBinding.cpp
    ${GAME}/src/InputBinding.cpp
    ${GAME}/src/TimeBinding.cpp
    ${GAME}/src/WindowBinding.cpp
    ${GAME}/src/EngineBindings.cpp
)

add_executable(${GAME_BIN_NAME} ${SRCS_GAME} ${SRCS_SHARED})
target_include_directories(${GAME_BIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${GAME}/include
)
target_link_libraries(${GAME_BIN_NAME} PRIVATE engine-system engine-graphical2D engine-graphical3D engine-audio sol2::sol2)

install(TARGETS ${SYSTEM_LIB_NAME} ${GRAPHICAL2D_LIB_NAME} ${GRAPHICAL3D_LIB_NAME} ${NETWORK_LIB_NAME} ${AUDIO_LIB_NAME} ${GAME_BIN_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)