if (NOT TARGET engine-graphical2D)
    message(FATAL_ERROR "Graphical3D requires Graphical2D (enable RTYPE_BUILD_GRAPHICAL2D)")
endif ()

include(RTypeDependencies)
rtype_find_raylib()
find_package(RaylibCpp REQUIRED)

add_library(engine-graphical3D SHARED
    src/Camera3D.cpp
    src/Model3D.cpp
    src/Sprite3D.cpp
    src/ModelAnimation3D.cpp
    src/SpriteSheet3D.cpp
    src/Draw3D.cpp
)

target_include_directories(engine-graphical3D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(engine-graphical3D PUBLIC engine-system engine-graphical2D raylib-cpp::raylib-cpp)

if (MSVC)
    target_compile_options(engine-graphical3D PRIVATE /wd4251)
endif ()

install(TARGETS engine-graphical3D
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)