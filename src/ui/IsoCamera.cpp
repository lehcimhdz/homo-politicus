#include "ui/IsoCamera.hpp"
#include <algorithm>

void IsoCamera::setZoom(float z) {
    zoom_ = std::clamp(z, 0.5f, 2.f);
}

sf::Vector2f IsoCamera::worldToScreen(float wx, float wy, float wz) const {
    float dx = wx - center_.x;
    float dy = wy - center_.y;
    float sx = (dx - dy) * (kTileW * 0.5f) * zoom_;
    float sy = (dx + dy) * (kTileH * 0.5f) * zoom_ - wz * kTileH * zoom_;
    return { sx + viewport_.x, sy + viewport_.y };
}

sf::Vector2f IsoCamera::screenToWorld(float sx, float sy) const {
    float rx = (sx - viewport_.x) / zoom_;
    float ry = (sy - viewport_.y) / zoom_;
    // Resolver el sistema:
    //   rx = (dx - dy) * kTileW/2
    //   ry = (dx + dy) * kTileH/2
    float a = rx / (kTileW * 0.5f);
    float b = ry / (kTileH * 0.5f);
    float dx = (a + b) * 0.5f;
    float dy = (b - a) * 0.5f;
    return { dx + center_.x, dy + center_.y };
}
