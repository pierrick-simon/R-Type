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

    void Texture2D::unloadTextures2D(GameManager &gm)
    {
        for (auto [e, _] : gm.view<Texture2D>().getVector())
            gm.destroyEntity(e);
    }
}; // namespace RType
