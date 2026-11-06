find_package(Miniaudio REQUIRED)

set_source_files_properties(src/MiniaudioImpl.cpp PROPERTIES
    COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fvisibility=hidden;-w>"
)

add_library(engine-audio SHARED
    src/MiniaudioImpl.cpp
    src/Listener.cpp
    src/Sound.cpp
    src/Decoder.cpp
)

target_include_directories(engine-audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(engine-audio PUBLIC engine-system miniaudio::miniaudio)

install(TARGETS engine-audio
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)