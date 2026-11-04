/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineComponents
*/

#include "Clock.hpp"
#include "Exceptions.hpp"
#include "Font.hpp"
#include "Friction2D.hpp"
#include "Game.hpp"
#include "GravityField2D.hpp"
#include "Listener.hpp"
#include "Model3D.hpp"
#include "ModelAnimation3D.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Position3D.hpp"
#include "Size2D.hpp"
#include "Sprite2D.hpp"
#include "Sprite3D.hpp"
#include "SpriteSheet2D.hpp"
#include "SpriteSheet3D.hpp"
#include "Text2D.hpp"
#include "Texture2D.hpp"
#include "Velocity2D.hpp"

namespace {

    using Args = sol::optional<sol::table>;

    /**
     * @brief Reads a field of the table given by the script, or its
     * default value if the table or the field is missing.
     */
    template <typename T>
    T field(const Args &args, const char *key, T defaultValue)
    {
        if (!args)
            return defaultValue;
        return args->get_or<T>(key, defaultValue);
    }

    /**
     * @brief Reads a field that the component cannot do without.
     * @throw ScriptError If the field is missing.
     */
    std::string requiredPath(const Args &args, const std::string &component)
    {
        sol::optional<std::string> path;

        if (args)
            path = args->get<sol::optional<std::string>>("path");
        if (!path)
            throw RType::ScriptError(component + " needs a path");
        return *path;
    }

} // namespace

namespace RType {

    const std::unordered_map<std::string, Game::ComponentFactory>
        Game::componentFactories = {
        // engine-system
        {"Position2D", [](GameManager &gm, Entity e, const Args &args) {
            Position2D pos;
            pos.x = field(args, "x", pos.x);
            pos.y = field(args, "y", pos.y);
            gm.addComponent<Position2D>(e, pos);
        }},
        {"Position3D", [](GameManager &gm, Entity e, const Args &args) {
            Position3D pos;
            pos.x = field(args, "x", pos.x);
            pos.y = field(args, "y", pos.y);
            pos.z = field(args, "z", pos.z);
            gm.addComponent<Position3D>(e, pos);
        }},
        {"Origin2D", [](GameManager &gm, Entity e, const Args &args) {
            Origin2D origin;
            origin.x = field(args, "x", origin.x);
            origin.y = field(args, "y", origin.y);
            gm.addComponent<Origin2D>(e, origin);
        }},
        {"Size2D", [](GameManager &gm, Entity e, const Args &args) {
            Size2D size;
            size.height = field(args, "height", size.height);
            size.width = field(args, "width", size.width);
            gm.addComponent<Size2D>(e, size);
        }},
        {"Velocity2D", [](GameManager &gm, Entity e, const Args &args) {
            Velocity2D velocity;
            velocity.x = field(args, "x", velocity.x);
            velocity.y = field(args, "y", velocity.y);
            gm.addComponent<Velocity2D>(e, velocity);
        }},
        {"Friction2D", [](GameManager &gm, Entity e, const Args &args) {
            Friction2D friction {};
            friction.f = field(args, "f", friction.f);
            gm.addComponent<Friction2D>(e, friction);
        }},
        {"GravityField2D", [](GameManager &gm, Entity e, const Args &args) {
            GravityField2D gravity;
            gravity.x = field(args, "x", gravity.x);
            gravity.y = field(args, "y", gravity.y);
            gravity.g = field(args, "g", gravity.g);
            gm.addComponent<GravityField2D>(e, gravity);
        }},
        {"Clock", [](GameManager &gm, Entity e, const Args &args) {
            Clock clock;
            clock.speed = field(args, "speed", clock.speed);
            gm.addComponent<Clock>(e, clock);
        }},
        // engine-graphical2D
        {"Texture2D", [](GameManager &gm, Entity e, const Args &args) {
            gm.addComponent<Texture2D>(e, requiredPath(args, "Texture2D"));
        }},
        {"Font", [](GameManager &gm, Entity e, const Args &args) {
            gm.addComponent<Font>(e, requiredPath(args, "Font"));
        }},
        {"Sprite2D", [](GameManager &gm, Entity e, const Args &args) {
            Sprite2D sprite;
            sprite.scale = field(args, "scale", sprite.scale);
            sprite.rotation = field(args, "rotation", sprite.rotation);
            sprite.flip = field(args, "flip", sprite.flip);
            gm.addComponent<Sprite2D>(e, sprite);
        }},
        {"SpriteSheet2D", [](GameManager &gm, Entity e, const Args &args) {
            SpriteSheet2D sheet;
            sheet.recNb.height =
                field(args, "rows", sheet.recNb.height);
            sheet.recNb.width =
                field(args, "columns", sheet.recNb.width);
            sheet.currentTile =
                field(args, "currentTile", sheet.currentTile);
            sheet.frameDuration = std::chrono::milliseconds(field(args,
                "frameDuration", sheet.frameDuration.count()));
            sheet.animation = field(args, "animation", sheet.animation);
            sheet.scale = field(args, "scale", sheet.scale);
            sheet.rotation = field(args, "rotation", sheet.rotation);
            sheet.flip = field(args, "flip", sheet.flip);
            gm.addComponent<SpriteSheet2D>(e, sheet);
        }},
        {"Text2D", [](GameManager &gm, Entity e, const Args &args) {
            Text2D text;
            text.str = field(args, "str", text.str);
            text.fontSize = field(args, "fontSize", text.fontSize);
            text.rotation = field(args, "rotation", text.rotation);
            text.spacing = field(args, "spacing", text.spacing);
            gm.addComponent<Text2D>(e, text);
        }},
        // engine-graphical3D
        {"Model3D", [](GameManager &gm, Entity e, const Args &args) {
            gm.addComponent<Model3D>(e, requiredPath(args, "Model3D"));
        }},
        {"ModelAnimation3D",
            [](GameManager &gm, Entity e, const Args &args) {
            gm.addComponent<ModelAnimation3D>(
                e, requiredPath(args, "ModelAnimation3D"));
        }},
        {"Sprite3D", [](GameManager &gm, Entity e, const Args &args) {
            Sprite3D sprite;
            sprite.scale = field(args, "scale", sprite.scale);
            gm.addComponent<Sprite3D>(e, sprite);
        }},
        {"SpriteSheet3D", [](GameManager &gm, Entity e, const Args &args) {
            SpriteSheet3D sheet;
            sheet.index = field(args, "index", sheet.index);
            sheet.frameDuration = std::chrono::milliseconds(field(args,
                "frameDuration", sheet.frameDuration.count()));
            sheet.animation = field(args, "animation", sheet.animation);
            sheet.scale = field(args, "scale", sheet.scale);
            gm.addComponent<SpriteSheet3D>(e, sheet);
        }},
        // engine-audio
        {"Listener", [](GameManager &gm, Entity e, const Args &) {
            gm.addComponent<Listener>(e);
        }},
    };

    void Game::registerComponents(sol::state &lua)
    {
        lua.new_usertype<Entity>("Entity", sol::no_constructor,
            "index", &Entity::index);
        lua["GameManager"]["createEntity"] = [](GameManager &gm) {
            return gm.createEntity();
        };
        lua["GameManager"]["addComponent"] = [](GameManager &gm,
                                                 Entity entity,
                                                 const std::string &name,
                                                 const Args &args) {
            auto iter = componentFactories.find(name);
            if (iter == componentFactories.end())
                throw UnknownComponent(name);
            iter->second(gm, entity, args);
        };
    }

}
