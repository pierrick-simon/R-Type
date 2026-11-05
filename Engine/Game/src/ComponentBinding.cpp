/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentBinding
*/

#include <algorithm>

#include "ComponentBinding.hpp"
#include "Exceptions.hpp"

namespace RType::Game {

    std::unordered_map<std::string, ComponentBinding::Entry>
        ComponentBinding::entries;

    const char *const ComponentBinding::setFieldsScript = R"(
        local component, fields, skip = ...
        for key, value in pairs(fields) do
            if key == skip then goto continue end
            local found, current = pcall(function() return component[key] end)
            if not found or current == nil then
                return "no field '" .. tostring(key) .. "'"
            end
            if type(value) ~= type(current) then
                return "field '" .. tostring(key) .. "' expects a "
                    .. type(current) .. ", got a " .. type(value)
            end
            local ok, err = pcall(function() component[key] = value end)
            if not ok then
                return "field '" .. tostring(key) .. "': " .. tostring(err)
            end
            ::continue::
        end
        return nil
    )";

    void ComponentBinding::registerFunctions(sol::state &lua)
    {
        lua.registry()["setFields"] =
            lua.load(setFieldsScript, "=setFields")
                .get<sol::protected_function>();
        lua["GameManager"]["addComponent"] = [](GameManager &gm,
                                                Entity entity,
                                                const std::string &name,
                                                const Args &args) {
            getEntry(name).add(gm, entity, args);
        };
        lua["GameManager"]["removeComponent"] =
            [](GameManager &gm, Entity entity, const std::string &name) {
                getEntry(name).remove(gm, entity);
            };
        lua["GameManager"]["hasComponent"] =
            [](GameManager &gm, Entity entity, const std::string &name) {
                return getEntry(name).has(gm, entity);
            };
        lua["GameManager"]["getComponent"] = [](GameManager &gm,
                                                Entity entity,
                                                const std::string &name,
                                                sol::this_state L) {
            return getEntry(name).get(L, gm, entity);
        };
        lua["GameManager"]["countComponents"] =
            [](GameManager &gm, const std::string &name) {
                return getEntry(name).count(gm);
            };

        bindEngineComponents(lua);
    }

    const ComponentBinding::Entry &ComponentBinding::getEntry(
        const std::string &name)
    {
        auto iter = entries.find(name);

        if (iter == entries.end())
            throw UnknownComponent(name);
        return iter->second;
    }

    std::vector<const ComponentBinding::Entry *> ComponentBinding::resolve(
        const sol::table &names)
    {
        std::vector<const Entry *> resolved;

        for (std::size_t i = 1; i <= names.size(); i++) {
            sol::object element = names[i];

            if (element.get_type() != sol::type::string)
                throw InvalidComponentList(
                    "element " + std::to_string(i) + " is not a name");
            std::string name = element.as<std::string>();
            const Entry *entry = &getEntry(name);
            if (std::ranges::find(resolved, entry) != resolved.end())
                throw InvalidComponentList(name + " is in it twice");
            resolved.push_back(entry);
        }
        return resolved;
    }

    void ComponentBinding::forEachMatch(GameManager &gm,
        const std::vector<const Entry *> &components,
        const std::function<void(Entity)> &func)
    {
        const Entry *smallest = nullptr;
        std::size_t smallestCount = 0;

        for (const Entry *entry : components) {
            std::size_t count = entry->count(gm);

            if (count == 0)
                return;
            if (!smallest || count < smallestCount) {
                smallest = entry;
                smallestCount = count;
            }
        }
        if (!smallest)
            return;
        smallest->forEachEntity(gm, [&](Entity entity) {
            for (const Entry *entry : components)
                if (!entry->has(gm, entity))
                    return;
            func(entity);
        });
    }

} // namespace RType::Game
