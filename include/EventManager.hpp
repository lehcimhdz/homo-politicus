#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include "Country.hpp"

// EventManager handles probabilites and random occurrences.
// It modifies the Country object directly.
class EventManager {
public:
    EventManager();
    void triggerRandomEvent(Country& country);
};

#endif // EVENTMANAGER_HPP
