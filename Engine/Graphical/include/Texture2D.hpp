/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Texture2D
*/

#ifndef TEXTURE2D_HPP_
#define TEXTURE2D_HPP_

    #include <raylib-cpp.hpp>

namespace RType {

    struct Texture2D : raylib::Texture2D {
        Texture2D(const std::string &path);
    };

} // namespace RType
#endif /* !TEXTURE2D_HPP_ */
