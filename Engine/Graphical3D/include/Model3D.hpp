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
    /**
     * @brief 3D model component built on top of raylib::Model.
     */
    struct Model3D : raylib::Model {
        /**
         * @brief Loads a 3D model from a file.
         * @param path Path to the model file.
         */
        Model3D(const std::string &path);

        /**
         * @brief System unloading all the Model3D components.
         * @param gm Game manager owning the models to unload.
         */
        static void unloadModels3D(GameManager &gm);
    };
} // namespace RType
#endif /* !MODEL3D_HPP_ */
