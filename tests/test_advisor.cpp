#include "TestFramework.hpp"
#include "Advisor.hpp"
#include "Country.hpp"

TEST_CASE(advisor_five_personalities_registered) {
    auto list = Advisors::all();
    CHECK_EQ(list.size(), (size_t)5);
}

TEST_CASE(advisor_find_by_id_works) {
    CHECK(Advisors::findById("hacienda_minister") != nullptr);
    CHECK(Advisors::findById("intel_chief") != nullptr);
    CHECK(Advisors::findById("spokesperson") != nullptr);
    CHECK(Advisors::findById("political_advisor") != nullptr);
    CHECK(Advisors::findById("opposition_critic") != nullptr);
    CHECK(Advisors::findById("ghost_advisor") == nullptr);
}

TEST_CASE(advisor_hacienda_responds_to_inflation) {
    Country c;
    c.economy.inflation = 0.6;
    auto a = Advisors::findById("hacienda_minister");
    std::string r = a->respond(c, "");
    CHECK(!r.empty());
    CHECK(r.find("inflacion") != std::string::npos);
}

TEST_CASE(advisor_intel_responds_to_military_pressure) {
    Country c;
    c.politics.military_pressure = 0.8;
    auto a = Advisors::findById("intel_chief");
    std::string r = a->respond(c, "");
    CHECK(!r.empty());
    CHECK(r.find("alto mando") != std::string::npos || r.find("oficiales") != std::string::npos);
}

TEST_CASE(advisor_critic_attacks_authoritarianism) {
    Country c;
    c.politics.authoritarian_actions_count = 4;
    auto a = Advisors::findById("opposition_critic");
    std::string r = a->respond(c, "");
    CHECK(r.find("4") != std::string::npos);
    CHECK(r.find("autoritarias") != std::string::npos);
}

TEST_CASE(advisor_all_return_nonempty_with_default_country) {
    Country c;
    for (auto& a : Advisors::all()) {
        std::string r = a->respond(c, "");
        CHECK(!r.empty());
    }
}
