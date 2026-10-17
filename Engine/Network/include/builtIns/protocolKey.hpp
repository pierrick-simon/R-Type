/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#ifndef MANAGE_KEY_PRESSED_HPP_
    #define MANAGE_KEY_PRESSED_HPP_

#include "GameManager.hpp"
#include "ClientManager.hpp"
#include "KeyboardManager.hpp"
#include "Network.hpp"

namespace RType {
    
    class ProtocolKey {
    public:
        ProtocolKey() {};
        void manageKeyPressed(
            const Header &header, const std::vector<uint8_t> &payload, Context &ctx);
        void manageKeyRelease(
            const Header &header, const std::vector<uint8_t> &payload, Context &ctx);
        void systemKey(ClientManager &c, KeyboardManager &km);
    private:
        
    };


} //namespace RType

#endif