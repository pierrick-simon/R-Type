/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentBinding
*/

#ifndef COMPONENTBINDING_HPP_
#define COMPONENTBINDING_HPP_

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <sol/sol.hpp>

#include "Exceptions.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Makes the engine's components usable from Lua by name, and
     * keeps, for each one, everything Lua can do with it without knowing
     * its C++ type.
     */
    class ComponentBinding {
    public:
        ComponentBinding() = delete;

        /**
         * @brief The table given by the script to build a component, if
         * any.
         */
        using Args = sol::optional<sol::table>;

        /**
         * @brief Everything Lua can do with one component, without knowing
         * its C++ type: each function calls the GameManager's templates
         * with the type fixed by bindComponent.
         */
        struct Entry {
            /**
             * @brief Adds the component to an entity, built from the
             * script's table (or its defaults if there is none).
             */
            std::function<void(GameManager &, Entity, const Args &)> add;

            /**
             * @brief Removes the component from an entity.
             */
            std::function<void(GameManager &, Entity)> remove;

            /**
             * @brief Checks whether an entity has the component. Never
             * throws.
             */
            std::function<bool(GameManager &, Entity)> has;

            /**
             * @brief Returns the entity's component as a Lua value that
             * refers to the real component, not to a copy: writing its
             * fields from Lua changes it in C++ too.
             */
            std::function<sol::object(lua_State *, GameManager &, Entity)>
                get;

            /**
             * @brief Counts the entities that have the component. 0 if it
             * has never been added, without throwing.
             */
            std::function<std::size_t(GameManager &)> count;

            /**
             * @brief Calls the function on every entity that has the
             * component. Does nothing if it has never been added.
             * @warning Outside update(), destroying entities during the
             * iteration invalidates it, as with View::each.
             */
            std::function<void(
                GameManager &, const std::function<void(Entity)> &)>
                forEachEntity;
        };

        /**
         * @brief Lets scripts give the engine's components to entities by
         * name, with gm:addComponent(entity, "Position2D", {x = 10}).
         * Fields left out keep the component's default value. Scripts can
         * also call gm:removeComponent(entity, name),
         * gm:hasComponent(entity, name), gm:getComponent(entity, name),
         * whose fields write to the real component, and
         * gm:countComponents(name). Binds every engine component.
         * @throw UnknownComponent If the name is not an engine component.
         */
        static void registerFunctions(sol::state &lua);

        /**
         * @brief Finds the entry of a component by the name scripts use.
         * @throw UnknownComponent If no component has this name.
         */
        static const Entry &getEntry(const std::string &name);

        /**
         * @brief Turns the list of component names given by a script, such
         * as {"Position2D", "Velocity2D"}, into their entries. Called once,
         * when the system or the view is created, so that a wrong name
         * stops the script right away.
         *
         * The entries are kept as pointers: those into entries stay valid
         * even if components are bound later. The components' arrays are
         * not kept, since they may not exist yet: the entries look them up
         * each time they are used.
         * @param names The script's table, read in order from 1 to its
         * length.
         * @return One entry per name, in the same order, which is the order
         * of the components given to the script. Empty if the table is
         * empty.
         * @throw UnknownComponent If a name is not a component.
         * @throw InvalidComponentList If an element is not a string, or if
         * a name appears twice.
         */
        static std::vector<const Entry *> resolve(const sol::table &names);

        /**
         * @brief Calls the function on every entity that has all the given
         * components, iterating over the one that has the fewest entities.
         * Does nothing if one of them has never been added, or if there is
         * no entry at all.
         */
        static void forEachMatch(GameManager &gm,
            const std::vector<const Entry *> &components,
            const std::function<void(Entity)> &func);

    private:
        /**
         * @brief Every component scripts can use, by name, filled by
         * bindComponent.
         */
        static std::unordered_map<std::string, Entry> entries;

        /**
         * @brief Every engine component scripts can use, one bindComponent
         * each. Defined in EngineBindings.cpp, the only file to edit to
         * make a new component usable from Lua.
         */
        static void bindEngineComponents(sol::state &lua);

        /**
         * @brief Makes a component usable from Lua under the given name:
         * scripts can then add it with gm:addComponent and read or write
         * the given fields, which are passed as sol::usertype expects them
         * ("x", &Position2D::x, ...).
         *
         * A component without a default constructor is built from the
         * "path" field of the script's table instead.
         */
        template <typename Component, typename... Fields>
        static void bindComponent(
            sol::state &lua, const std::string &name, Fields &&...fields);

        /**
         * @brief Lua code that copies every field of the script's table
         * onto the component, through its usertype. Written in Lua so that
         * a wrong field stops inside Lua and comes back as a normal error,
         * instead of jumping over C++ code.
         */
        static const std::string_view setFieldsScript;

        /**
         * @brief Builds a component from the script's table: its defaults
         * overwritten by the table's fields. A component that has no
         * default constructor is built by construct from the table's
         * "path", then given the table's other fields.
         * @throw InvalidComponentArgs If a field does not exist or has the
         * wrong type, or if the path is missing.
         */
        template <typename Component>
        static Component makeComponent(
            GameManager &gm, const std::string &name, const Args &args);

        /**
         * @brief Builds a component that has no default constructor, from
         * the path given by the script. Specialized in EngineBindings.cpp
         * for a component whose constructor needs more than the path.
         */
        template <typename Component>
        static Component construct(GameManager &gm, const std::string &path);

        /**
         * @brief Copies the fields of the script's table onto the
         * component, through its usertype, except the one named skip.
         * @throw InvalidComponentArgs If a field does not exist or has the
         * wrong type.
         */
        template <typename Component>
        static void setFields(Component &component, const std::string &name,
            const sol::table &fields,
            std::optional<std::string_view> skip = std::nullopt);

        /**
         * @brief Exposes a std::chrono::milliseconds member to Lua as a
         * plain number of milliseconds, for bindComponent.
         */
        template <typename Component>
        static auto milliseconds(std::chrono::milliseconds Component::*member);

        /**
         * @brief Exposes a member of a member to Lua as if it was a direct
         * field, such as SpriteSheet2D::recNb.height as "rows", for
         * bindComponent.
         */
        template <typename Component, typename Inner, typename T>
        static auto nested(Inner Component::*outer, T Inner::*inner);
    };

    template <typename Component>
    auto ComponentBinding::milliseconds(
        std::chrono::milliseconds Component::*member)
    {
        return sol::property(
            [member](Component &c) {
                return (c.*member).count();
            },
            [member](Component &c, std::int64_t ms) {
                c.*member = std::chrono::milliseconds(ms);
            });
    }

    template <typename Component, typename Inner, typename T>
    auto ComponentBinding::nested(Inner Component::*outer, T Inner::*inner)
    {
        return sol::property(
            [outer, inner](Component &c) {
                return c.*outer.*inner;
            },
            [outer, inner](Component &c, T value) {
                c.*outer.*inner = value;
            });
    }

    template <typename Component>
    void ComponentBinding::setFields(Component &component,
        const std::string &name, const sol::table &fields,
        std::optional<std::string_view> skip)
    {
        sol::state_view lua(fields.lua_state());
        sol::protected_function script = lua.registry()["setFields"];
        sol::protected_function_result result = skip
            ? script(&component, fields, *skip)
            : script(&component, fields);

        if (!result.valid()) {
            sol::error error = result;
            throw InvalidComponentArgs(name, error.what());
        }
        sol::optional<std::string> reason = result;
        if (reason)
            throw InvalidComponentArgs(name, *reason);
    }

    template <typename Component>
    Component ComponentBinding::construct(
        GameManager &, const std::string &path)
    {
        return Component(path);
    }

    template <typename Component>
    Component ComponentBinding::makeComponent(
        GameManager &gm, const std::string &name, const Args &args)
    {
        if constexpr (std::is_default_constructible_v<Component>) {
            Component component {};

            if (args)
                setFields(component, name, *args);
            return component;
        } else {
            sol::optional<std::string> path;

            if (args)
                path = args->get<sol::optional<std::string>>("path");
            if (!path)
                throw InvalidComponentArgs(name, "missing 'path'");
            Component component = construct<Component>(gm, *path);

            setFields(component, name, *args, "path");
            return component;
        }
    }

    template <typename Component, typename... Fields>
    void ComponentBinding::bindComponent(
        sol::state &lua, const std::string &name, Fields &&...fields)
    {
        lua.new_usertype<Component>(name, std::forward<Fields>(fields)...);
        entries[name] = Entry {
            .add =
                [name](GameManager &gm, Entity entity, const Args &args) {
                    gm.addComponent<Component>(
                        entity, makeComponent<Component>(gm, name, args));
                },
            .remove =
                [](GameManager &gm, Entity entity) {
                    gm.removeComponent<Component>(entity);
                },
            .has =
                [](GameManager &gm, Entity entity) {
                    return gm.hasComponent<Component>(entity);
                },
            .get =
                [](lua_State *L, GameManager &gm, Entity entity) {
                    return sol::make_object(
                        L, &gm.getComponent<Component>(entity));
                },
            .count =
                [](GameManager &gm) {
                    return gm.countComponents<Component>();
                },
            .forEachEntity =
                [](GameManager &gm,
                    const std::function<void(Entity)> &func) {
                    gm.view<Component>().each(
                        [&func](Entity entity, Component &) {
                            func(entity);
                        });
                },
        };
    }

} // namespace RType::Game

#endif /* !COMPONENTBINDING_HPP_ */
