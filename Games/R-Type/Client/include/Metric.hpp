/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Metric
*/

#ifndef METRIC_HPP_
#define METRIC_HPP_

#include "ClientManager.hpp"
#include "Clock.hpp"

namespace RType::Client {
    class Metric {
    public:
        enum MetricInfo {
            FPS,
            LAGOMETER,
        };

        Metric(const std::string &path, bool network = false);

        void setOn(bool on)
        {
            _on = on;
        }

        void activate()
        {
            _on = !_on;
        }

        static void systemUpdateMetric(
            GameManager &gm, const MasterClock &clock);
        static void systemUpdateMetricLagometer(
            GameManager &gm, const ClientManager &client);

    private:
        void updateMetric(GameManager &gm);
        void createFps(GameManager &gm);
        void createLagometer(GameManager &gm);
        bool _on = false;
        std::string _path;
        Lagometer::Info _lagInfo;
        int _fps = 0.f;
        bool _network;
        std::vector<Entity> _entities;
    };
} // namespace RType::Client

#endif /* !METRIC_HPP_ */
