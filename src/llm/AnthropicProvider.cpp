#include "llm/LLMProvider.hpp"
#include <cstdlib>
#include <sstream>

// AnthropicProvider real requiere libcurl + nlohmann/json. Para evitar dependencia
// dura en este sprint, se implementa el wrapper que prepara request y devuelve mock
// si HP_LLM_REAL no esta definido. Cuando se compile con curl/json activados,
// reemplazar el cuerpo de ask() por la llamada HTTP real.

class AnthropicProvider : public LLMProvider {
public:
    explicit AnthropicProvider(std::string key) : apiKey_(std::move(key)) {}

    std::string ask(const std::string& systemPrompt, const std::string& userMessage) override {
        if (!isAvailable()) return "";
#ifdef HP_LLM_REAL
        // TODO: integrar curl + nlohmann/json en sesion futura.
        // POST https://api.anthropic.com/v1/messages
        // Headers: x-api-key, anthropic-version: 2023-06-01, content-type
        // Body: { model, max_tokens, system, messages: [{role:"user", content: userMessage}] }
        // Response: { content: [{text}] }
        return "[Anthropic real WIP]";
#else
        // Mientras no este HP_LLM_REAL, devuelve mock identificandose como Anthropic offline.
        std::ostringstream oss;
        oss << "[Anthropic offline - API key detectada pero HP_LLM_REAL no esta compilado] ";
        if (systemPrompt.find("hacienda") != std::string::npos)
            oss << "Recomendacion fiscal generica.";
        else
            oss << "Respuesta a: " << userMessage.substr(0, 60);
        return oss.str();
#endif
    }

    std::string name() const override { return "anthropic-claude"; }
    bool isAvailable() const override { return !apiKey_.empty(); }

private:
    std::string apiKey_;
};

namespace LLMFactory {

std::unique_ptr<LLMProvider> createAnthropic(const std::string& apiKey) {
    return std::make_unique<AnthropicProvider>(apiKey);
}

std::unique_ptr<LLMProvider> create() {
    const char* key = std::getenv("ANTHROPIC_API_KEY");
    if (key && *key) return createAnthropic(key);
    return createMock();
}

}
