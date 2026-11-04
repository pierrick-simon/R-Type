/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineSystems
*/

#include "Camera3D.hpp"
#include "Draw3D.hpp"
#include "Exceptions.hpp"
#include "Friction2D.hpp"
#include "Game.hpp"
#include "GravityField2D.hpp"
#include "Keybind.hpp"
#include "Listener.hpp"
#include "Sound.hpp"
#include "Sprite2D.hpp"
#include "Sprite3D.hpp"
#include "SpriteSheet2D.hpp"
#include "SpriteSheet3D.hpp"
#include "Text2D.hpp"
#include "Velocity2D.hpp"

namespace RType {

    template <typename... Components>
    Game::SystemFactory Game::makeSystem(void (*system)(Components...))
    {
        return [system](GameManager &gm) {
            return gm.createSystem<std::remove_cvref_t<Components>...>(system);
        };
    }

    template <typename... Components>
    Game::SystemFactory Game::makeSystem(
        void (*system)(GameManager &, Components...))
    {
        return [system](GameManager &gm) {
            return gm.createSystem<std::remove_cvref_t<Components>...>(system);
        };
    }

    void Game::registerSystems(sol::state &lua)
    {
        lua["GameManager"]["addSystem"] =
            [](GameManager &gm, const std::string &name) {
                auto iter = systemFactories.find(name);
                if (iter == systemFactories.end())
                    throw UnknownSystem(name);
                return iter->second(gm);
            };
    }

    const std::unordered_map<std::string, Game::SystemFactory>
        Game::systemFactories = {
        // engine-system
        {"Clock", makeSystem(Clock::systemClock)},
        {"MasterClock", makeSystem(MasterClock::systemMasterClock)},
        {"Velocity2D", makeSystem(Velocity2D::systemVelocity2D)},
        {"MasterVelocity2D", makeSystem(Velocity2D::systemMasterVelocity2D)},
        {"Friction2D", makeSystem(Friction2D::systemFriction2D)},
        {"MasterFriction2D", makeSystem(Friction2D::systemMasterFriction2D)},
        {"GravityField2D", makeSystem(GravityField2D::systemGravityField2D)},
        {"MasterGravityField2D", makeSystem(GravityField2D::systemMasterGravityField2D)},
        // engine-graphical2D
        {"ClearWindow", makeSystem(Window::systemClearWindow)},
        {"KeyboardManager", makeSystem(KeyboardManager::systemKeyboardManager)},
        {"KeyboardManagerClear", makeSystem(KeyboardManager::systemKeyboardManagerClear)},
        {"MouseManager", makeSystem(MouseManager::systemMouseManager)},
        {"UpdateKeybind", makeSystem(Keybind::systemUpdateKeybind)},
        {"DrawText2D", makeSystem(Text2D::systemDrawText2D)},
        {"DrawSprite2D", makeSystem(Sprite2D::systemDrawSprite2D)},
        {"DrawSpriteSheet2D", makeSystem(SpriteSheet2D::systemDrawSpriteSheet2D)},
        {"AnimateSpriteSheet2D", makeSystem(SpriteSheet2D::systemAnimateSpriteSheet2D)},
        {"MasterAnimateSpriteSheet2D", makeSystem(SpriteSheet2D::systemMasterAnimateSpriteSheet2D)},
        // engine-graphical3D
        {"Draw3D", makeSystem(Draw3D::systemDraw3D)},
        {"DrawSprite3D", makeSystem(Sprite3D::systemDrawSprite3D)},
        {"UpdateCamera", makeSystem(Camera3D::systemUpdateCamera)},
        {"AnimateSpriteSheet3D", makeSystem(SpriteSheet3D::systemAnimateSpriteSheet3D)},
        {"MasterAnimateSpriteSheet3D", makeSystem(SpriteSheet3D::systemMasterAnimateSpriteSheet3D)},
        {"DrawSpriteSheet3D", makeSystem(SpriteSheet3D::systemDrawSpriteSheet3D)},
        // engine-audio
        {"Sound", makeSystem(Sound::systemSound)},
        {"Sound2D", makeSystem(Sound::systemSound2D)},
        {"Listener2D", makeSystem(Listener::systemListener2D)},
    };

}
