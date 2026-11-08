/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Music
*/

#include "Common/Music.hpp"

#include <vector>

#include "Common/Background.hpp"
#include "Player/Destroy.hpp"
#include "SoundInfo.hpp"
#include "UpdateClientEntity.hpp"

namespace RType::Server {

    std::optional<AssetId> levelMusic(GameManager &gm)
    {
        if (gm.view<Background>().getVector().empty())
            return std::nullopt;
        return levelEnded(gm) ? Asset::BOSS_MUSIC : Asset::FIRST_LEVEL_MUSIC;
    }

    void playMusic(GameManager &gm, std::optional<AssetId> track)
    {
        std::vector<Entity> old;
        bool playing = false;

        for (auto [entity, music] : gm.view<Music>().getVector()) {
            if (track && music.track == *track)
                playing = true;
            else
                old.push_back(entity);
        }
        for (const Entity entity : old)
            destroyNetworked(gm, entity);
        if (playing || !track)
            return;
        const Entity entity = gm.createEntity();
        SoundInfo sound;

        sound.state = SoundInfo::PLAY;
        sound.loop = true;
        gm.addComponent<Music>(entity, Music {*track});
        gm.addComponent<SoundInfo>(entity, sound);
        gm.addComponent<UpdateClientEntity>(
            entity, EntityType::SOUND, *track, gm, entity);
    }

    void musicSystem(GameManager &gm)
    {
        playMusic(gm, levelMusic(gm));
    }

} // namespace RType::Server
