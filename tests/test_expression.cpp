#include "TestFramework.hpp"
#include "ExpressionEvaluator.hpp"
#include "Country.hpp"

TEST_CASE(expr_simple_gt) {
    Country c;
    c.economy.inflation = 0.6;
    CHECK(ExpressionEvaluator::evaluate("country.economy.inflation > 0.5", c));
    CHECK(!ExpressionEvaluator::evaluate("country.economy.inflation > 1.0", c));
}

TEST_CASE(expr_simple_lt) {
    Country c;
    c.politics.popularity = 0.3;
    CHECK(ExpressionEvaluator::evaluate("country.politics.popularity < 0.5", c));
    CHECK(!ExpressionEvaluator::evaluate("country.politics.popularity < 0.1", c));
}

TEST_CASE(expr_and_two_conditions) {
    Country c;
    c.economy.inflation = 0.6;
    c.economy.monetary_emission = 0.4;
    CHECK(ExpressionEvaluator::evaluate(
        "country.economy.inflation > 0.5 AND country.economy.monetary_emission > 0.3", c));
    c.economy.monetary_emission = 0.1;
    CHECK(!ExpressionEvaluator::evaluate(
        "country.economy.inflation > 0.5 AND country.economy.monetary_emission > 0.3", c));
}

TEST_CASE(expr_or_two_conditions) {
    Country c;
    c.economy.exchange_rate_stability = 0.2;
    c.politics.popularity = 0.8;
    CHECK(ExpressionEvaluator::evaluate(
        "country.economy.exchange_rate_stability < 0.3 OR country.politics.popularity < 0.4", c));
}

TEST_CASE(expr_unknown_field_returns_false) {
    Country c;
    CHECK(!ExpressionEvaluator::evaluate("country.unknown.thing > 1.0", c));
}

TEST_CASE(expr_empty_returns_false) {
    Country c;
    CHECK(!ExpressionEvaluator::evaluate("", c));
}
