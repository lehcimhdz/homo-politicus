#include "Advisor.hpp"

class HaciendaMinister : public Advisor {
public:
    std::string id() const override { return "hacienda_minister"; }
    std::string name_es() const override { return "Ministra de Hacienda"; }
    std::string respond(const Country& c, const std::string&) const override {
        if (c.economy.inflation > 0.5)
            return "Ministra: la inflacion esta fuera de control. Congelar emision y pacto antiinflacionario son tu unica salida. Cada turno que esperes pierde popularidad.";
        if (c.economy.debt_to_gdp_ratio > 0.8)
            return "Ministra: la deuda al " + std::to_string((int)(c.economy.debt_to_gdp_ratio*100))
                 + "% del PIB es peligrosa. Considera reestructurar antes de un default forzado.";
        if (c.economy.in_recession)
            return "Ministra: recesion confirmada. Estimulo fiscal (con costo politico de inflacion) o paciencia (con costo de desempleo). Elegi.";
        if (c.economy.commodity_supercycle > 0.2)
            return "Ministra: bonanza de commodities. Es el momento de ahorrar al SWF, no de gastar. La fiesta termina siempre.";
        return "Ministra: situacion fiscal estable. Buen momento para invertir en infraestructura o ciencia.";
    }
};

class IntelChief : public Advisor {
public:
    std::string id() const override { return "intel_chief"; }
    std::string name_es() const override { return "Jefe de Inteligencia"; }
    std::string respond(const Country& c, const std::string&) const override {
        if (c.politics.military_pressure > 0.7)
            return "Inteligencia: detectamos reuniones sospechosas en el alto mando. Recomendamos purgar o desplazar oficiales sospechosos antes del fin del turno.";
        if (c.politics.popular_pressure > 0.7)
            return "Inteligencia: la calle se organiza. Movimientos sociales coordinan. Tres opciones: concesiones, represion blanda, represion dura.";
        if (c.politics.judicial_pressure > 0.7)
            return "Inteligencia: fiscales abren causas sensibles. Tus puntos debiles son " + std::to_string(c.politics.active_scandals) + " escandalos activos.";
        if (c.politics.active_scandals >= 2)
            return "Inteligencia: " + std::to_string(c.politics.active_scandals) + " escandalos circulando. Recomendamos counter_narrative o cover_up antes de que se acumulen.";
        return "Inteligencia: situacion bajo control. Sin amenazas internas inminentes detectadas.";
    }
};

class Spokesperson : public Advisor {
public:
    std::string id() const override { return "spokesperson"; }
    std::string name_es() const override { return "Vocero Presidencial"; }
    std::string respond(const Country& c, const std::string&) const override {
        if (c.politics.popularity < 0.3)
            return "Vocero: tu narrativa pierde fuerza. Sugerimos discurso de unidad nacional con foco en logros pasados. Disponible: 'apologize' para reset emocional.";
        if (c.security.war_active)
            return "Vocero: estamos en guerra. Cada comunicado debe encuadrar al enemigo y unificar al pueblo. 'Patria o muerte' funciona.";
        if (c.politics.popularity > 0.7)
            return "Vocero: estas en pico de popularidad. Capitaliza con anuncio grande: ley simbolica, gira nacional, o referendum.";
        return "Vocero: mensaje del dia recomendado: 'crecimiento con responsabilidad'. Aburrido pero efectivo.";
    }
};

class PoliticalAdvisor : public Advisor {
public:
    std::string id() const override { return "political_advisor"; }
    std::string name_es() const override { return "Asesor Politico"; }
    std::string respond(const Country& c, const std::string&) const override {
        if (c.politics.coalition_cohesion < 0.4)
            return "Asesor: tu coalicion se desangra. Hacele una concesion grande al socio mas frio antes que se vaya.";
        if (c.politics.congressional_pressure > 0.6)
            return "Asesor: el congreso te aprieta. Tres jugadas: dividir oposicion comprando 2 votos, llamar a referendum, o aceptar derrota legislativa para conservar capital.";
        int allies = 0;
        for (const auto& n : c.neighbors) if (n.diplomatic_relations > 0.3) allies++;
        if (allies < 1)
            return "Asesor: estas aislado regionalmente. Improve_relations con el menos hostil antes que se cierre la ventana.";
        return "Asesor: tu posicion es estable. Buen momento para una jugada de mediano plazo (reforma estructural).";
    }
};

class OppositionCritic : public Advisor {
public:
    std::string id() const override { return "opposition_critic"; }
    std::string name_es() const override { return "Critico Opositor"; }
    std::string respond(const Country& c, const std::string&) const override {
        if (c.politics.authoritarian_actions_count >= 3)
            return "Critico: '" + std::to_string(c.politics.authoritarian_actions_count)
                 + " acciones autoritarias en menos de un mandato. Esto no es democracia, es una farsa. La historia te juzgara.'";
        if (c.economy.inflation > 0.3)
            return "Critico: 'inflacion del " + std::to_string((int)(c.economy.inflation*100))
                 + "% destruye salarios mientras el gobierno mira para otro lado. Hambre y miseria.'";
        if (c.politics.active_scandals > 0)
            return "Critico: 'escandalos sin investigacion seria. Corrupcion estructural. Justicia para todos, sin excepciones de poderosos.'";
        if (c.welfare.unemployment_rate > 0.1)
            return "Critico: 'desempleo del " + std::to_string((int)(c.welfare.unemployment_rate*100))
                 + "%. Familias que no llegan a fin de mes. El modelo fracaso.'";
        return "Critico: 'cada dia que pasa, este gobierno improvisa. Falta plan, falta liderazgo, falta vision.'";
    }
};

namespace Advisors {

std::vector<std::unique_ptr<Advisor>> all() {
    std::vector<std::unique_ptr<Advisor>> v;
    v.push_back(std::make_unique<HaciendaMinister>());
    v.push_back(std::make_unique<IntelChief>());
    v.push_back(std::make_unique<Spokesperson>());
    v.push_back(std::make_unique<PoliticalAdvisor>());
    v.push_back(std::make_unique<OppositionCritic>());
    return v;
}

Advisor* findById(const std::string& id) {
    static auto advisors = all();
    for (auto& a : advisors) if (a->id() == id) return a.get();
    return nullptr;
}

} // namespace Advisors
