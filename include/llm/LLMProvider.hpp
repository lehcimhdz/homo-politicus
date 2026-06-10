#ifndef LLM_PROVIDER_HPP
#define LLM_PROVIDER_HPP

#include <string>
#include <memory>

// LLMProvider: abstraccion para clientes LLM (Anthropic, OpenAI, local).
// Implementacion default es MockProvider (responses canned).
// Real provider requiere libcurl + API key via ANTHROPIC_API_KEY env var.
class LLMProvider {
public:
    virtual ~LLMProvider() = default;
    // Sincronico, retorna texto completo. Si falla, retorna "".
    virtual std::string ask(const std::string& systemPrompt,
                            const std::string& userMessage) = 0;
    virtual std::string name() const = 0;
    virtual bool isAvailable() const = 0;
};

namespace LLMFactory {
    // Default: AnthropicProvider si ANTHROPIC_API_KEY esta seteada, sino MockProvider.
    std::unique_ptr<LLMProvider> create();
    std::unique_ptr<LLMProvider> createMock();
    std::unique_ptr<LLMProvider> createAnthropic(const std::string& apiKey);
}

#endif
