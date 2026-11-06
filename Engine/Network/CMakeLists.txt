find_package(Asio REQUIRED)

add_library(engine-network SHARED
    src/ClientUDP.cpp
    src/ServerUDP.cpp
    src/Utils.cpp
    src/Lagometer.cpp
)

target_include_directories(engine-network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/BuiltIn
)
target_link_libraries(engine-network PUBLIC engine-system asio::asio)

install(TARGETS engine-network
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)