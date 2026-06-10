#include "ui/CountrySilhouette.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cmath>

static std::string slurp(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {};
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

static std::string extractAttr(const std::string& xml, const std::string& attr) {
    // Busca attr=\"...\" o attr='...'
    std::string key = attr + "=";
    auto pos = xml.find(key);
    if (pos == std::string::npos) return {};
    pos += key.size();
    if (pos >= xml.size()) return {};
    char q = xml[pos];
    if (q != '"' && q != '\'') return {};
    auto end = xml.find(q, pos + 1);
    if (end == std::string::npos) return {};
    return xml.substr(pos + 1, end - pos - 1);
}

bool CountrySilhouette::loadFromFile(const std::string& path) {
    std::string xml = slurp(path);
    if (xml.empty()) return false;
    std::string d = extractAttr(xml, "d");
    if (d.empty()) return false;

    points_.clear();
    // Tokens: comandos (letras) y numeros. Solo soportamos M, L, Z (case-insensitive).
    // Numeros separados por whitespace, comas o "-".
    size_t i = 0;
    char cmd = 'M';
    sf::Vector2f cur{0.f, 0.f};
    auto skipSep = [&]() {
        while (i < d.size() && (std::isspace((unsigned char)d[i]) || d[i] == ',')) ++i;
    };
    auto readNum = [&](float& out) -> bool {
        skipSep();
        if (i >= d.size()) return false;
        size_t start = i;
        if (d[i] == '+' || d[i] == '-') ++i;
        while (i < d.size() && (std::isdigit((unsigned char)d[i]) || d[i] == '.')) ++i;
        if (start == i) return false;
        try { out = std::stof(d.substr(start, i - start)); }
        catch (...) { return false; }
        return true;
    };

    while (i < d.size()) {
        skipSep();
        if (i >= d.size()) break;
        char c = d[i];
        if (std::isalpha((unsigned char)c)) {
            cmd = c;
            ++i;
            if (cmd == 'Z' || cmd == 'z') {
                // Cierra path - no agrega puntos adicionales.
                continue;
            }
            continue;
        }
        // Numero implicito - usar cmd anterior (L/l despues de M, M despues de Z).
        float x, y;
        if (!readNum(x)) break;
        if (!readNum(y)) break;
        if (cmd == 'M' || cmd == 'L') {
            cur = {x, y};
        } else if (cmd == 'm' || cmd == 'l') {
            cur = {cur.x + x, cur.y + y};
        } else {
            // comando no soportado (C, Q, A, etc.): tratar como L.
            cur = {x, y};
        }
        points_.push_back(cur);
    }

    recomputeBBox();
    return points_.size() >= 3;
}

void CountrySilhouette::recomputeBBox() {
    if (points_.empty()) {
        bboxMin_ = {0.f, 0.f};
        bboxMax_ = {1.f, 1.f};
        return;
    }
    bboxMin_ = points_[0];
    bboxMax_ = points_[0];
    for (const auto& p : points_) {
        bboxMin_.x = std::min(bboxMin_.x, p.x);
        bboxMin_.y = std::min(bboxMin_.y, p.y);
        bboxMax_.x = std::max(bboxMax_.x, p.x);
        bboxMax_.y = std::max(bboxMax_.y, p.y);
    }
}

void CountrySilhouette::draw(sf::RenderWindow& win, float cx, float cy, float radius,
                             sf::Color fill, sf::Color outline) const {
    if (points_.size() < 3) return;
    float w = bboxMax_.x - bboxMin_.x;
    float h = bboxMax_.y - bboxMin_.y;
    if (w <= 0.f || h <= 0.f) return;
    float maxDim = std::max(w, h);
    float scale = (radius * 2.f) / maxDim;
    float bx = (bboxMin_.x + bboxMax_.x) * 0.5f;
    float by = (bboxMin_.y + bboxMax_.y) * 0.5f;
    auto transform = [&](sf::Vector2f p) -> sf::Vector2f {
        return { cx + (p.x - bx) * scale, cy + (p.y - by) * scale };
    };

    // Centroide (promedio simple) para triangle fan.
    sf::Vector2f centroid{0.f, 0.f};
    for (const auto& p : points_) { centroid.x += p.x; centroid.y += p.y; }
    centroid.x /= (float)points_.size();
    centroid.y /= (float)points_.size();
    sf::Vector2f c2 = transform(centroid);

    // Fill triangle fan.
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan);
    fan.append(sf::Vertex{c2, fill, {}});
    for (size_t i = 0; i <= points_.size(); ++i) {
        auto p = transform(points_[i % points_.size()]);
        fan.append(sf::Vertex{p, fill, {}});
    }
    win.draw(fan);

    // Outline (line strip).
    sf::VertexArray outlineVA(sf::PrimitiveType::LineStrip);
    for (size_t i = 0; i <= points_.size(); ++i) {
        auto p = transform(points_[i % points_.size()]);
        outlineVA.append(sf::Vertex{p, outline, {}});
    }
    win.draw(outlineVA);
}
