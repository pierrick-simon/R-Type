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

    /**
     * @brief Game window, wrapper around raylib::Window.
     *
     * Also owns the audio device, so a Window must exist before any
     * Texture2D, Font or Sound is loaded.
     */
    class Window : public raylib::Window {
    public:
        /**
         * @brief Create the window and initialize the audio device.
         *
         * @param width  Width of the window, in pixels.
         * @param height Height of the window, in pixels.
         * @param name   Title of the window.
         * @param clearScreenColor Color used to clear the screen each frame.
         *
         * @throw EngineException If the window cannot be created.
         *
         * @warning Only one window can exist at a time (raylib limitation).
         */
        Window(int width = 800, int height = 600,
            const std::string &name = "Raylib Window",
            raylib::Color clearScreenColor = raylib::Color::Black());

        raylib::Color clearScreenColor = raylib::Color::Black();

        int fps = 120;

        /**
         * @brief System handling the window each frame.
         *
         * @param w The window to update.
         *
         * @warning Must be called once per frame; it delimits the drawing
         *          frame in which the drawing systems (Sprite2D, SpriteSheet2D,
         *          Text2D) are expected to run.
         */
        static void systemWindow(Window &w);

    private:
        raylib::AudioDevice _audioDevice;
    };

} // namespace RType

#endif /* !WINDOW_HPP_ */
