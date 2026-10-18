/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#ifndef PROTOCOL_CREATE_HPP_
    #define PROTOCOL_CREATE_HPP_

#include "GameManager.hpp"
#include "ClientManager.hpp"
#include "Network.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Sound.hpp"
#include "Text2D.hpp"

namespace RType {
    
    struct ProtocolEntity {
        static constexpr std::size_t SIZE = 4 + 1 + 2 + 4 + 4;
        uint32_t id;
        uint8_t type;
        uint16_t assetId;
        uint32_t posX;
        uint32_t posY;
    };

    enum EntityType : uint8_t {
        SPRITE = 0,
        SPRITESHEET = 1,
        SOUND = 2,
        TEXT = 3
    };


    class ProtocolCreate {
    public:
        ProtocolCreate() {};
        void manageCreate(
            const Header &header, const std::vector<uint8_t> &payload, Context &ctx);

        void manageCreateSprite2D(ProtocolEntity &e, Context &ctx);
        void manageCreateSpriteSheet(ProtocolEntity &e, Context &ctx);
        void manageCreateSound(ProtocolEntity &e, Context &ctx);
        void manageCreateText2D(ProtocolEntity &e, Context &ctx);


    private:
        
    };


} //namespace RType

#endif