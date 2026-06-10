#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>

// Sprint 9: Hello World con SFML 3. Sin engine wire todavia.
// Sprint 10 conectara via UIBridge.

int main(int argc, char** argv) {
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--headless") headless = true;
    }

    sf::RenderWindow window(sf::VideoMode({1280, 800}), "Homo Politicus");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontOk = false;
    const char* candidates[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
    };
    for (const char* p : candidates) {
        if (font.openFromFile(p)) { fontOk = true; break; }
    }

    sf::Text title(font, "HOMO POLITICUS", 72);
    if (fontOk) {
        auto bounds = title.getLocalBounds();
        title.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        title.setPosition({640.f, 350.f});
        title.setFillColor(sf::Color(230, 230, 240));
    }

    sf::Text subtitle(font, "v0.7-beta UI scaffold (Sprint 9)", 24);
    if (fontOk) {
        auto bounds = subtitle.getLocalBounds();
        subtitle.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        subtitle.setPosition({640.f, 440.f});
        subtitle.setFillColor(sf::Color(150, 150, 170));
    }

    sf::Text hint(font, "ESC para salir", 18);
    if (fontOk) {
        auto bounds = hint.getLocalBounds();
        hint.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        hint.setPosition({640.f, 760.f});
        hint.setFillColor(sf::Color(100, 100, 120));
    }

    if (headless) {
        window.close();
        std::cout << "[headless smoke] window created, font_ok=" << fontOk << std::endl;
        return 0;
    }

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPress->code == sf::Keyboard::Key::Escape) window.close();
            }
        }
        window.clear(sf::Color(20, 22, 30));
        if (fontOk) {
            window.draw(title);
            window.draw(subtitle);
            window.draw(hint);
        }
        window.display();
    }
    return 0;
}
