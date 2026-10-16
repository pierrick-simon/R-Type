cmake_minimum_required(VERSION 3.25...4.0)
project(r-type_engine CXX)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
find_package(Threads REQUIRED)

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-34-2
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _no_cmake
)

FetchContent_Declare(plf-colony
    GIT_REPOSITORY https://github.com/mattreecebentley/plf_colony.git
    GIT_TAG        899f576280cd038cabb15935edd7304866e93501
    SOURCE_SUBDIR  _no_cmake
)

FetchContent_Declare(raylib-cpp
    GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
    GIT_TAG        v5.5.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _no_cmake
)

FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(asio plf-colony raylib-cpp)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# --- Asio ---
add_library(asio INTERFACE)
add_library(asio::asio ALIAS asio)
target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_link_libraries(asio INTERFACE Threads::Threads)
if (WIN32)
    target_compile_definitions(asio INTERFACE _WIN32_WINNT=0x0A00)
    target_link_libraries(asio INTERFACE ws2_32 mswsock)
endif ()

# --- plf_colony ---
add_library(plf-colony INTERFACE)
target_include_directories(plf-colony SYSTEM INTERFACE ${plf-colony_SOURCE_DIR})

# --- raylib-cpp (header-only wrapper, needs raylib) ---
add_library(raylib-cpp INTERFACE)
add_library(raylib-cpp::raylib-cpp ALIAS raylib-cpp)
target_include_directories(raylib-cpp SYSTEM INTERFACE ${raylib-cpp_SOURCE_DIR}/include)
target_link_libraries(raylib-cpp INTERFACE raylib)

# Create System engine lib

set(SYSTEM_LIB_NAME engine-system)
set(SYSTEM System)

set(SRCS_SYSTEM
    ${SYSTEM}/src/GameManager.cpp
    ${SYSTEM}/src/Entity.cpp
    ${SYSTEM}/src/ComponentManager.cpp
    ${SYSTEM}/src/SystemManager.cpp
    ${SYSTEM}/src/BuiltIn/Clock.cpp
)

set(INCLUDE_SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include/Components
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include/System
    ${CMAKE_CURRENT_SOURCE_DIR}/${SYSTEM}/include/BuiltIn
)

add_library(${SYSTEM_LIB_NAME} SHARED ${SRCS_SYSTEM} ${SRCS_SHARED})
target_include_directories(${SYSTEM_LIB_NAME} PUBLIC ${INCLUDE_SYSTEM})
target_link_libraries(${SYSTEM_LIB_NAME} PUBLIC plf-colony)

# Create Network engine lib

set(NETWORK_LIB_NAME engine-network)
set(NETWORK Network)

set(SRCS_NETWORK
    ${NETWORK}/src/Empty.cpp
)

add_library(${NETWORK_LIB_NAME} SHARED ${SRCS_NETWORK} ${SRCS_SHARED})
target_include_directories(${NETWORK_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${NETWORK}/include
)
target_link_libraries(${NETWORK_LIB_NAME} PUBLIC engine-system asio::asio)

# Create Graphical engine lib

set(GRAPHICAL_LIB_NAME engine-graphical)
set(GRAPHICAL Graphical)

set(SRCS_GRAPHICAL
    ${GRAPHICAL}/src/Window.cpp
    ${GRAPHICAL}/src/Text2D.cpp
    ${GRAPHICAL}/src/Sprite2D.cpp
    ${GRAPHICAL}/src/SpriteSheet2D.cpp
    ${GRAPHICAL}/src/Sound.cpp
    ${GRAPHICAL}/src/Font.cpp
    ${GRAPHICAL}/src/Texture2D.cpp
    ${GRAPHICAL}/src/KeyBoard.cpp
    ${GRAPHICAL}/src/Mouse.cpp
)

add_library(${GRAPHICAL_LIB_NAME} SHARED ${SRCS_GRAPHICAL} ${SRCS_SHARED})
target_include_directories(${GRAPHICAL_LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${GRAPHICAL}/include
)
target_link_libraries(${GRAPHICAL_LIB_NAME} PUBLIC engine-system raylib-cpp::raylib-cpp)

if (MSVC)
    target_compile_options(${GRAPHICAL_LIB_NAME} PRIVATE /wd4251)
endif ()