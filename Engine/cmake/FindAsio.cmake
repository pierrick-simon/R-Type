include(FindPackageHandleStandardArgs)

find_path(Asio_INCLUDE_DIR NAMES asio.hpp)

if (NOT Asio_INCLUDE_DIR AND RTYPE_FETCH_MISSING_DEPS)
    include(FetchContent)
    FetchContent_Declare(asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG        asio-1-34-2
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  _no_cmake
    )
    FetchContent_MakeAvailable(asio)
    set(Asio_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE PATH "" FORCE)
endif ()

find_package_handle_standard_args(Asio REQUIRED_VARS Asio_INCLUDE_DIR)

if (Asio_FOUND AND NOT TARGET asio::asio)
    find_package(Threads REQUIRED)
    add_library(asio::asio INTERFACE IMPORTED)
    target_include_directories(asio::asio SYSTEM INTERFACE ${Asio_INCLUDE_DIR})
    target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
    target_link_libraries(asio::asio INTERFACE Threads::Threads)
    if (WIN32)
        target_compile_definitions(asio::asio INTERFACE _WIN32_WINNT=0x0A00)
        target_link_libraries(asio::asio INTERFACE ws2_32 mswsock)
    endif ()
endif ()