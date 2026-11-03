/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Draw3D
*/

#ifndef DRAW3D_HPP_
#define DRAW3D_HPP_

#include "Camera3D.hpp"
#include "GameManager.hpp"
#include "Window.hpp"

namespace RType {
    /**
     * @brief Groups the systems responsible for the 3D rendering pass.
     */
    struct Draw3D {
        /**
         * @brief System running the 3D drawing pass.
         * @param gm Game manager giving access to the entities and
         * components.
         * @param window Window the scene is rendered to.
         */
        static void systemDraw3D(GameManager &gm, Window &window);
    };
} // namespace RType

#endif /* !DRAW3D_HPP_ */
