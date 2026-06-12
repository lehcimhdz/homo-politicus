#include "ui/IsoBuilding.hpp"
#include "ui/AssetManager.hpp"
#include <cmath>
#include <algorithm>

void IsoBuilding::configure(const IsoWorld& world, const CountrySilhouette& sil,
                            sf::Vector2f homePos, float homeRadius,
                            const Country& country) {
    (void)sil; (void)homePos; (void)homeRadius;
    buildings_.clear();
    if (world.tiles().empty()) return;

    // Encontrar tiles urban dentro de la silueta.
    std::vector<size_t> urbanIdx;
    int centerGx = 0, centerGy = 0;
    int countInside = 0;
    for (size_t i = 0; i < world.tiles().size(); ++i) {
        const auto& t = world.tiles()[i];
        if (!t.inside) continue;
        ++countInside;
        centerGx += t.gx;
        centerGy += t.gy;
        if (t.biome == IsoWorld::Biome::Urban) urbanIdx.push_back(i);
    }
    if (countInside == 0) return;
    float avgGx = (float)centerGx / countInside;
    float avgGy = (float)centerGy / countInside;

    // Capital: tile urban mas cercano al centro (o cualquier inside si no hay urban).
    size_t capitalIdx = (size_t)-1;
    float bestD = 1e9f;
    auto consider = [&](size_t i) {
        const auto& t = world.tiles()[i];
        float dx = (float)t.gx - avgGx;
        float dy = (float)t.gy - avgGy;
        float d2 = dx*dx + dy*dy;
        if (d2 < bestD) { bestD = d2; capitalIdx = i; }
    };
    if (!urbanIdx.empty()) for (size_t i : urbanIdx) consider(i);
    else for (size_t i = 0; i < world.tiles().size(); ++i) if (world.tiles()[i].inside) consider(i);
    if (capitalIdx == (size_t)-1) return;

    // GDP determina escala global de altura (log10).
    float gdpFactor = (float)std::log10(std::max(1e6, country.economy.gdp)) - 6.f; // ~0..4
    if (gdpFactor < 0.5f) gdpFactor = 0.5f;
    if (gdpFactor > 4.0f) gdpFactor = 4.0f;

    Building cap;
    cap.gx = (float)world.tiles()[capitalIdx].gx + 0.5f;
    cap.gy = (float)world.tiles()[capitalIdx].gy + 0.5f;
    cap.w = 1.6f; cap.d = 1.6f;
    cap.h = 1.6f + gdpFactor * 0.6f;
    cap.kind = Kind::Capital;
    cap.baseColor = sf::Color(210, 175, 80);
    buildings_.push_back(cap);

    // Ciudades secundarias: hasta 9 urban tiles esparcidos con tipos variados.
    std::vector<size_t> remainingUrban;
    for (size_t i : urbanIdx) if (i != capitalIdx) remainingUrban.push_back(i);
    int targetCities = std::min<int>(9, (int)remainingUrban.size());
    if (targetCities > 0) {
        float step = (float)remainingUrban.size() / (float)targetCities;
        for (int k = 0; k < targetCities; ++k) {
            size_t idx = remainingUrban[(size_t)(k * step)];
            const auto& t = world.tiles()[idx];
            Building b;
            b.gx = (float)t.gx + 0.5f;
            b.gy = (float)t.gy + 0.5f;
            b.w = 0.9f; b.d = 0.9f;
            unsigned hash = (unsigned)(t.gx * 73856093 ^ t.gy * 19349663);
            float hFactor = 0.4f + ((hash >> 8) & 0xFF) / 255.f * 0.7f;
            b.h = hFactor + gdpFactor * 0.25f;
            // Asignacion de tipo: el primero Market, segundo Temple, tercero Fortress
            // si hay guerra, resto Residential.
            if (k == 0) { b.kind = Kind::Market;   b.baseColor = sf::Color(160, 120, 80); }
            else if (k == 1) { b.kind = Kind::Temple; b.baseColor = sf::Color(210, 200, 180); b.h *= 1.2f; }
            else if (k == 2 && country.security.war_active) {
                b.kind = Kind::Fortress; b.baseColor = sf::Color(110, 110, 120);
                b.w *= 1.3f; b.d *= 1.3f;
            } else {
                b.kind = Kind::Residential;
                uint8_t cr = 90  + (hash & 0x3F);
                uint8_t cg = 95  + ((hash >> 6) & 0x3F);
                uint8_t cb = 130 + ((hash >> 12) & 0x3F);
                b.baseColor = sf::Color(cr, cg, cb);
            }
            buildings_.push_back(b);
        }
    }
}

