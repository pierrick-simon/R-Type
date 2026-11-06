/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Metric
*/

#include "Metric.hpp"
#include "Font.hpp"
#include "Text2D.hpp"

namespace RType::Client {
    Metric::Metric(const std::string &path, bool network) :
        _network(network)
    {
        _path = path;
    }

    void Metric::updateMetric(GameManager &gm)
    {
        if (_on) {
            if (!_entities.empty())
                return;
            createFps(gm);
            if (_network)
                createLagometer(gm);
        } else {
            for (auto entity : _entities)
                gm.destroyEntity(entity);
            _entities.clear();
        }
    }

    void Metric::createFps(GameManager &gm)
    {
        auto e = gm.createEntity();
        gm.addComponent<Text2D>(e, 20, "");
        gm.addComponent<Font>(e, _path);
        gm.addComponent<Position2D>(e, 1920.f - 150.f, 10.f);
        _entities.push_back(e);
    }

    void Metric::createLagometer(GameManager &gm)
    {
        auto e = gm.createEntity();
        gm.addComponent<Text2D>(e, 20, "");
        gm.addComponent<Font>(e, _path);
        gm.addComponent<Position2D>(e, 1920.f - 400.f, 10.f);
        _entities.push_back(e);
    }

    void Metric::systemUpdateMetric(GameManager &gm, const MasterClock &clock)
    {
        for (auto [_, m] : gm.view<Metric>().getVector()) {
            m.updateMetric(gm);
            m._fps = int(MILLI_IN_SEC / clock.getElapsed().count());
            if (!m._on)
                continue;
            if (gm.hasComponent<Text2D>(m._entities[FPS])) {
                auto &text = gm.getComponent<Text2D>(m._entities[FPS]);
                text.str = std::to_string(m._fps) + " FPS";
            }
        }
    }

    void Metric::systemUpdateMetricLagometer(
        GameManager &gm, const ClientManager &client)
    {
        for (auto [_, m] : gm.view<Metric>().getVector()) {
            m._lagInfo = client.getLagometerInfo();
            if (!m._on)
                continue;
            if (m._entities.size() >= LAGOMETER + 1 &&
                gm.hasComponent<Text2D>(m._entities[LAGOMETER])) {
                auto &text = gm.getComponent<Text2D>(m._entities[LAGOMETER]);
                auto lossSent =
                    m._lagInfo.lastBeenRecieved > m._lagInfo.lastSent
                    ? 0
                    : m._lagInfo.lastSent - m._lagInfo.lastBeenRecieved;
                auto lossRecieved =
                    m._lagInfo.lastRecieved > m._lagInfo.lastBeenSent
                    ? 0
                    : m._lagInfo.lastBeenSent - m._lagInfo.lastRecieved;
                text.str = "ping: " + std::to_string(int(m._lagInfo.ping)) +
                    " ms\nloss: " + std::to_string(lossSent) + " " +
                    std::to_string(lossRecieved);
            }
        }
    }
} // namespace RType::Client
