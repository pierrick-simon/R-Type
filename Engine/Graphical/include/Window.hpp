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

    class Window : public raylib::Window {
    public:
        Window(int width = 800, int height = 600,
            const std::string &name = "Raylib Window",
            raylib::Color clearScreenColor = raylib::Color::Black());
        raylib::Color clearScreenColor = raylib::Color::Black();

        static void systemWindow(Window &w);

    private:
        raylib::AudioDevice _audioDevice;
    };

} // namespace RType

#endif /* !WINDOW_HPP_ */
