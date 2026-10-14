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

    struct Font : public raylib::Font {
        Font(const std::string &path);
    };

} // namespace RType
#endif /* !FONT_HPP_ */
