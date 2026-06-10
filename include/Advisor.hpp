#ifndef ADVISOR_HPP
#define ADVISOR_HPP

#include <string>
#include <vector>
#include <memory>
#include "Country.hpp"

class LLMProvider;

class Advisor {
public:
    virtual ~Advisor() = default;
    virtual std::string id() const = 0;
    virtual std::string name_es() const = 0;
    virtual std::string respond(const Country& c, const std::string& question = "") const = 0;
    // Si hay un LLMProvider, envia system prompt + estado del Country. Cae al heuristic.
    std::string respondWithLLM(LLMProvider* llm, const Country& c, const std::string& question = "") const;
};

namespace Advisors {
    std::vector<std::unique_ptr<Advisor>> all();
    Advisor* findById(const std::string& id);
}

#endif