void IsoBuilding::updateForCountry(const Country& country) {
    // Re-escalar alturas con GDP actual (sin reposicionar).
    float gdpFactor = (float)std::log10(std::max(1e6, country.economy.gdp)) - 6.f;
    if (gdpFactor < 0.5f) gdpFactor = 0.5f;
    if (gdpFactor > 4.0f) gdpFactor = 4.0f;
    for (auto& b : buildings_) {
        if (b.kind == Kind::Capital) {
            b.h = 1.6f + gdpFactor * 0.6f;
        } else {
            unsigned hash = (unsigned)((int)(b.gx * 100) * 73856093 ^ (int)(b.gy * 100) * 19349663);
            float hFactor = 0.4f + ((hash >> 8) & 0xFF) / 255.f * 0.7f;
            b.h = hFactor + gdpFactor * 0.25f;
        }
    }
}

sf::Vector2f IsoBuilding::screenAt(size_t idx, const IsoCamera& cam) const {
    if (idx >= buildings_.size()) return {0.f, 0.f};
    const auto& b = buildings_[idx];
    return cam.worldToScreen(b.gx, b.gy, 0.f);
}

void IsoBuilding::drawPrism(sf::RenderWindow& win, const IsoCamera& cam,
                            float gx, float gy, float w, float d, float h,
                            sf::Color base, float nightAmount,
                            const sf::Texture* wallTex,
                            const sf::Texture* roofTex) {
    // Esquinas del cubo (base z=0, top z=h).
    sf::Vector2f baseBL = cam.worldToScreen(gx - w*0.5f, gy + d*0.5f, 0.f); // bottom-left
    sf::Vector2f baseBR = cam.worldToScreen(gx + w*0.5f, gy + d*0.5f, 0.f); // bottom-right
    sf::Vector2f baseTL = cam.worldToScreen(gx - w*0.5f, gy - d*0.5f, 0.f); // top-left
    sf::Vector2f topBL  = cam.worldToScreen(gx - w*0.5f, gy + d*0.5f, h);
    sf::Vector2f topBR  = cam.worldToScreen(gx + w*0.5f, gy + d*0.5f, h);
    sf::Vector2f topTL  = cam.worldToScreen(gx - w*0.5f, gy - d*0.5f, h);
    sf::Vector2f topTR  = cam.worldToScreen(gx + w*0.5f, gy - d*0.5f, h);

    auto shade = [&](sf::Color c, float k) {
        c.r = (uint8_t)std::clamp((int)(c.r * k), 0, 255);
        c.g = (uint8_t)std::clamp((int)(c.g * k), 0, 255);
        c.b = (uint8_t)std::clamp((int)(c.b * k), 0, 255);
        c.r = (uint8_t)(c.r * (1.f - 0.35f * nightAmount));
        c.g = (uint8_t)(c.g * (1.f - 0.35f * nightAmount));
        c.b = (uint8_t)(c.b * (1.f - 0.25f * nightAmount));
        return c;
    };

    // Cara izquierda (front-left): base BL, base TL, top TL, top BL.
    sf::Color leftColor  = shade(base, 0.65f);
    sf::ConvexShape left(4);
    left.setPoint(0, baseBL);
    left.setPoint(1, baseTL);
    left.setPoint(2, topTL);
    left.setPoint(3, topBL);
    if (wallTex) {
        left.setTexture(wallTex);
        left.setFillColor(shade(sf::Color(255, 255, 255), 0.65f));
    } else {
        left.setFillColor(leftColor);
    }
    left.setOutlineColor(sf::Color(0,0,0,80));
    left.setOutlineThickness(0.6f);
    win.draw(left);

    // Cara derecha (front-right): base BR, base BL, top BL, top BR.
    sf::Color rightColor = shade(base, 0.85f);
    sf::ConvexShape right(4);
    right.setPoint(0, baseBR);
    right.setPoint(1, baseBL);
    right.setPoint(2, topBL);
    right.setPoint(3, topBR);
    if (wallTex) {
        right.setTexture(wallTex);
        right.setFillColor(shade(sf::Color(255, 255, 255), 0.85f));
    } else {
        right.setFillColor(rightColor);
    }
    right.setOutlineColor(sf::Color(0,0,0,80));
    right.setOutlineThickness(0.6f);
    win.draw(right);

    // Cara superior: top TL, top TR, top BR, top BL.
    sf::Color topColor   = shade(base, 1.05f);
    sf::ConvexShape top(4);
    top.setPoint(0, topTL);
    top.setPoint(1, topTR);
    top.setPoint(2, topBR);
    top.setPoint(3, topBL);
    if (roofTex) {
        top.setTexture(roofTex);
        top.setFillColor(shade(sf::Color(255, 255, 255), 1.0f));
    } else {
        top.setFillColor(topColor);
    }
    top.setOutlineColor(sf::Color(0,0,0,90));
    top.setOutlineThickness(0.8f);
    win.draw(top);
}

