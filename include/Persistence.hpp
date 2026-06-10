#ifndef PERSISTENCE_HPP
#define PERSISTENCE_HPP

#include <string>
#include "Country.hpp"
#include "EndCondition.hpp"

namespace Persistence {
    bool save(const Country& c, int turnCount, double popularitySum,
              EndCondition endCondition, const std::string& path,
              const std::string& achievements_line = "");
    bool load(Country& c, int& turnCount, double& popularitySum,
              EndCondition& endCondition, const std::string& path,
              std::string* achievements_line_out = nullptr);
}

#endif
