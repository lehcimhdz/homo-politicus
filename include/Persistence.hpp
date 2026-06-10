#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP

#include <string>
#include "Country.hpp"
#include "EndCondition.hpp"

namespace Persistence {
    bool save(const Country& c, int turnCount, double popularitySum,
              EndCondition endCondition, const std::string& path);
    bool load(Country& c, int& turnCount, double& popularitySum,
              EndCondition& endCondition, const std::string& path);
}

#endif
