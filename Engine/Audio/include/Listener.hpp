#ifndef LISTENER_HPP_
#define LISTENER_HPP_

    #include <memory>
    #include <miniaudio.h>
    #include "Position2D.hpp"

namespace RType {
    class Listener {
    public:
        Listener();

        [[nodiscard]] ma_engine &getEngine()
        {
            return *_engine;
        }

        static void systemListener2D(Listener &l, const Position2D &pos);

    private:
        std::unique_ptr<ma_engine> _engine;
    };
} // namespace RType

#endif /* !LISTENER_HPP_ */
