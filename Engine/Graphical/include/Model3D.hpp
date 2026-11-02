/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Model3D
*/

#ifndef MODEL3D_HPP_
#define MODEL3D_HPP_

    #include <raylib-cpp.hpp>

namespace RType {
    struct Model3D : raylib::Model {
        Model3D(const std::string &path);
    };
} // namespace RType
#endif /* !MODEL3D_HPP_ */
