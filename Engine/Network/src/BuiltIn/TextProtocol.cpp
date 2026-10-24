/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TextProtocol
*/

#include "TextProtocol.hpp"
#include "HelperProtocol.hpp"
#include "Text2D.hpp"

namespace RType {

    void TextProtocol::manageText(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        auto e = parseText(payload);
        if (!e)
            return;
        auto entity = findNetEntity(ctx, e->id);
        if (!entity)
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<Text2D>(*entity))
            return;
        gm.getComponent<Text2D>(*entity).str = std::move(e->text);
    }

    void TextProtocol::sendText(const std::vector<uint32_t> &ids,
        ServerManager &s, uint32_t id, const std::string &text)
    {
        s.sendToClients(ids, Opcode::TEXT, serializeText({id, text}));
    }

    std::vector<uint8_t> TextProtocol::serializeText(const TextPayload &e)
    {
        const auto len = static_cast<uint16_t>(
            std::min<std::size_t>(e.text.size(), UINT16_MAX));
        std::vector<uint8_t> out(TextPayload::MIN_SIZE + len);

        put32(&out[0], e.id);
        put16(&out[4], len);
        std::copy_n(e.text.begin(), len, out.begin() + TextPayload::MIN_SIZE);
        return out;
    }

    std::optional<TextPayload> TextProtocol::parseText(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() < TextPayload::MIN_SIZE)
            return std::nullopt;
        const std::size_t len = get16(payload.data() + 4);
        if (payload.size() != TextPayload::MIN_SIZE + len)
            return std::nullopt;
        TextPayload e;
        e.id = get32(payload.data());
        e.text.assign(payload.begin() + TextPayload::MIN_SIZE, payload.end());
        return e;
    }

} // namespace RType
