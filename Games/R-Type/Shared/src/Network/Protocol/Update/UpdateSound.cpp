/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateSound
*/

#include "UpdateSound.hpp"
#include "SyncField.hpp"

namespace RType {
    void UpdateSound::systemUpdateSound(UpdateSound &update, const SoundInfo &s)
    {
        auto track = [&](auto SoundInfo::*member, Property prop) {
            if (syncField(update.*member, s.*member))
                update._update.push_back(prop);
        };

        track(&SoundInfo::loop, Property::LOOP);
        track(&SoundInfo::state, Property::SOUND_STATE);
    }

    std::optional<SetValue> UpdateSound::getProperty(Property prop) const
    {
        switch (prop) {
            case Property::LOOP:
                return loop;
            case Property::SOUND_STATE:
                return state;
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> UpdateSound::_default = {
        Property::LOOP,
        Property::SOUND_STATE,
    };
}; // namespace RType
