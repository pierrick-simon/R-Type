find_package(PlfColony REQUIRED)

add_library(engine-system SHARED
    src/GameManager.cpp
    src/Entity.cpp
    src/ComponentManager.cpp
    src/SystemManager.cpp
    src/Clock.cpp
)

target_include_directories(engine-system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Components
    ${CMAKE_CURRENT_SOURCE_DIR}/include/System
    ${CMAKE_CURRENT_SOURCE_DIR}/include/BuiltIn
)
target_link_libraries(engine-system PUBLIC plf::colony)

install(TARGETS engine-system
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)