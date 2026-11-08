foreach (dep engine-graphical2D engine-graphical3D engine-audio)
    if (NOT TARGET ${dep})
        message(FATAL_ERROR "Game requires ${dep} (enable the matching RTYPE_BUILD_* option)")
    endif ()
endforeach ()

include(RTypeDependencies)
rtype_find_lua()
find_package(Sol2 REQUIRED)

add_executable(game
    src/Main.cpp
    src/Runner.cpp
    src/ConfigBuilder.cpp
    src/Modules.cpp
    src/Scripts.cpp
    src/GameManagerBinding.cpp
    src/ComponentBinding.cpp
    src/SystemBinding.cpp
    src/ViewBinding.cpp
    src/InputBinding.cpp
    src/TimeBinding.cpp
    src/WindowBinding.cpp
    src/AudioBinding.cpp
    src/EngineBindings.cpp
    src/Console.cpp
)

target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(game PRIVATE
    engine-system engine-graphical2D engine-graphical3D engine-audio engine-physics
    sol2::sol2 rtype::lua
)

install(TARGETS game
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)