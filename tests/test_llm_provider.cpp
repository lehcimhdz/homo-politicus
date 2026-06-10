#include "TestFramework.hpp"
#include "llm/LLMProvider.hpp"
#include "Advisor.hpp"
#include "Country.hpp"

TEST_CASE(llm_mock_returns_nonempty) {
    auto m = LLMFactory::createMock();
    CHECK(m->isAvailable());
    std::string r = m->ask("eres ministro de hacienda", "que hago con la inflacion?");
    CHECK(!r.empty());
}

TEST_CASE(llm_anthropic_without_key_unavailable) {
    auto a = LLMFactory::createAnthropic("");
    CHECK(!a->isAvailable());
    std::string r = a->ask("foo", "bar");
    CHECK(r.empty());
}

TEST_CASE(llm_anthropic_with_key_returns_response) {
    auto a = LLMFactory::createAnthropic("dummy-key-for-test");
    CHECK(a->isAvailable());
    std::string r = a->ask("eres ministro de hacienda", "consejo");
    CHECK(!r.empty());
}

TEST_CASE(llm_factory_create_returns_provider) {
    auto p = LLMFactory::create();
    CHECK(p != nullptr);
    CHECK(p->isAvailable());
}

TEST_CASE(advisor_respondWithLLM_uses_provider) {
    Country c;
    auto llm = LLMFactory::createMock();
    auto a = Advisors::findById("hacienda_minister");
    CHECK(a != nullptr);
    std::string r = a->respondWithLLM(llm.get(), c, "que hago?");
    CHECK(!r.empty());
}

TEST_CASE(advisor_respondWithLLM_fallback_when_no_provider) {
    Country c;
    auto a = Advisors::findById("hacienda_minister");
    std::string r = a->respondWithLLM(nullptr, c, "");
    // Cae al heuristic, debe devolver algo
    CHECK(!r.empty());
}
