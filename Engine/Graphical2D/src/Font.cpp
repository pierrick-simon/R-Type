/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Font
*/

#include "Font.hpp"
#include "Exceptions.hpp"

namespace RType {
    Font::Font(const std::string &path)
    {
        try {
            this->Load(path);
        } catch (raylib::RaylibException &e) {
            throw(EngineException(e.what()));
        }
    }
}; // namespace RType
