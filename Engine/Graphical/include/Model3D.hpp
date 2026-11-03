/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Model3D
*/

#ifndef MODEL3D_HPP_
#define MODEL3D_HPP_

    #include <raylib-cpp.hpp>
    #include "GameManager.hpp"

namespace RType {
    struct Model3D : raylib::Model {
        Model3D(const std::string &path);

        static void unloadModels3D(GameManager &gm);
    };
} // namespace RType
#endif /* !MODEL3D_HPP_ */
