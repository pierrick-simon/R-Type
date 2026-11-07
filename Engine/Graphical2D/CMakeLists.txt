include(RTypeDependencies)
rtype_find_raylib()
find_package(RaylibCpp REQUIRED)

add_library(engine-graphical2D SHARED
    src/Window.cpp
    src/Text2D.cpp
    src/Sprite2D.cpp
    src/SpriteSheet2D.cpp
    src/Font.cpp
    src/Texture2D.cpp
    src/KeyboardManager.cpp
    src/MouseManager.cpp
    src/Keybind.cpp
)

target_include_directories(engine-graphical2D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(engine-graphical2D PUBLIC engine-system raylib-cpp::raylib-cpp)

if (MSVC)
    target_compile_options(engine-graphical2D PRIVATE /wd4251)
endif ()

install(TARGETS engine-graphical2D
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)