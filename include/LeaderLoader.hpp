#ifndef LEADER_LOADER_HPP
#define LEADER_LOADER_HPP

#include <string>
#include "Country.hpp"

namespace LeaderLoader {
    struct Metadata {
        std::string id;
        std::string name_es;
        std::string country_origin;
    };
    bool load(const std::string& path, Country& c, Metadata& meta);
}

#endif
