include(FindPackageHandleStandardArgs)

find_path(RaylibCpp_INCLUDE_DIR NAMES raylib-cpp.hpp)

if (NOT RaylibCpp_INCLUDE_DIR AND RTYPE_FETCH_MISSING_DEPS)
    include(FetchContent)
    FetchContent_Declare(raylib-cpp
        GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
        GIT_TAG        v5.5.0
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  _no_cmake
    )
    FetchContent_MakeAvailable(raylib-cpp)
    set(RaylibCpp_INCLUDE_DIR ${raylib-cpp_SOURCE_DIR}/include CACHE PATH "" FORCE)
endif ()

find_package_handle_standard_args(RaylibCpp REQUIRED_VARS RaylibCpp_INCLUDE_DIR)

if (RaylibCpp_FOUND AND NOT TARGET raylib-cpp::raylib-cpp)
    add_library(raylib-cpp::raylib-cpp INTERFACE IMPORTED)
    target_include_directories(raylib-cpp::raylib-cpp SYSTEM INTERFACE ${RaylibCpp_INCLUDE_DIR})
    target_link_libraries(raylib-cpp::raylib-cpp INTERFACE raylib)
endif ()