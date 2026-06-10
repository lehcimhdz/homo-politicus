#ifndef ISO_CAMERA_HPP
#define ISO_CAMERA_HPP

#include <SFML/Graphics.hpp>

// IsoCamera: proyeccion isometrica dimetric 2:1 estilo CK3.
// World coords: x=east, y=north (en grid), z=altura.
// Screen coords: sx = (x - y) * tileW/2 * zoom + viewportX
//                sy = (x + y) * tileH/2 * zoom - z * tileH * zoom + viewportY
class IsoCamera {
public:
    IsoCamera() = default;

    void setViewport(float cx, float cy) { viewport_ = {cx, cy}; }
    sf::Vector2f viewport() const { return viewport_; }

    void setCenter(float wx, float wy) { center_ = {wx, wy}; }
    sf::Vector2f center() const { return center_; }

    void setZoom(float z);
    float zoom() const { return zoom_; }

    void pan(float dx, float dy) { center_.x += dx; center_.y += dy; }

    // 3D (x, y, z) -> screen 2D
    sf::Vector2f worldToScreen(float wx, float wy, float wz = 0.f) const;
    // 2D screen -> world (asume z=0)
    sf::Vector2f screenToWorld(float sx, float sy) const;

    // Tile size base (sin zoom)
    static constexpr float kTileW = 32.f;
    static constexpr float kTileH = 16.f;

    float tileW() const { return kTileW * zoom_; }
    float tileH() const { return kTileH * zoom_; }

private:
    sf::Vector2f viewport_ {624.f, 380.f};
    sf::Vector2f center_   {0.f, 0.f};
    float zoom_ = 1.f;
};

#endif
