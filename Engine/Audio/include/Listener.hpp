#ifndef LISTENER_HPP_
#define LISTENER_HPP_

    #include <memory>
    #include <miniaudio.h>
    #include "Position2D.hpp"

namespace RType {
    /**
     * @class Listener
     * @brief Audio listener component, owns the miniaudio engine.
     *
     * The Listener wraps a heap-allocated @c ma_engine and represents the
     * "ears" of the scene: spatialized Sound components are heard relative
     * to its position (see systemListener2D()).
     *
     * @note Only one Listener can exist during the lifetime of the program.
     * @warning Every Sound created from a Listener must be destroyed before
     *          that Listener.
     *
     * @see Sound
     */
    class Listener {
    public:
        /**
         * @brief Create the listener and initialise the audio engine.
         *
         * @throw EngineException If a Listener has already been created, or
         *        if the audio engine cannot be initialised.
         */
        Listener();

        /**
         * @brief Get the underlying miniaudio engine.
         *
         * @return A reference to the @c ma_engine owned by this listener.
         *
         * @warning The reference is only valid as long as the Listener is
         *          alive.
         */
        [[nodiscard]] ma_engine &getEngine()
        {
            return *_engine;
        }

        /**
         * @brief System updating the position of the listener in 2D space.
         *
         * Sets the position of the engine's listener (index 0) to
         * (pos.x, pos.y, 0).
         *
         * @param l   The listener to move.
         * @param pos The new position of the listener.
         */
        static void systemListener2D(Listener &l, const Position2D &pos);

    private:
        /**
         * @brief Custom deleter uninitialising the audio engine.
         */
        struct EngineDeleter {
            /**
             * @brief Call @c ma_engine_uninit on the engine.
             *
             * @param e The engine to uninitialise.
             */
            void operator()(ma_engine *e) const;
        };

        std::unique_ptr<ma_engine, EngineDeleter> _engine;
    };
} // namespace RType

#endif /* !LISTENER_HPP_ */
