include(FindPackageHandleStandardArgs)

find_path(Miniaudio_INCLUDE_DIR NAMES miniaudio.h)

if (NOT Miniaudio_INCLUDE_DIR AND RTYPE_FETCH_MISSING_DEPS)
    include(FetchContent)
    FetchContent_Declare(miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio.git
        GIT_TAG        0.11.22
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  _no_cmake
    )
    FetchContent_MakeAvailable(miniaudio)
    set(Miniaudio_INCLUDE_DIR ${miniaudio_SOURCE_DIR} CACHE PATH "" FORCE)
endif ()

find_package_handle_standard_args(Miniaudio REQUIRED_VARS Miniaudio_INCLUDE_DIR)

if (Miniaudio_FOUND AND NOT TARGET miniaudio::miniaudio)
    find_package(Threads REQUIRED)
    add_library(miniaudio::miniaudio INTERFACE IMPORTED)
    target_include_directories(miniaudio::miniaudio SYSTEM INTERFACE ${Miniaudio_INCLUDE_DIR})
    target_link_libraries(miniaudio::miniaudio INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
    if (UNIX)
        target_link_libraries(miniaudio::miniaudio INTERFACE m)
    endif ()
endif ()