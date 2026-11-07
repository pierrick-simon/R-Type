/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManager
*/

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "GameManager.hpp"

namespace RType {

    GameManager::GameManager(bool debug) :
        _cm(), _em(), _sm(), _debug(debug)
    {
        if (_debug)
            openLogFile();
        log("debug mode enabled");
    }

    void GameManager::openLogFile()
    {
        std::time_t now = std::time(nullptr);
        std::ostringstream name;
        std::error_code error;

        std::filesystem::create_directories("log", error);
        name << "log/"
             << std::put_time(std::localtime(&now), "%Y-%m-%d_%H-%M-%S")
             << ".log";
        _logFile.open(name.str(), std::ios::app);
    }

    void GameManager::update()
    {
        if (_updating)
            return;
        _updating = true;
        try {
            _sm.process(_cm);
        } catch (...) {
            log("update: a system threw an exception");
            _updating = false;
            try {
                flush();
            } catch (...) {
            };
            throw;
        }
        _updating = false;
        if (_cq.size())
            log("update: applying ", _cq.size(), " queued command(s)");
        flush();
    }

    Entity GameManager::createEntity()
    {
        Entity entity = _em.createEntity();

        log("createEntity -> entity ", entity.index());
        return entity;
    }

    void GameManager::destroyEntity(Entity entity)
    {
        log("destroyEntity ", entity.index());
        _cq.emplace_back([this, entity]() {
            if (!hasEntity(entity))
                return;
            _cm.removeComponents(entity);
            _em.destroyEntity(entity);
        });
        tryFlush();
    }

    bool GameManager::hasEntity(Entity entity) const
    {
        return _em.hasEntity(entity);
    }

    void GameManager::removeComponents(Entity entity)
    {
        log("removeComponents on entity ", entity.index());
        _cq.emplace_back([this, entity]() {
            if (!hasEntity(entity))
                throw NoSuchEntity();
            _cm.removeComponents(entity);
        });
        tryFlush();
    }

    void GameManager::destroySystem(SystemId id)
    {
        log("destroySystem ", id);
        _cq.emplace_back([this, id]() {
            if (!hasSystem(id))
                return;
            _sm.destroySystem(id);
        });
        tryFlush();
    }

    bool GameManager::hasSystem(SystemId id) const
    {
        return _sm.hasSystem(id);
    }

    bool GameManager::isUpdating() const
    {
        return _updating;
    }

    void GameManager::setPaused(bool paused)
    {
        _paused = paused;
    }

    bool GameManager::isPaused() const
    {
        return _paused;
    }

    void GameManager::flush()
    {
        std::exception_ptr error = nullptr;

        for (auto &command : _cq) {
            try {
                command();
            } catch (const std::exception &e) {
                log("a queued command failed: ", e.what());
                if (!error)
                    error = std::current_exception();
            } catch (...) {
                log("a queued command failed");
                if (!error)
                    error = std::current_exception();
            }
        }
        _cq.clear();
        if (error)
            std::rethrow_exception(error);
    }

    void GameManager::tryFlush()
    {
        if (!_updating)
            flush();
    }

    void GameManager::run(
        int fps, const std::function<bool()> &keepRunning = [] { return true; })
    {
        using clock = std::chrono::steady_clock;
        const auto step = std::chrono::nanoseconds(1'000'000'000 / fps);
        auto next = clock::now();
        log("run at ", fps, " fps");
        while (keepRunning()) {
            if (!_paused)
                update();
            next += step;
            if (next < clock::now())
                next = clock::now();
            std::this_thread::sleep_until(next);
        }
        log("run stopped");
    }

} // namespace RType
