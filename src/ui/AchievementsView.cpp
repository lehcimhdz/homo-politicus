#include "ui/AchievementsView.hpp"
#include <cmath>
#include <algorithm>

void AchievementsView::configure(const AchievementTracker& tracker) {
    entries_.clear();
    const auto& cat = AchievementTracker::catalog();
    entries_.reserve(cat.size());
    for (const auto& def : cat) {
        Entry e;
        e.id = def.id;
        e.name = def.name_es;
        e.description = def.description_es;
        e.rarity = rarityFor(def.id);
        e.category = categoryFor(def.id);
        e.unlocked = tracker.isUnlocked(def.id);
        entries_.push_back(e);
    }
}

void AchievementsView::update(const AchievementTracker& tracker) {
    for (auto& e : entries_) {
        e.unlocked = tracker.isUnlocked(e.id);
    }
}

int AchievementsView::unlockedCount() const {
    int n = 0;
    for (const auto& e : entries_) if (e.unlocked) ++n;
    return n;
}

void AchievementsView::onMouseMove(sf::Vector2f mouse) {
    mouse_ = mouse;
    // Hovered se computa en draw (sabe coords de cards).
}

void AchievementsView::onClick(sf::Vector2f mouse) {
    // Click en chips de filtro (top bar).
    if (mouse.y >= 102.f && mouse.y <= 130.f) {
        if (mouse.x >= 218.f && mouse.x <= 290.f) { filter_ = Filter::All;      return; }
        if (mouse.x >= 296.f && mouse.x <= 408.f) { filter_ = Filter::Unlocked; return; }
        if (mouse.x >= 414.f && mouse.x <= 510.f) { filter_ = Filter::Locked;   return; }
    }
}

AchievementsView::Rarity AchievementsView::rarityFor(const std::string& id) const {
    // Heuristica simple por longitud del id + hash de chars.
    unsigned h = 0;
    for (char c : id) h = h * 31u + (unsigned char)c;
    unsigned r = h % 100;
    if (r < 50)      return Rarity::Common;
    else if (r < 80) return Rarity::Uncommon;
    else if (r < 95) return Rarity::Rare;
    else             return Rarity::Legendary;
}

AchievementsView::Category AchievementsView::categoryFor(const std::string& id) const {
    auto contains = [&](const char* needle) -> bool {
        return id.find(needle) != std::string::npos;
    };
    if (contains("election") || contains("legitim") || contains("congress")
     || contains("coup") || contains("regime")) return Category::Politics;
    if (contains("gdp") || contains("growth") || contains("trade")
     || contains("inflation") || contains("fta")) return Category::Economy;
    if (contains("war") || contains("nuclear") || contains("pandemic")
     || contains("crisis") || contains("revolution")
     || contains("scandal")) return Category::Crisis;
    return Category::Milestone;
}

sf::Color AchievementsView::rarityColor(Rarity r) {
    switch (r) {
        case Rarity::Common:    return sf::Color(150, 155, 165);
        case Rarity::Uncommon:  return sf::Color( 80, 200, 120);
        case Rarity::Rare:      return sf::Color( 80, 160, 240);
        case Rarity::Legendary: return sf::Color(240, 200,  80);
    }
    return sf::Color(150, 155, 165);
}

const char* AchievementsView::rarityLabel(Rarity r) {
    switch (r) {
        case Rarity::Common:    return "Common";
        case Rarity::Uncommon:  return "Uncommon";
        case Rarity::Rare:      return "Rare";
        case Rarity::Legendary: return "Legendary";
    }
    return "";
}

