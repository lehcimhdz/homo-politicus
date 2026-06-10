#ifndef ADVISOR_HPP
#define ADVISOR_HPP

#include <string>
#include <vector>
#include <memory>
#include "Country.hpp"

class Advisor {
public:
    virtual ~Advisor() = default;
    virtual std::string id() const = 0;
    virtual std::string name_es() const = 0;
    // TODO: cuando se conecte LLM real, este método llamará a LLMProvider::ask()
    // pasando system prompt + Country state. Por ahora, lógica heuristica canned.
    virtual std::string respond(const Country& c, const std::string& question = "") const = 0;
};

namespace Advisors {
    std::vector<std::unique_ptr<Advisor>> all();
    Advisor* findById(const std::string& id);
}

#endif
