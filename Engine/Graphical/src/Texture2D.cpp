/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Texture2D
*/

#include "Texture2D.hpp"
#include "Exceptions.hpp"

namespace RType {
    Texture2D::Texture2D(const std::string &path)
    {
        try {
            this->Load(path);
        } catch (raylib::RaylibException &e) {
            throw(EngineException(e.what()));
        }
    }
}; // namespace RType
