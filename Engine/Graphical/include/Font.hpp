/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Font
*/

#ifndef FONT_HPP_
#define FONT_HPP_

    #include <raylib-cpp.hpp>

namespace RType {
    
    struct Font
    {
        std::string path = "";
        raylib::Font font = raylib::Font(path);
    };

}
#endif /* !FONT_HPP_ */
