#pragma once

#include <SFML/System.hpp>

class DirectionResolver
{
    public:
        void upActive(bool b);
        void downActive(bool b);
        void leftActive(bool b);
        void rightActive(bool b);

        void upMaybeActive(bool b);
        void downMaybeActive(bool b);
        void leftMaybeActive(bool b);
        void rightMaybeActive(bool b);

        void upMaybeInactive(bool b);
        void downMaybeInactive(bool b);
        void leftMaybeInactive(bool b);
        void rightMaybeInactive(bool b);

        sf::Vector2f getDirection();

    private:
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
};
