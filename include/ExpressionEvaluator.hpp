#ifndef EXPRESSION_EVALUATOR_HPP
#define EXPRESSION_EVALUATOR_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include "Country.hpp"

namespace ExpressionEvaluator {
    // Soporta: paths country.<system>.<field>, comparadores >, <, >=, <=, ==, !=
    //          conectores AND, OR (sin parentesis)
    // No soporta: NOT, parentesis, helpers tipo any_neighbor.X
    bool evaluate(const std::string& expression, const Country& c);

    // Tabla de campos accesibles via path
    const std::unordered_map<std::string, std::function<double(const Country&)>>& fieldTable();
}

#endif
