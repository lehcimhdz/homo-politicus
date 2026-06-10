#ifndef ACTION_PANEL_HPP
#define ACTION_PANEL_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

// ActionPanel: sub-tabs por familia de comandos, botones clickables.
// Sprint 13: estructura + 4 familias + botones que disparan callbacks.
class ActionPanel {
public:
    using ActionCallback = std::function<void(const std::string& commandId)>;

    struct Action {
        std::string id;           // ej "tax_up"
        std::string label;        // ej "Subir impuestos"
        std::string tooltip;      // efecto esperado
    };

    struct Family {
        std::string name;
        std::vector<Action> actions;
    };

    ActionPanel();
    void setCallback(ActionCallback cb) { cb_ = std::move(cb); }
    // Eventos del mouse: pasa coordenadas relativas a la ventana
    void onClick(sf::Vector2f mouse);
    void onMouseMove(sf::Vector2f mouse);
    void draw(sf::RenderWindow& win, const sf::Font& font) const;
    int currentFamily() const { return currentFamily_; }
    void setCurrentFamily(int idx) { if (idx >= 0 && idx < (int)families_.size()) currentFamily_ = idx; }
    const std::vector<Family>& families() const { return families_; }
    const std::string& hoveredActionId() const { return hoveredActionId_; }
    const std::string& hoveredTooltip() const { return hoveredTooltip_; }

private:
    std::vector<Family> families_;
    int currentFamily_ = 0;
    std::string hoveredActionId_;
    std::string hoveredTooltip_;
    ActionCallback cb_;
};

#endif
