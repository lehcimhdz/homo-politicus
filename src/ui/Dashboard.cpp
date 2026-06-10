#include "ui/Dashboard.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

static const sf::Color kPanel  = sf::Color(32, 36, 48);
static const sf::Color kBorder = sf::Color(60, 65, 80);
static const sf::Color kText   = sf::Color(220, 222, 232);
static const sf::Color kMuted  = sf::Color(150, 154, 168);
static const sf::Color kAccent = sf::Color(80, 160, 240);
static const sf::Color kGood   = sf::Color(80, 200, 120);
static const sf::Color kBad    = sf::Color(220, 80, 80);
static const sf::Color kWarn   = sf::Color(240, 180, 60);

static const float kCardW = 260.f;
static const float kCardH = 200.f;
static const float kGap   = 16.f;

Dashboard::Dashboard() {
    popHist.reserve(kHistMax);
    gdpHist.reserve(kHistMax);
    inflHist.reserve(kHistMax);
}

void Dashboard::recordHistory(const Country& c) {
    auto pushClamped = [](std::vector<double>& v, double val) {
        v.push_back(val);
        if (v.size() > kHistMax) v.erase(v.begin());
    };
    pushClamped(popHist, c.politics.popularity);
    pushClamped(gdpHist, c.economy.gdp);
    pushClamped(inflHist, c.economy.inflation);
}

static sf::Text makeText(const sf::Font& font, const std::string& s, unsigned size, sf::Color color, float x, float y) {
    sf::Text t(font, s, size);
    t.setFillColor(color);
    t.setPosition({x, y});
    return t;
}

static std::string fmtPct(double v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << (v * 100) << "%";
    return oss.str();
}

static std::string fmtMoney(double v) {
    std::ostringstream oss;
    if (v >= 1e9)      oss << "$" << std::fixed << std::setprecision(1) << v / 1e9 << "B";
    else if (v >= 1e6) oss << "$" << std::fixed << std::setprecision(0) << v / 1e6 << "M";
    else               oss << "$" << std::fixed << std::setprecision(0) << v;
    return oss.str();
}

static sf::Color popColor(double pop) {
    if (pop < 0.30) return kBad;
    if (pop < 0.50) return kWarn;
    return kGood;
}

static void drawSparkline(sf::RenderWindow& win, float x, float y, float w, float h,
                          const std::vector<double>& data, sf::Color color,
                          double yMin, double yMax) {
    if (data.size() < 2) return;
    double range = yMax - yMin; if (range <= 0) range = 1.0;
    for (size_t i = 1; i < data.size(); ++i) {
        float x1 = x + (float)(i - 1) * w / (float)(data.size() - 1);
        float x2 = x + (float)i       * w / (float)(data.size() - 1);
        float y1 = y + h - (float)((data[i - 1] - yMin) / range) * h;
        float y2 = y + h - (float)((data[i]     - yMin) / range) * h;
        sf::Vertex line[2];
        line[0].position = {x1, y1}; line[0].color = color;
        line[1].position = {x2, y2}; line[1].color = color;
        win.draw(line, 2, sf::PrimitiveType::Lines);
    }
}

static void drawBar(sf::RenderWindow& win, float x, float y, float w, float h, double ratio, sf::Color fg) {
    sf::RectangleShape bg({w, h});
    bg.setPosition({x, y});
    bg.setFillColor(sf::Color(50, 55, 70));
    win.draw(bg);
    if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
    sf::RectangleShape fill({(float)(w * ratio), h});
    fill.setPosition({x, y});
    fill.setFillColor(fg);
    win.draw(fill);
}

void Dashboard::drawCard(sf::RenderWindow& win, const sf::Font& font,
                         float x, float y, float w, float h,
                         const std::string& title) const {
    sf::RectangleShape r({w, h});
    r.setPosition({x, y});
    r.setFillColor(kPanel);
    r.setOutlineColor(kBorder);
    r.setOutlineThickness(1.f);
    win.draw(r);
    win.draw(makeText(font, title, 14, kMuted, x + 12, y + 10));
}

