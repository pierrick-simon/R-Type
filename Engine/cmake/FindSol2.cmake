include(FindPackageHandleStandardArgs)

find_path(Sol2_INCLUDE_DIR NAMES sol/sol.hpp)

if (NOT Sol2_INCLUDE_DIR AND RTYPE_FETCH_MISSING_DEPS)
    include(FetchContent)
    FetchContent_Declare(sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG        v3.5.0
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  _no_cmake
    )
    FetchContent_MakeAvailable(sol2)
    set(Sol2_INCLUDE_DIR ${sol2_SOURCE_DIR}/include CACHE PATH "" FORCE)
endif ()

find_package_handle_standard_args(Sol2 REQUIRED_VARS Sol2_INCLUDE_DIR)

if (Sol2_FOUND AND NOT TARGET sol2::sol2)
    add_library(sol2::sol2 INTERFACE IMPORTED GLOBAL)
    target_include_directories(sol2::sol2 SYSTEM INTERFACE ${Sol2_INCLUDE_DIR})
endif ()