void IsoBuilding::draw(sf::RenderWindow& win, const IsoCamera& cam,
                       const Country& country, float nightAmount,
                       float sunPhase) const {
    (void)country;
    // Ordenar por (gy, gx) para que los traseros se rendericen primero (painter's algo).
    std::vector<size_t> order(buildings_.size());
    for (size_t i = 0; i < buildings_.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        const auto& A = buildings_[a];
        const auto& B = buildings_[b];
        float ka = A.gx + A.gy;
        float kb = B.gx + B.gy;
        return ka < kb;
    });
    float z = cam.zoom();
    for (size_t i : order) {
        const auto& b = buildings_[i];
        // Sombra direccional rotando con sunPhase + atenuacion nocturna.
        {
            // sunPhase: 0 = noon, +-PI/2 = horizonte, PI = midnight.
            // Sombra apunta opuesto al sol. shadowLength escala con sin de altura solar.
            float sunAltitude = std::cos(sunPhase);   // 1=high, -1=low
            float shadowLen = 0.15f + 0.45f * std::max(0.f, -sunAltitude + 1.f) * 0.5f;
            // Direccion en world space: sol rotando en plano XY.
            float dx = std::sin(sunPhase) * shadowLen * b.h;
            float dy = std::cos(sunPhase + 0.4f) * shadowLen * b.h;
            // Cuando nightAmount muy alto, casi sin sombra.
            uint8_t alpha = (uint8_t)((1.f - nightAmount) * 130 + 20);
            sf::Vector2f shCenter = cam.worldToScreen(b.gx + dx, b.gy + dy, 0.f);
            sf::ConvexShape shadow(4);
            float sw = b.w * IsoCamera::kTileW * 0.55f * z;
            float sh = b.d * IsoCamera::kTileH * 0.55f * z;
            shadow.setPoint(0, {shCenter.x,           shCenter.y - sh * 0.5f});
            shadow.setPoint(1, {shCenter.x + sw*0.5f, shCenter.y});
            shadow.setPoint(2, {shCenter.x,           shCenter.y + sh * 0.5f});
            shadow.setPoint(3, {shCenter.x - sw*0.5f, shCenter.y});
            // Tinte azulado de noche, neutro de dia.
            sf::Color shCol = nightAmount > 0.5f
                ? sf::Color(20, 20, 40, alpha)
                : sf::Color(0, 0, 0, alpha);
            shadow.setFillColor(shCol);
            win.draw(shadow);
        }

        // Seleccionar texturas segun kind.
        const sf::Texture* wallTex = nullptr;
        const sf::Texture* roofTex = AssetManager::instance().getTexture("texture_roof");
        if (b.kind == Kind::Capital) {
            wallTex = AssetManager::instance().getTexture("texture_marble");
        } else {
            wallTex = AssetManager::instance().getTexture("texture_brick");
        }
        drawPrism(win, cam, b.gx, b.gy, b.w, b.d, b.h, b.baseColor, nightAmount,
                  wallTex, roofTex);

        // Decoracion por kind: toldo Market, torre Temple, mura Fortress, chimenea Residential.
        sf::Vector2f topCtr = cam.worldToScreen(b.gx, b.gy, b.h);
        if (b.kind == Kind::Market) {
            // Toldo rojo sobre el techo.
            sf::RectangleShape awning({b.w * IsoCamera::kTileW * 0.9f * z, b.d * IsoCamera::kTileH * 0.3f * z});
            awning.setOrigin({awning.getSize().x / 2.f, awning.getSize().y / 2.f});
            awning.setPosition({topCtr.x, topCtr.y - 3.f * z});
            awning.setFillColor(sf::Color(180, 60, 40));
            awning.setOutlineColor(sf::Color(120, 30, 20));
            awning.setOutlineThickness(0.6f);
            win.draw(awning);
        } else if (b.kind == Kind::Temple) {
            // Torre puntiaguda encima.
            sf::Vector2f baseSp = cam.worldToScreen(b.gx, b.gy, b.h);
            sf::Vector2f tipSp  = cam.worldToScreen(b.gx, b.gy, b.h + 1.2f);
            sf::ConvexShape spire(3);
            spire.setPoint(0, tipSp);
            spire.setPoint(1, {baseSp.x + 6.f * z, baseSp.y});
            spire.setPoint(2, {baseSp.x - 6.f * z, baseSp.y});
            spire.setFillColor(sf::Color(220, 200, 170));
            spire.setOutlineColor(sf::Color(140, 120, 90));
            spire.setOutlineThickness(0.6f);
            win.draw(spire);
            // Cruz en la punta.
            sf::RectangleShape v({1.5f * z, 6.f * z});
            v.setOrigin({0.75f * z, 5.f * z});
            v.setPosition(tipSp);
            v.setFillColor(sf::Color(240, 220, 90));
            win.draw(v);
        } else if (b.kind == Kind::Fortress) {
            // 4 torres en las esquinas.
            const float corners[4][2] = {
                {-b.w * 0.5f, -b.d * 0.5f}, { b.w * 0.5f, -b.d * 0.5f},
                {-b.w * 0.5f,  b.d * 0.5f}, { b.w * 0.5f,  b.d * 0.5f},
            };
            for (int k = 0; k < 4; ++k) {
                drawPrism(win, cam, b.gx + corners[k][0], b.gy + corners[k][1],
                          0.35f, 0.35f, b.h + 0.5f,
                          sf::Color(140, 140, 150), nightAmount);
            }
        } else if (b.kind == Kind::Residential) {
            // Chimenea con humo (si clima frio - no podemos saberlo aqui, mostramos siempre).
            sf::Vector2f chimBot = cam.worldToScreen(b.gx + b.w * 0.30f, b.gy - b.d * 0.30f, b.h);
            sf::Vector2f chimTop = cam.worldToScreen(b.gx + b.w * 0.30f, b.gy - b.d * 0.30f, b.h + 0.4f);
            sf::RectangleShape chim({3.f * z, std::abs(chimTop.y - chimBot.y) + 2.f});
            chim.setOrigin({1.5f * z, 0.f});
            chim.setPosition(chimTop);
            chim.setFillColor(sf::Color(80, 50, 40));
            win.draw(chim);
            // 3 puff de humo.
            for (int p = 0; p < 3; ++p) {
                float py = chimTop.y - (float)p * 5.f * z;
                float radius = 2.f * z + (float)p * 0.8f * z;
                sf::CircleShape puff(radius);
                puff.setOrigin({radius, radius});
                puff.setPosition({chimTop.x + (float)p * 1.5f * z, py});
                uint8_t a = (uint8_t)(180 - p * 50);
                puff.setFillColor(sf::Color(200, 200, 210, a));
                win.draw(puff);
            }
        }

        // Ventanas iluminadas de noche en City o Residential.
        if (b.kind == Kind::City || b.kind == Kind::Residential) {
            sf::Vector2f topBL = cam.worldToScreen(b.gx - b.w*0.5f, b.gy + b.d*0.5f, b.h);
            sf::Vector2f topBR = cam.worldToScreen(b.gx + b.w*0.5f, b.gy + b.d*0.5f, b.h);
            sf::Vector2f baseBL = cam.worldToScreen(b.gx - b.w*0.5f, b.gy + b.d*0.5f, 0.f);
            sf::Vector2f baseBR = cam.worldToScreen(b.gx + b.w*0.5f, b.gy + b.d*0.5f, 0.f);
            (void)topBL; (void)topBR;
            // Grid 3 cols x 3 rows con sprites rectangulares + cruz interna.
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 3; ++col) {
                    float tx = (col + 1.f) / 4.f;
                    float ty = (row + 1.f) / 4.f;
                    sf::Vector2f anchor = baseBR + (baseBL - baseBR) * tx;
                    sf::Vector2f anchorTop = cam.worldToScreen(b.gx - b.w*0.5f + b.w * tx, b.gy + b.d*0.5f, b.h);
                    sf::Vector2f winPos = anchor + (anchorTop - anchor) * ty;
                    unsigned h = (unsigned)((int)(b.gx * 100) ^ (int)(b.gy * 100 + row * 17 + col * 31));
                    bool lit = (h & 0x3) != 0;
                    sf::Color winColor;
                    if (lit && nightAmount > 0.3f) {
                        uint8_t a = (uint8_t)std::min(255.f, 100.f + 155.f * nightAmount);
                        winColor = sf::Color(255, 230, 130, a);
                    } else {
                        winColor = sf::Color(40, 40, 50, 200);
                    }
                    // Ventana rectangular pequeña, ligeramente rotada por la perspectiva iso.
                    float ww = 3.2f * z;
                    float wh = 4.5f * z;
                    sf::RectangleShape body({ww, wh});
                    body.setOrigin({ww / 2.f, wh / 2.f});
                    body.setPosition(winPos);
                    body.setRotation(sf::degrees(-15.f)); // tilt iso aprox
                    body.setFillColor(winColor);
                    body.setOutlineColor(sf::Color(30, 25, 20, 200));
                    body.setOutlineThickness(0.5f);
                    win.draw(body);
                    // Cruz interna (mullion) para marco.
                    sf::Color frame(80, 60, 40, 220);
                    sf::RectangleShape vert({0.6f * z, wh * 0.85f});
                    vert.setOrigin({0.3f * z, wh * 0.425f});
                    vert.setPosition(winPos);
                    vert.setRotation(sf::degrees(-15.f));
                    vert.setFillColor(frame);
                    win.draw(vert);
                    sf::RectangleShape horiz({ww * 0.85f, 0.6f * z});
                    horiz.setOrigin({ww * 0.425f, 0.3f * z});
                    horiz.setPosition(winPos);
                    horiz.setRotation(sf::degrees(-15.f));
                    horiz.setFillColor(frame);
                    win.draw(horiz);
                }
            }
        }
        // Capitolio: cupula, columnas y bandera.
        if (b.kind == Kind::Capital) {
            // Cupula: semicirculo encima del top center con textura dorada.
            sf::Vector2f topCenter = cam.worldToScreen(b.gx, b.gy, b.h);
            float domeR = b.w * IsoCamera::kTileW * 0.4f * z;
            sf::CircleShape dome(domeR);
            dome.setOrigin({domeR, domeR});
            dome.setPosition(topCenter);
            const sf::Texture* goldTex = AssetManager::instance().getTexture("texture_gold");
            if (goldTex) {
                dome.setTexture(goldTex);
                sf::Color tint(255, 255, 255);
                tint.r = (uint8_t)(tint.r * (1.f - 0.3f * nightAmount));
                tint.g = (uint8_t)(tint.g * (1.f - 0.3f * nightAmount));
                tint.b = (uint8_t)(tint.b * (1.f - 0.2f * nightAmount));
                dome.setFillColor(tint);
            } else {
                sf::Color domeColor(230, 195, 100);
                domeColor.r = (uint8_t)(domeColor.r * (1.f - 0.3f * nightAmount));
                domeColor.g = (uint8_t)(domeColor.g * (1.f - 0.3f * nightAmount));
                domeColor.b = (uint8_t)(domeColor.b * (1.f - 0.2f * nightAmount));
                dome.setFillColor(domeColor);
            }
            dome.setOutlineColor(sf::Color(120, 90, 30));
            dome.setOutlineThickness(0.8f);
            win.draw(dome);
            // Columnas: 4 lineas verticales en la fachada frontal.
            sf::Vector2f baseBL = cam.worldToScreen(b.gx - b.w*0.45f, b.gy + b.d*0.5f, 0.f);
            sf::Vector2f baseBR = cam.worldToScreen(b.gx + b.w*0.45f, b.gy + b.d*0.5f, 0.f);
            for (int k = 0; k < 4; ++k) {
                float t = (k + 0.5f) / 4.f;
                sf::Vector2f bot = baseBL + (baseBR - baseBL) * t;
                sf::Vector2f topC = cam.worldToScreen(b.gx - b.w*0.45f + b.w * 0.9f * t, b.gy + b.d*0.5f, b.h * 0.85f);
                sf::Vertex col[2] = {
                    sf::Vertex{bot,  sf::Color(220, 220, 210, 230), {}},
                    sf::Vertex{topC, sf::Color(220, 220, 210, 230), {}},
                };
                win.draw(col, 2, sf::PrimitiveType::Lines);
            }
            // Bandera dorada en la cupula.
            sf::Vector2f flagBase = cam.worldToScreen(b.gx, b.gy, b.h + 0.4f);
            sf::Vector2f flagTip  = cam.worldToScreen(b.gx, b.gy, b.h + 1.1f);
            sf::Vertex pole[2] = {
                sf::Vertex{flagBase, sf::Color(180, 180, 180), {}},
                sf::Vertex{flagTip,  sf::Color(180, 180, 180), {}},
            };
            win.draw(pole, 2, sf::PrimitiveType::Lines);
            sf::ConvexShape flag(3);
            flag.setPoint(0, flagTip);
            flag.setPoint(1, {flagTip.x + 14.f * z, flagTip.y + 4.f * z});
            flag.setPoint(2, {flagTip.x,           flagTip.y + 8.f * z});
            flag.setFillColor(sf::Color(220, 180, 80));
            win.draw(flag);
        }
    }
}
