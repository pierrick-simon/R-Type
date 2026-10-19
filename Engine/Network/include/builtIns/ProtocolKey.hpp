/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#ifndef PROTOCOL_KEY_HPP_
    #define PROTOCOL_KEY_HPP_

#include "GameManager.hpp"
#include "ClientManager.hpp"
#include "Network.hpp"
#include "KeyboardManager.hpp"

namespace RType {
    
    class ProtocolKey {
    public:
        ProtocolKey() {};
        void manageKeyPressed(const Header &header, const std::vector<uint8_t> &payload, Context &ctx);
        void manageKeyRelease(const Header &header, const std::vector<uint8_t> &payload, Context &ctx);
        void systemKey(ClientManager &c, KeyboardManager &k);
    private:
        
    };


} //namespace RType

#endif