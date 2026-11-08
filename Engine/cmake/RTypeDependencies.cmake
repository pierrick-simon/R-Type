include_guard(GLOBAL)
include(FetchContent)

# ---- raylib: system first, else fetch ----
macro(rtype_find_raylib)
    if (NOT TARGET raylib)
        set(_rtype_use_system_raylib FALSE)
        find_path(RTYPE_SYSTEM_RAYLIB_INCLUDE raylib.h)
        if (RTYPE_SYSTEM_RAYLIB_INCLUDE)
            file(STRINGS "${RTYPE_SYSTEM_RAYLIB_INCLUDE}/raylib.h"
                _rtype_raylib_version
                REGEX "^#define RAYLIB_VERSION_(MAJOR|MINOR) +[0-9]+")
            string(REGEX MATCH "MAJOR +([0-9]+)" _ "${_rtype_raylib_version}")
            set(_rtype_raylib_major "${CMAKE_MATCH_1}")
            string(REGEX MATCH "MINOR +([0-9]+)" _ "${_rtype_raylib_version}")
            set(_rtype_raylib_minor "${CMAKE_MATCH_1}")
            if ("${_rtype_raylib_major}.${_rtype_raylib_minor}" STREQUAL "5.5")
                set(_rtype_use_system_raylib TRUE)
            else ()
                message(STATUS "System raylib is "
                    "${_rtype_raylib_major}.${_rtype_raylib_minor}, not 5.5: "
                    "downloading raylib 5.5")
            endif ()
        endif ()
        if (_rtype_use_system_raylib)
            find_package(raylib 5.5 QUIET CONFIG)
        endif ()
        if (_rtype_use_system_raylib AND raylib_FOUND)
            get_target_property(_rtype_raylib_imported raylib IMPORTED)
            if (_rtype_raylib_imported)
                set_property(TARGET raylib PROPERTY IMPORTED_GLOBAL TRUE)
            endif ()
        elseif (RTYPE_FETCH_MISSING_DEPS)
            FetchContent_Declare(raylib
                GIT_REPOSITORY https://github.com/raysan5/raylib.git
                GIT_TAG        5.5
                GIT_SHALLOW    TRUE
            )
            set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(raylib)
            set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        else ()
            message(FATAL_ERROR "raylib 5.5 not found and RTYPE_FETCH_MISSING_DEPS=OFF")
        endif ()
    endif ()
endmacro()

# ---- Lua: system first, else fetch. Exposes the global target rtype::lua ----
macro(rtype_find_lua)
    if (NOT TARGET rtype::lua)
        find_package(Lua 5.4 QUIET)
        if (LUA_FOUND AND TARGET Lua::Lua)
            set_property(TARGET Lua::Lua PROPERTY IMPORTED_GLOBAL TRUE)
            add_library(rtype::lua INTERFACE IMPORTED GLOBAL)
            target_link_libraries(rtype::lua INTERFACE Lua::Lua)
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
            add_library(rtype::lua INTERFACE IMPORTED GLOBAL)
            target_link_libraries(rtype::lua INTERFACE lua_static)
        else ()
            message(FATAL_ERROR "Lua not found and RTYPE_FETCH_MISSING_DEPS=OFF")
        endif ()
    endif ()
endmacro()