void AchievementsView::drawIcon(sf::RenderWindow& win, Category cat,
                                float cx, float cy, float size, sf::Color color) {
    switch (cat) {
        case Category::Politics: {
            // Corona: trapecio + 3 puntas con bolitas.
            sf::ConvexShape band(4);
            band.setPoint(0, {cx - size * 0.45f, cy + size * 0.10f});
            band.setPoint(1, {cx + size * 0.45f, cy + size * 0.10f});
            band.setPoint(2, {cx + size * 0.40f, cy + size * 0.30f});
            band.setPoint(3, {cx - size * 0.40f, cy + size * 0.30f});
            band.setFillColor(color);
            win.draw(band);
            for (int i = -1; i <= 1; ++i) {
                sf::CircleShape pt(size * 0.08f);
                pt.setOrigin({size * 0.08f, size * 0.08f});
                pt.setPosition({cx + (float)i * size * 0.30f, cy - size * 0.10f});
                pt.setFillColor(color);
                win.draw(pt);
                sf::RectangleShape spike({size * 0.06f, size * 0.20f});
                spike.setOrigin({size * 0.03f, size * 0.10f});
                spike.setPosition({cx + (float)i * size * 0.30f, cy});
                spike.setFillColor(color);
                win.draw(spike);
            }
            break;
        }
        case Category::Economy: {
            // Moneda concentrica con $.
            sf::CircleShape coin(size * 0.45f);
            coin.setOrigin({size * 0.45f, size * 0.45f});
            coin.setPosition({cx, cy});
            coin.setFillColor(color);
            coin.setOutlineColor(sf::Color(40, 30, 10, 200));
            coin.setOutlineThickness(1.f);
            win.draw(coin);
            sf::CircleShape inner(size * 0.30f);
            inner.setOrigin({size * 0.30f, size * 0.30f});
            inner.setPosition({cx, cy});
            inner.setFillColor(sf::Color(color.r * 0.7f, color.g * 0.7f, color.b * 0.7f));
            win.draw(inner);
            sf::RectangleShape bar({size * 0.08f, size * 0.45f});
            bar.setOrigin({size * 0.04f, size * 0.225f});
            bar.setPosition({cx, cy});
            bar.setFillColor(color);
            win.draw(bar);
            break;
        }
        case Category::Crisis: {
            // Triangulo de advertencia con !
            sf::ConvexShape tri(3);
            tri.setPoint(0, {cx,            cy - size * 0.50f});
            tri.setPoint(1, {cx + size * 0.45f, cy + size * 0.35f});
            tri.setPoint(2, {cx - size * 0.45f, cy + size * 0.35f});
            tri.setFillColor(color);
            tri.setOutlineColor(sf::Color(50, 20, 20, 200));
            tri.setOutlineThickness(1.f);
            win.draw(tri);
            sf::RectangleShape bang({size * 0.08f, size * 0.30f});
            bang.setOrigin({size * 0.04f, size * 0.15f});
            bang.setPosition({cx, cy - size * 0.04f});
            bang.setFillColor(sf::Color(30, 20, 20));
            win.draw(bang);
            sf::CircleShape dot(size * 0.05f);
            dot.setOrigin({size * 0.05f, size * 0.05f});
            dot.setPosition({cx, cy + size * 0.20f});
            dot.setFillColor(sf::Color(30, 20, 20));
            win.draw(dot);
            break;
        }
        case Category::Milestone: {
            // Estrella 5 puntas.
            sf::ConvexShape star(10);
            for (int i = 0; i < 10; ++i) {
                float ang = -1.5708f + i * (3.14159f / 5.f);
                float rad = (i % 2 == 0) ? size * 0.50f : size * 0.22f;
                star.setPoint(i, {cx + std::cos(ang) * rad, cy + std::sin(ang) * rad});
            }
            star.setFillColor(color);
            star.setOutlineColor(sf::Color(40, 30, 10, 200));
            star.setOutlineThickness(1.f);
            win.draw(star);
            break;
        }
    }
}

