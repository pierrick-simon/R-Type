/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Window
*/

#ifndef WINDOW_HPP_
    #define WINDOW_HPP_

    #include <raylib-cpp.hpp>

namespace RType {
    
    struct Window
    {
        std::size_t width = 800;
        std::size_t height = 600;
        std::string name = "Raylib Window";
        raylib::Color clearScreenColor = raylib::Color::Black();
        raylib::Window window = raylib::Window(width, height, name);
        
        static void systemWindow(Window &w);
    };

}

#endif /* !WINDOW_HPP_ */