void Dashboard::drawPopularityCard(sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const {
    drawCard(win, font, x, y, kCardW, kCardH, "POPULARIDAD");
    auto col = popColor(c.politics.popularity);
    win.draw(makeText(font, fmtPct(c.politics.popularity), 48, col, x + 12, y + 30));
    drawBar(win, x + 12, y + 96, kCardW - 24, 10, c.politics.popularity, col);
    drawSparkline(win, x + 12, y + 120, kCardW - 24, 60, popHist, kAccent, 0.0, 1.0);
}

void Dashboard::drawEconomyCard(sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const {
    drawCard(win, font, x, y, kCardW, kCardH, "ECONOMIA");
    win.draw(makeText(font, "GDP", 12, kMuted, x + 12, y + 32));
    win.draw(makeText(font, fmtMoney(c.economy.gdp), 22, kText, x + 12, y + 46));

    std::ostringstream infl;
    infl << "Inflacion: " << std::fixed << std::setprecision(1) << (c.economy.inflation * 100) << "%";
    win.draw(makeText(font, infl.str(), 13, kText, x + 12, y + 78));

    std::ostringstream gr;
    gr << "Crecimiento: " << std::fixed << std::setprecision(1) << (c.economy.growth_rate * 100) << "%";
    auto colGr = c.economy.growth_rate >= 0 ? kGood : kBad;
    win.draw(makeText(font, gr.str(), 13, colGr, x + 12, y + 96));

    // Sparkline de GDP (normalizado al rango observado)
    if (gdpHist.size() >= 2) {
        double mn = *std::min_element(gdpHist.begin(), gdpHist.end());
        double mx = *std::max_element(gdpHist.begin(), gdpHist.end());
        drawSparkline(win, x + 12, y + 120, kCardW - 24, 60, gdpHist, kGood, mn, mx);
    }
}

void Dashboard::drawPressuresCard(sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const {
    drawCard(win, font, x, y, kCardW, kCardH, "PRESIONES");
    struct Row { const char* label; double val; };
    Row rows[5] = {
        {"Congresional", c.politics.congressional_pressure},
        {"Judicial",     c.politics.judicial_pressure},
        {"Militar",      c.politics.military_pressure},
        {"Popular",      c.politics.popular_pressure},
        {"Internacional",c.politics.international_pressure},
    };
    for (int i = 0; i < 5; ++i) {
        float ry = y + 32 + i * 30;
        win.draw(makeText(font, rows[i].label, 12, kText, x + 12, ry));
        sf::Color col = rows[i].val >= 0.85 ? kBad : rows[i].val >= 0.7 ? kWarn : kAccent;
        drawBar(win, x + 110, ry + 4, kCardW - 130, 12, rows[i].val, col);
    }
}

void Dashboard::drawScandalsCard(sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const {
    drawCard(win, font, x, y, kCardW, kCardH, "ESCANDALOS");
    std::ostringstream count;
    count << c.politics.active_scandals << " activos";
    win.draw(makeText(font, count.str(), 22, c.politics.active_scandals > 0 ? kBad : kGood, x + 12, y + 32));
    struct Row { const char* label; double sev; };
    Row rows[5] = {
        {"Corrupcion",    c.politics.scandal_corruption_severity},
        {"Sexual",        c.politics.scandal_sex_severity},
        {"Financiero",    c.politics.scandal_financial_severity},
        {"Violencia",     c.politics.scandal_violence_severity},
        {"Sustancias",    c.politics.scandal_substance_severity},
    };
    for (int i = 0; i < 5; ++i) {
        float ry = y + 76 + i * 22;
        win.draw(makeText(font, rows[i].label, 11, kMuted, x + 12, ry));
        drawBar(win, x + 100, ry + 2, kCardW - 120, 10, rows[i].sev, kBad);
    }
}

void Dashboard::drawSystemsCard(sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const {
    drawCard(win, font, x, y, kCardW, kCardH, "SISTEMAS DEL PAIS");
    // Agregados simples 0-1 por sistema
    double wel = (c.welfare.literacy_rate + c.welfare.health_coverage + c.welfare.un_score
                 + c.welfare.freedom_of_expression + c.welfare.minority_protection) / 5.0;
    double eco = std::min(1.0, std::max(0.0, c.economy.growth_rate + 0.5));
    double pol = c.politics.regime_legitimacy;
    double sec = c.security.diplomatic_prestige;
    double inf = (c.infra.road_connectivity + c.infra.internet_coverage + c.infra.maintenance_level) / 3.0;
    struct Row { const char* label; double v; };
    Row rows[5] = {
        {"Bienestar",    wel},
        {"Economia",     eco},
        {"Politica",     pol},
        {"Seguridad",    sec},
        {"Infraestruct.",inf},
    };
    for (int i = 0; i < 5; ++i) {
        float ry = y + 32 + i * 30;
        win.draw(makeText(font, rows[i].label, 12, kText, x + 12, ry));
        sf::Color col = rows[i].v >= 0.7 ? kGood : rows[i].v >= 0.4 ? kAccent : kBad;
        drawBar(win, x + 110, ry + 4, kCardW - 130, 12, rows[i].v, col);
    }
}

void Dashboard::drawNeighborsCard(sf::RenderWindow& win, const sf::Font& font, float x, float y, const Country& c) const {
    drawCard(win, font, x, y, kCardW, kCardH, "VECINOS");
    for (size_t i = 0; i < c.neighbors.size() && i < 3; ++i) {
        const auto& n = c.neighbors[i];
        float ry = y + 32 + (float)i * 52;
        win.draw(makeText(font, n.name, 14, kText, x + 12, ry));
        std::ostringstream rel;
        rel << "rel " << std::fixed << std::setprecision(2) << n.diplomatic_relations;
        if (n.at_war) rel << " GUERRA";
        else if (n.has_territorial_claim) rel << " CLAIM";
        sf::Color col = n.at_war ? kBad
                      : n.diplomatic_relations > 0.5 ? kGood
                      : n.diplomatic_relations < -0.3 ? kBad : kAccent;
        win.draw(makeText(font, rel.str(), 12, col, x + 12, ry + 18));
        drawBar(win, x + 12, ry + 38, kCardW - 24, 6, (n.diplomatic_relations + 1.0) / 2.0, col);
    }
}

void Dashboard::draw(sf::RenderWindow& win, const sf::Font& font, const Country& c) const {
    // Grid 3 columnas x 2 filas, x base 218, y base 100
    const float baseX = 218.f;
    const float baseY = 100.f;
    drawPopularityCard(win, font, baseX + 0 * (kCardW + kGap), baseY + 0 * (kCardH + kGap), c);
    drawEconomyCard   (win, font, baseX + 1 * (kCardW + kGap), baseY + 0 * (kCardH + kGap), c);
    drawPressuresCard (win, font, baseX + 2 * (kCardW + kGap), baseY + 0 * (kCardH + kGap), c);
    drawScandalsCard  (win, font, baseX + 0 * (kCardW + kGap), baseY + 1 * (kCardH + kGap), c);
    drawSystemsCard   (win, font, baseX + 1 * (kCardW + kGap), baseY + 1 * (kCardH + kGap), c);
    drawNeighborsCard (win, font, baseX + 2 * (kCardW + kGap), baseY + 1 * (kCardH + kGap), c);
}
