#include "llm/LLMProvider.hpp"
#include <unordered_map>

class MockProvider : public LLMProvider {
public:
    std::string ask(const std::string& systemPrompt, const std::string& userMessage) override {
        // Heuristica trivial: si el system prompt menciona "ministro" devuelve consejo fiscal,
        // si dice "inteligencia" devuelve narrativa intelligence-like, etc.
        if (systemPrompt.find("ministro") != std::string::npos || systemPrompt.find("hacienda") != std::string::npos)
            return "[Mock Hacienda] Considera el balance fiscal antes del proximo turno. (LLM real no disponible)";
        if (systemPrompt.find("inteligencia") != std::string::npos)
            return "[Mock Intel] Sin actividad inusual detectada esta semana. (LLM real no disponible)";
        if (systemPrompt.find("vocero") != std::string::npos)
            return "[Mock Vocero] Conferencia programada: 'crecimiento con responsabilidad'. (LLM real no disponible)";
        if (systemPrompt.find("opositor") != std::string::npos || systemPrompt.find("critico") != std::string::npos)
            return "[Mock Critico] El gobierno fracasa. La oposicion exige respuestas. (LLM real no disponible)";
        return "[Mock] " + userMessage.substr(0, 80) + "... (LLM real no disponible)";
    }
    std::string name() const override { return "mock"; }
    bool isAvailable() const override { return true; }
};

namespace LLMFactory {
std::unique_ptr<LLMProvider> createMock() {
    return std::make_unique<MockProvider>();
}
}
