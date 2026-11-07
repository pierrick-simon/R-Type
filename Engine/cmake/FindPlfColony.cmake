include(FindPackageHandleStandardArgs)

find_path(PlfColony_INCLUDE_DIR NAMES plf_colony.h)

if (NOT PlfColony_INCLUDE_DIR AND RTYPE_FETCH_MISSING_DEPS)
    include(FetchContent)
    FetchContent_Declare(plf-colony
        GIT_REPOSITORY https://github.com/mattreecebentley/plf_colony.git
        GIT_TAG        899f576280cd038cabb15935edd7304866e93501
        SOURCE_SUBDIR  _no_cmake
    )
    FetchContent_MakeAvailable(plf-colony)
    set(PlfColony_INCLUDE_DIR ${plf-colony_SOURCE_DIR} CACHE PATH "" FORCE)
endif ()

find_package_handle_standard_args(PlfColony REQUIRED_VARS PlfColony_INCLUDE_DIR)

if (PlfColony_FOUND AND NOT TARGET plf::colony)
    add_library(plf::colony INTERFACE IMPORTED)
    target_include_directories(plf::colony SYSTEM INTERFACE ${PlfColony_INCLUDE_DIR})
endif ()