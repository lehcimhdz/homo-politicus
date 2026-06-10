#include "TestFramework.hpp"
#include "Country.hpp"
#include <limits>

// Tests adicionales de invariantes: bordes que podrían crashear el motor.

TEST_CASE(country_default_gdp_under_max_int_when_divided) {
    Country c;
    long long m = (long long)(c.economy.gdp / 1e6);
    CHECK(m > 0);
    CHECK(m < std::numeric_limits<long long>::max() / 2);
}

TEST_CASE(country_default_neighbors_three) {
    Country c;
    CHECK_EQ(c.neighbors.size(), (size_t)3);
}

TEST_CASE(country_pressures_sum_under_5) {
    Country c;
    double sum = c.politics.congressional_pressure + c.politics.judicial_pressure +
                 c.politics.military_pressure + c.politics.popular_pressure +
                 c.politics.international_pressure;
    CHECK(sum <= 5.0);
    CHECK(sum >= 0.0);
}

TEST_CASE(country_diplomatic_relations_in_range) {
    Country c;
    for (const auto& n : c.neighbors) {
        CHECK(n.diplomatic_relations >= -1.0);
        CHECK(n.diplomatic_relations <= 1.0);
    }
}

TEST_CASE(country_credit_rating_in_enum_bounds) {
    Country c;
    int r = (int)c.economy.credit_rating;
    CHECK(r >= 0);
    CHECK(r <= 10);
}
