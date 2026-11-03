/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ModelAnimation3D
*/

#ifndef MODELANIMATION3D_HPP_
#define MODELANIMATION3D_HPP_

#include <raylib-cpp.hpp>
#include <string>
#include <vector>

namespace RType {
    /**
     * @brief Holds the set of animations loaded for a 3D model.
     */
    class ModelAnimation3D {
    public:
        /**
         * @brief Loads the animations from a file.
         * @param path Path to the file containing the animations.
         */
        explicit ModelAnimation3D(const std::string &path);

        /**
         * @brief Applies an animation frame to a model.
         * @param model Model to animate.
         * @param index Index of the animation to use.
         * @param frame Frame of the animation to apply.
         */
        void update(const raylib::Model &model, std::size_t index, int frame);

        /**
         * @brief Gets the number of loaded animations.
         * @return Number of animations.
         */
        [[nodiscard]] std::size_t count() const
        {
            return _animations.size();
        }

        /**
         * @brief Gets the number of frames of an animation.
         * @param index Index of the animation.
         * @return Number of frames of the animation.
         */
        [[nodiscard]] int frameCount(std::size_t index) const;

        /**
         * @brief Gets an animation by index.
         * @param index Index of the animation (wrapped around the number of
         * animations).
         * @return Reference to the requested animation.
         */
        [[nodiscard]] const raylib::ModelAnimation &getAnimation(
            int index) const
        {
            return _animations[index % _animations.size()];
        }

    private:
        std::vector<raylib::ModelAnimation> _animations;
    };
} // namespace RType
#endif /* !MODELANIMATION3D_HPP_ */
