add_library(engine-physics SHARED
    src/Friction2D.cpp
    src/GravityField2D.cpp
    src/Velocity2D.cpp
    src/Friction3D.cpp
    src/GravityField3D.cpp
    src/Velocity3D.cpp
    src/AngularVelocity2D.cpp
    src/AngularVelocity3D.cpp
)

target_include_directories(engine-physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(engine-physics PUBLIC engine-system)

install(TARGETS engine-physics
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)