void AchievementsView::draw(sf::RenderWindow& win, const sf::Font& font,
                            float x, float y, float w, float h) const {
    // Top bar: filtros + contador.
    const float topY = y;
    const float topH = 28.f;
    auto drawChip = [&](const std::string& label, float bx, float bw, bool active) {
        sf::RectangleShape chip({bw, topH});
        chip.setPosition({bx, topY});
        chip.setFillColor(active ? sf::Color(80, 160, 240, 80)
                                 : sf::Color(40, 44, 58, 200));
        chip.setOutlineColor(active ? sf::Color(80, 160, 240) : sf::Color(70, 78, 100));
        chip.setOutlineThickness(1.f);
        win.draw(chip);
        sf::Text t(font, label, 12);
        t.setStyle(active ? sf::Text::Bold : sf::Text::Regular);
        t.setFillColor(active ? sf::Color(255, 255, 255) : sf::Color(180, 184, 200));
        auto lb = t.getLocalBounds();
        t.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
        t.setPosition({bx + bw / 2.f, topY + topH / 2.f});
        win.draw(t);
    };
    drawChip("Todos",         218.f,  72.f, filter_ == Filter::All);
    drawChip("Desbloqueados", 296.f, 112.f, filter_ == Filter::Unlocked);
    drawChip("Bloqueados",    414.f,  96.f, filter_ == Filter::Locked);

    // Contador.
    {
        std::string s = std::to_string(unlockedCount()) + " / " + std::to_string(totalCount()) + " desbloqueados";
        sf::Text t(font, s, 12);
        t.setFillColor(sf::Color(180, 184, 200));
        t.setPosition({w + x - 220.f, topY + 8.f});
        win.draw(t);
    }

    // Grid 4 cols × N rows.
    const float gridY = y + topH + 10.f;
    const int cols = 4;
    const float cardW = 190.f;
    const float cardH = 100.f;
    const float gap = 8.f;

    // Filtrar entries.
    std::vector<const Entry*> visible;
    visible.reserve(entries_.size());
    for (const auto& e : entries_) {
        if (filter_ == Filter::Unlocked && !e.unlocked) continue;
        if (filter_ == Filter::Locked   &&  e.unlocked) continue;
        visible.push_back(&e);
    }

    // Determinar hovered.
    int hov = -1;
    for (size_t i = 0; i < visible.size(); ++i) {
        int col = (int)i % cols;
        int row = (int)i / cols;
        float cx = x + (float)col * (cardW + gap);
        float cy = gridY + (float)row * (cardH + gap);
        if (cy + cardH > y + h) break;
        if (mouse_.x >= cx && mouse_.x <= cx + cardW &&
            mouse_.y >= cy && mouse_.y <= cy + cardH) {
            hov = (int)i;
        }
    }
    const_cast<int&>(hovered_) = hov;

    int idx = 0;
    int hoveredEntryIdx = -1;
    for (const Entry* ep : visible) {
        const auto& e = *ep;
        int col = idx % cols;
        int row = idx / cols;
        float cx = x + (float)col * (cardW + gap);
        float cy = gridY + (float)row * (cardH + gap);
        if (cy + cardH > y + h) break;

        sf::Color rcolor = rarityColor(e.rarity);
        sf::Color fill = e.unlocked
            ? sf::Color(48, 46, 36, 230)
            : sf::Color(28, 30, 38, 230);
        sf::Color outline = e.unlocked ? rcolor : sf::Color(70, 75, 90);
        if (idx == hov) {
            fill.r = (uint8_t)std::min(255, fill.r + 18);
            fill.g = (uint8_t)std::min(255, fill.g + 18);
            fill.b = (uint8_t)std::min(255, fill.b + 18);
            hoveredEntryIdx = idx;
        }

        sf::RectangleShape card({cardW, cardH});
        card.setPosition({cx, cy});
        card.setFillColor(fill);
        card.setOutlineColor(outline);
        card.setOutlineThickness(e.unlocked ? 2.f : 1.f);
        win.draw(card);

        // Icono.
        sf::Color iconColor = e.unlocked ? rcolor : sf::Color(80, 85, 100);
        drawIcon(win, e.category, cx + 24.f, cy + 32.f, 32.f, iconColor);

        // Nombre.
        std::string name = e.name;
        if (name.size() > 22) name = name.substr(0, 21) + "...";
        sf::Text n(font, name, 12);
        n.setStyle(sf::Text::Bold);
        n.setFillColor(e.unlocked ? sf::Color(230, 232, 240) : sf::Color(140, 145, 160));
        n.setPosition({cx + 50.f, cy + 8.f});
        win.draw(n);

        // Rareza label.
        sf::Text r(font, rarityLabel(e.rarity), 9);
        r.setFillColor(rcolor);
        r.setPosition({cx + 50.f, cy + 26.f});
        win.draw(r);

        // Estado.
        const char* statusLbl = e.unlocked ? "DESBLOQUEADO" : "Bloqueado";
        sf::Color statusCol = e.unlocked ? rcolor : sf::Color(110, 115, 130);
        sf::Text st(font, statusLbl, 9);
        st.setFillColor(statusCol);
        st.setPosition({cx + 50.f, cy + 42.f});
        win.draw(st);

        // Descripcion truncada.
        std::string desc = e.description;
        if (desc.size() > 30) desc = desc.substr(0, 28) + "..";
        sf::Text d(font, desc, 9);
        d.setFillColor(e.unlocked ? sf::Color(180, 185, 200) : sf::Color(100, 105, 120));
        d.setPosition({cx + 8.f, cy + 68.f});
        win.draw(d);

        // Barra de progreso parcial.
        if (e.progressTarget > 0 && e.progressCurrent < e.progressTarget) {
            float t = (float)e.progressCurrent / (float)e.progressTarget;
            sf::RectangleShape bg({cardW - 16.f, 3.f});
            bg.setPosition({cx + 8.f, cy + cardH - 8.f});
            bg.setFillColor(sf::Color(60, 65, 80));
            win.draw(bg);
            sf::RectangleShape fg({(cardW - 16.f) * t, 3.f});
            fg.setPosition({cx + 8.f, cy + cardH - 8.f});
            fg.setFillColor(rcolor);
            win.draw(fg);
        }
        ++idx;
    }

    // Tooltip de hovered.
    if (hoveredEntryIdx >= 0 && hoveredEntryIdx < (int)visible.size()) {
        const auto& e = *visible[hoveredEntryIdx];
        float tw = 320.f;
        float th = 70.f;
        float tx = mouse_.x + 14.f;
        float ty = mouse_.y + 14.f;
        if (tx + tw > x + w) tx = mouse_.x - tw - 14.f;
        if (ty + th > y + h) ty = mouse_.y - th - 14.f;
        sf::RectangleShape tip({tw, th});
        tip.setPosition({tx, ty});
        tip.setFillColor(sf::Color(20, 22, 30, 235));
        tip.setOutlineColor(rarityColor(e.rarity));
        tip.setOutlineThickness(1.5f);
        win.draw(tip);
        sf::Text n(font, e.name, 12);
        n.setStyle(sf::Text::Bold);
        n.setFillColor(rarityColor(e.rarity));
        n.setPosition({tx + 8.f, ty + 6.f});
        win.draw(n);
        sf::Text d(font, e.description, 10);
        d.setFillColor(sf::Color(220, 224, 235));
        d.setPosition({tx + 8.f, ty + 26.f});
        win.draw(d);
        sf::Text r(font, std::string(rarityLabel(e.rarity)) + (e.unlocked ? " | DESBLOQUEADO" : " | Bloqueado"),
                   9);
        r.setFillColor(sf::Color(150, 155, 170));
        r.setPosition({tx + 8.f, ty + 50.f});
        win.draw(r);
    }
}
