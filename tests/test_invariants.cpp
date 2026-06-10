#include "TestFramework.hpp"
#include "Country.hpp"

TEST_CASE(default_country_has_positive_gdp) {
    Country c;
    CHECK(c.economy.gdp > 0.0);
}

TEST_CASE(default_population_positive) {
    Country c;
    CHECK(c.welfare.population > 0);
}

TEST_CASE(default_popularity_in_unit_range) {
    Country c;
    CHECK(c.politics.popularity >= 0.0);
    CHECK(c.politics.popularity <= 1.0);
}

TEST_CASE(default_pressures_in_unit_range) {
    Country c;
    CHECK(c.politics.congressional_pressure >= 0.0 && c.politics.congressional_pressure <= 1.0);
    CHECK(c.politics.judicial_pressure >= 0.0 && c.politics.judicial_pressure <= 1.0);
    CHECK(c.politics.military_pressure >= 0.0 && c.politics.military_pressure <= 1.0);
    CHECK(c.politics.popular_pressure >= 0.0 && c.politics.popular_pressure <= 1.0);
    CHECK(c.politics.international_pressure >= 0.0 && c.politics.international_pressure <= 1.0);
}

TEST_CASE(default_credit_rating_in_enum_range) {
    Country c;
    CHECK((int)c.economy.credit_rating >= (int)CreditRating::AAA);
    CHECK((int)c.economy.credit_rating <= (int)CreditRating::D);
}

TEST_CASE(default_ideology_in_unit_range) {
    Country c;
    CHECK(c.politics.economic_ideology >= 0.0 && c.politics.economic_ideology <= 1.0);
    CHECK(c.politics.social_ideology >= 0.0 && c.politics.social_ideology <= 1.0);
    CHECK(c.politics.auth_dem_axis >= 0.0 && c.politics.auth_dem_axis <= 1.0);
}

TEST_CASE(country_starts_with_three_neighbors_after_init) {
    Country c;
    // Constructor inicializa 3 vecinos
    CHECK_EQ(c.neighbors.size(), (size_t)3);
}
