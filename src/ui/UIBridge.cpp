#include "ui/UIBridge.hpp"
#include <iostream>
#include <sstream>

// C20.4: en lugar de descartar el cout del motor, lo capturamos en un
// stringstream y exponemos las lineas con [!!!] o [INFO] al UI.
namespace {
struct CoutCapture {
    std::stringstream buf;
    std::streambuf* old;
    CoutCapture() : buf(), old(std::cout.rdbuf(buf.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(old); }
};
}

UIBridge::UIBridge() : game_() {
    game_.setQuietMode(true);
}

void UIBridge::tick() {
    if (isGameOver()) return;
    CoutCapture cap;
    // Game::update() ya incrementa turnCount internamente (linea ~7894).
    game_.update();
    game_.checkGameOver();
    // Parsear lineas criticas.
    std::string line;
    while (std::getline(cap.buf, line)) {
        // Filtrar marcadores: [!!!], [!!], [!], [INFO]
        if (line.find("[!!!]") != std::string::npos ||
            line.find("[!!]")  != std::string::npos ||
            line.find("[INFO]") != std::string::npos) {
            criticalBuf_.push_back(line);
        }
    }
}

std::vector<std::string> UIBridge::drainCriticalMessages() {
    std::vector<std::string> out;
    out.swap(criticalBuf_);
    return out;
}

void UIBridge::resetCountry() {
    game_ = Game();
    game_.setQuietMode(true);
    criticalBuf_.clear();
}
