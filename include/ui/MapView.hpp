#ifndef MAP_VIEW_HPP
#define MAP_VIEW_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Country.hpp"
#include "ui/CountrySilhouette.hpp"
#include "ui/IsoCamera.hpp"
#include "ui/IsoWorld.hpp"
#include "ui/IsoBuilding.hpp"
#include "ui/IsoNpcs.hpp"
#include "ui/IsoVehicles.hpp"
#include "ui/WeatherSystem.hpp"
#include "ui/ProvinceMap.hpp"

// MapView: dibuja el pais central + 3 vecinos como poligonos con relaciones diplomaticas
// y lineas comerciales animadas. Area: x=218..1028, y=100..680 (830 x 580).
class MapView {
public:
    MapView();
    void update(float dt);                 // dt en segundos
    void setTurn(int t) { turn_ = t; }
    int turn() const { return turn_; }
    void onMouseMove(sf::Vector2f mouse);
    void draw(sf::RenderWindow& win, const sf::Font& font, const Country& c) const;
    // Devuelve indice del vecino bajo el mouse (-1 si ninguno)
    int neighborAt(sf::Vector2f mouse) const;
    // Carga silueta de pais por nombre (argentina, chile, cuba, brasil, usa).
    bool loadSilhouette(const std::string& name);
    bool hasSilhouette() const { return homeSilhouette_.loaded(); }

private:
    float t_ = 0.f; // tiempo acumulado para animaciones
    // posiciones de los nodos en la pantalla
    sf::Vector2f homePos_  = {624.f, 380.f};
    sf::Vector2f neighbor_[3] = {
        {340.f, 220.f}, // Northland (NW)
        {920.f, 220.f}, // Easteria  (NE)
        {624.f, 600.f}, // Southaven (S)
    };
    float homeRadius_ = 80.f;
    float neighRadius_ = 55.f;
    int turn_ = 0;
    CountrySilhouette homeSilhouette_;
    mutable IsoCamera isoCam_;
    mutable IsoWorld isoWorld_;
    mutable IsoBuilding isoBuildings_;
    mutable IsoNpcs isoNpcs_;
    mutable IsoVehicles isoVehicles_;
    mutable WeatherSystem weather_;
    mutable ProvinceMap provinces_;
    mutable bool isoConfigured_ = false;

    // Population dots: posiciones precomputadas dentro de la silueta
    // (raw coords, no de pantalla), con micro-movimiento.
    struct PopDot {
        sf::Vector2f base;
        float phase;
    };
    mutable std::vector<PopDot> popDots_;
    void ensurePopDots(int count) const;
};

#endif
