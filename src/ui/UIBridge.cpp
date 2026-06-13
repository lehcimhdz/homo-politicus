#include "ui/UIBridge.hpp"
#include <iostream>

// RAII helper para silenciar std::cout durante un scope (Game::update
// imprime miles de mensajes que en CLI son utiles pero en la UI son ruido).
namespace {
struct CoutSuppressor {
    std::streambuf* old;
    CoutSuppressor() : old(std::cout.rdbuf(nullptr)) {}
    ~CoutSuppressor() { std::cout.rdbuf(old); }
};
}

UIBridge::UIBridge() : game_() {
    game_.setQuietMode(true);
}

void UIBridge::tick() {
    if (isGameOver()) return;
    {
        CoutSuppressor suppress;
        // Game::update() ya incrementa turnCount internamente (linea ~7894).
        game_.update();
        game_.checkGameOver();
    }
}

void UIBridge::resetCountry() {
    game_ = Game();
    game_.setQuietMode(true);
}
