#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

struct TestCase {
    const char* name;
    void (*fn)(int&);
};

inline std::vector<TestCase>& testRegistry() {
    static std::vector<TestCase> reg;
    return reg;
}

#define TEST_CASE(name)                                                            \
    static void name(int& __failures);                                             \
    static struct name##_registrar {                                               \
        name##_registrar() { testRegistry().push_back({#name, name}); }            \
    } name##_reg_instance;                                                         \
    static void name(int& __failures)

#define CHECK(cond)                                                                \
    do {                                                                           \
        if (!(cond)) {                                                             \
            std::cerr << "  [FAIL] " << #cond << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
            __failures++;                                                          \
        }                                                                          \
    } while (0)

#define CHECK_EQ(a, b)                                                             \
    do {                                                                           \
        auto __va = (a); auto __vb = (b);                                          \
        if (!(__va == __vb)) {                                                     \
            std::cerr << "  [FAIL] " << #a << " == " << #b                         \
                      << " (got " << __va << " vs " << __vb << ") at "             \
                      << __FILE__ << ":" << __LINE__ << "\n";                      \
            __failures++;                                                          \
        }                                                                          \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                      \
    do {                                                                           \
        double __va = (a); double __vb = (b);                                      \
        if (std::fabs(__va - __vb) > (eps)) {                                      \
            std::cerr << "  [FAIL] " << #a << " ≈ " << #b                          \
                      << " (got " << __va << " vs " << __vb << ", eps " << (eps)   \
                      << ") at " << __FILE__ << ":" << __LINE__ << "\n";           \
            __failures++;                                                          \
        }                                                                          \
    } while (0)

inline int runAllTests() {
    int total = (int)testRegistry().size();
    int passed = 0;
    int totalFailures = 0;
    for (const auto& t : testRegistry()) {
        int localFailures = 0;
        std::cout << "[RUN ] " << t.name << "\n";
        t.fn(localFailures);
        if (localFailures == 0) {
            std::cout << "[ OK ] " << t.name << "\n";
            passed++;
        } else {
            std::cout << "[FAIL] " << t.name << " (" << localFailures << " check(s) failed)\n";
            totalFailures += localFailures;
        }
    }
    std::cout << "\n=== " << passed << "/" << total << " test cases passed";
    if (totalFailures > 0) std::cout << " (" << totalFailures << " failures)";
    std::cout << " ===\n";
    return totalFailures == 0 ? 0 : 1;
}

#endif
