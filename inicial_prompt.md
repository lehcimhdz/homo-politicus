# Homo Politicus — Visión de producto

> Documento vivo. Última revisión: junio 2026.
> Este archivo reemplaza al prompt original de exploración (febrero 2026) y formaliza
> la transición de **proyecto de aprendizaje** a **producto comercial dirigido a Steam Early Access**.

---

## 1. Qué es Homo Politicus

Un simulador político-económico de profundidad inusual donde el jugador encarna a un
líder nacional y enfrenta crisis, conspiraciones, decisiones morales y la presión
constante de mantenerse en el poder. Cada partida es una historia emergente con final
distinto: golpe militar, impeachment, revolución popular, exilio, magnicidio, derrota
electoral o mandato cumplido.

**Tagline:** *Liderá. Sobreviví. Decidí.*

**Géneros:** Strategy, Simulation, Political, Turn-Based, Historical, Singleplayer,
Choices Matter.

**Referencias:** Democracy 4, Suzerain, Crusader Kings, Tropico, Frostpunk.

---

## 2. Propuesta de valor diferenciada

| Competidor | Lo que hace bien | Lo que Homo Politicus hace mejor |
|---|---|---|
| **Democracy 4** | Sistema fiscal/social profundo | Geopolítica, golpes militares, escándalos, escenarios históricos curados |
| **Suzerain** | Narrativa rica, choice-driven | Simulación real (no narrativa pre-escrita): cada variable mueve a otras |
| **Crusader Kings** | Agentes con personalidad | Foco en siglo XX y XXI con países reales, no medievales ficticios |
| **Tropico** | Caricaturesco, accesible | Realismo crudo: terrorismo de estado, hiperinflación documentada |

**Pitch en una línea:** *El primer simulador político donde podés revivir Cuba 1959,
Chile 1973 o Argentina 1976 con un motor de 200+ variables interconectadas y asesores
LLM opcionales.*

---

## 3. Público objetivo

### Primario
- 25-45 años, lectores de historia política, fans de Paradox / Positech.
- Latinoamérica + España (mercado natural por escenarios).
- Hispanohablantes con interés en política comparada.

### Secundario
- Estudiantes y docentes de ciencias políticas / historia (uso educativo).
- Streamers de strategy (Quill18, Sips, Many a True Nerd y equivalentes en español).
- Modders que valoran motores extensibles vía YAML.

### Tamaño estimado de mercado
Democracy 4 vendió ~500k copias en su primera ventana. Homo Politicus apunta a 30-50k
copias en Early Access español + EN, como objetivo realista para juego solo-dev indie.

---

## 4. Estado actual (lo que ya está consolidado)

### Motor (`homo-politicus`, junio 2026)
- 11.435 líneas C++17, 187 commits, 62 tests pasando, cero warnings.
- 15 módulos extraídos: Game, DecisionSystem, GameOverChecker, Persistence,
  AchievementTracker, LeaderLoader, ScenarioLoader, EventLoader, ExpressionEvaluator,
  FeedbackBuilder, MiniYaml, Advisor, EventManager, Country, TestFramework.
- Save/load funcional (subset crítico).
- 9 condiciones de fin de partida.
- 5 decisiones reactivas que bloquean el turno.
- 100+ comandos de jugador.
- Sistema de logros (15 implementados).
- 5 asesores tipo gabinete (canned, listos para wiring LLM).

### Contenido (`homo-politicus-game`, junio 2026)
- 8 escenarios históricos completos: Argentina 1976, Cuba 1959, Venezuela 2013,
  Chile 1973, Singapur 1965, USA 1929, Alemania 1933, Sudáfrica 1994.
- 15 líderes históricos: Perón, Thatcher, Mandela, Allende, Pinochet, Lee Kuan Yew,
  Castro, Chávez, Merkel, Lula, FDR, Hitler, De Klerk, Gorbachov, Bukele.
- 30 eventos con flavor text en 4 archivos temáticos.
- Locales ES/EN preparados.
- 5 escenarios + 5 líderes con investigación bibliográfica.

### Documentación de diseño
- Tutorial de 5 misiones (guion completo).
- 5 modos de juego definidos.
- Tabla de balance con 30+ constantes tuneables.
- 40 logros catalogados.
- Formato de feedback estándar.
- 11 specs detallados (Fases 5-15).

### Lo que esto significa
**El 60% del trabajo de un producto vendible ya está hecho.** Lo que falta es la capa
de presentación (UI gráfica, audio, polish), distribución (Steam, marketing) y QA real
(playtesting con humanos).

---

## 5. Compromisos del producto

Decisiones tomadas y no negociables salvo evidencia fuerte en contra:

1. **C++17** como lenguaje del motor. Sin migración a otro stack.
2. **Single-player primero.** Multiplayer queda para post-v1.0 si la base lo pide.
3. **Turn-based.** Sin tiempo real, sin pausa táctica.
4. **2D.** Sin 3D, sin motion capture. Estética cercana a Democracy 4 / Suzerain.
5. **Histórico + sandbox.** Sin futurismo ni fantasía.
6. **Modding desde el día uno** (YAML extensible).
7. **Open source del motor + closed source del juego.** Engine bajo MIT, juego comercial.
8. **Soporte oficial: español (canónico) + inglés.** Otros idiomas por comunidad o DLC.

---

## 6. Roadmap a lanzamiento

### Hito 1 — Closed Alpha (T+2 meses)
**Versión:** 0.5 terminal-jugable.
**Objetivo:** validar propuesta con 10-20 testers de confianza antes de invertir en UI.
**Entregables:**
- Loaders YAML completos (eventos + locales).
- Tutorial implementado scripted (no solo guion).
- 25/40 logros funcionales.
- Save/load extendido a 100+ variables.
- Refactor del legacy `if/else` completado.
- README de "cómo jugar en terminal" en repo público.

**Métrica de éxito:** ≥7/10 testers completan una partida completa y describen al menos
una decisión moralmente difícil.

### Hito 2 — Open Beta UI (T+5 meses)
**Versión:** 0.7 con UI gráfica básica.
**Objetivo:** primera versión jugable por usuarios sin terminal.
**Entregables:**
- UI con SFML 2.6 + Dear ImGui.
- 5 paneles: Dashboard, Mapa, Acción, Decisiones, Log.
- Gráficos temporales (popularidad, GDP, presiones).
- Modal de decisión bloqueante.
- Página Steam abierta para wishlist.
- 10 escenarios totales (3 más).
- 20 líderes totales (5 más).

**Métrica de éxito:** ≥5.000 wishlists en Steam, ≥75% retention al día 7 en beta privada
con 100 jugadores.

### Hito 3 — Early Access (T+7 meses)
**Versión:** 1.0-ea.
**Objetivo:** lanzamiento pagado en Steam.
**Entregables:**
- 30 logros funcionales.
- AI advisor con LLM real (Claude API) como feature opcional.
- Localización ES + EN testeada por hablantes nativos.
- 3 modos de juego: Sandbox, Misiones, Histórico.
- Trailer (60-90s), 10 screenshots, página Steam pulida.
- Sistema de telemetría opt-in para análisis de balance.

**Métrica de éxito:** 5.000 copias vendidas en primer mes, ≥75% reviews positivas.

### Hito 4 — Lanzamiento completo (T+12 meses)
**Versión:** 1.0.
**Objetivo:** Steam Release con todas las features de spec.
**Entregables:**
- 40/40 logros.
- Steam Workshop para mods.
- Modos Sucesor + Iron Man.
- Cloud saves.
- Steam achievements integrados.
- 50+ horas de gameplay sin contenido duplicado.

**Métrica de éxito:** 30-50.000 copias acumuladas, 80%+ reviews positivas, nominación
a algún premio indie.

### Post-launch (T+12 a T+24 meses)
- DLC "Latinoamérica" (5 escenarios nuevos): $4.99.
- DLC "Guerra Fría" (5 escenarios + 10 líderes): $4.99.
- Localización a portugués brasileño, francés, alemán por comunidad.
- Mobile port considerado (no decidido).

---

## 7. Estrategia de precio

| Etapa | Precio USD | Justificación |
|---|---|---|
| Closed Alpha | Gratis | Validación de propuesta |
| Open Beta | Gratis (wishlist) | Building hype |
| Early Access | $14.99 | Premium-indie, comparable a Suzerain ($19.99) |
| v1.0 release | $19.99 | Aumento +33% al salir de EA |
| DLCs futuros | $4.99 c/u | Paradox-style content drops |
| Bundle completo | $34.99 | Juego base + 2 DLC |

---

## 8. Plan de comunicación

### Antes de Early Access (T-90 a T+0)
- Devlog semanal en YouTube + reddit (r/IndieDev, r/strategy).
- Twitter/X con clips de eventos políticos en vivo (con permiso humorístico).
- Discord propio con canales por escenario.
- Post en Hacker News + Slashdot (engine open source es el gancho).
- Press kit a 20 outlets indie y al menos 5 streamers de strategy.

### Durante Early Access
- AMA en r/IndieDev día 1.
- Parche semanal con changelog en Spanish + English.
- Stream del autor 1x por semana mostrando desarrollo en vivo.
- Q&A monthly en YouTube.

### Lanzamiento v1.0
- Trailer profesional (60-90s).
- Coverage targeting: Rock Paper Shotgun, PC Gamer, IGN Latinoamérica.
- Steamfest participación (si calza con timing).

---

## 9. Riesgos y mitigaciones

| Riesgo | Probabilidad | Impacto | Mitigación |
|---|---|---|---|
| UI llega tarde / mal | Alta | Crítico | Contratar UI freelance si solo no funciona en 3 meses |
| Censura de escenarios (Hitler, etc.) | Media | Medio | Documentar mecánicas educativas, Hitler solo como antagonista |
| Crítica política partidaria | Alta | Bajo-medio | Tono académico, sin endorsement, mostrar costos de cada ideología |
| LLM advisor caro a escala | Media | Medio | Tier free de 5 consultas, BYO API key como opción |
| Burnout solo-dev | Alta | Crítico | Closed alpha temprana para feedback emocional, no perfectismo |
| Competencia (otro indie similar) | Baja | Medio | Diferenciación por escenarios LATAM exclusivos |

---

## 10. KPIs y métricas

### Validación de producto (pre-EA)
- ≥70% de testers completa una partida.
- ≥50% inicia una segunda partida sin solicitarlo.
- Net Promoter Score ≥40 entre testers.

### Early Access
- Wishlists pre-launch: ≥5.000.
- Conversión wishlist → venta: ≥10%.
- Reviews positivas: ≥75%.
- Tiempo promedio jugado: ≥8 horas.
- Tickets de soporte críticos por semana: ≤5.

### v1.0
- Copias vendidas año 1: 30.000-50.000.
- Reviews positivas: ≥80%.
- Mods publicados en Workshop: ≥100.
- Discord miembros activos: ≥1.000.

---

## 11. Decisiones pendientes (a tomar antes de Hito 1)

1. ¿Engine UI: **SFML+ImGui** (mi recomendación) o **Godot**?
2. ¿Open source del motor: liberar **ahora** (gana comunidad temprana) o **al lanzamiento** (mantiene ventaja)?
3. ¿AI advisor con LLM real: integrar a $X/mes o **bring-your-own-key** sin costo para el dev?
4. ¿LLC / persona jurídica antes de cobrar?
5. ¿Plataforma exclusiva (Steam) o también **itch.io + GOG** desde día uno?

---

## 12. Filosofía del proyecto

Tres principios no negociables:

### 12.1 Honestidad histórica
Los escenarios reales se modelan sin embellecimiento. Argentina 1976 incluye los
desaparecidos. Cuba 1959 incluye el éxodo. Singapur 1965 incluye los azotes. Esto no
es propaganda — es simulación con costo. Cada decisión real tuvo precio real.

### 12.2 Profundidad sobre accesibilidad
No vamos a "simplificar para que entiendan más jugadores". El público objetivo busca
profundidad. La accesibilidad se logra con buen tutorial y tooltips, no diluyendo el
modelo.

### 12.3 Modding como ciudadanía
La comunidad va a hacer mejores escenarios que el dev solo. El motor se diseña para
extenderse vía YAML desde el día uno. Steam Workshop es prioridad post-launch.

---

## 13. Compromiso del autor

Este proyecto deja de ser "ejercicio de aprendizaje" y pasa a ser **producto comercial
en desarrollo activo**. Compromiso del autor:

- Mínimo 20 horas/semana hasta Hito 1.
- Mínimo 30 horas/semana de Hito 1 a Early Access.
- Comunicación pública semanal del progreso.
- Apertura a refactor mayor si playtesting lo pide.
- No-shipping si el producto no cumple métricas de validación, antes que lanzar algo
  que destruya reputación.

**Si en Hito 1 las métricas de validación no se cumplen**, el proyecto pivota:
- Opción A: continuar como freeware / open source educacional.
- Opción B: pivotar a herramienta para profes de ciencias políticas.
- Opción C: archivo digno con portfolio aprendido.

No hay deshonor en cualquiera de las tres si los datos lo justifican.

---

## 14. Llamado a colaboración

El proyecto sigue siendo principalmente solo-dev, pero está abierto a:
- **Artistas 2D** dispuestos a trabajar a riesgo por revenue share.
- **Compositor** para 5 tracks de música ambiente.
- **Historiadores** para verificar escenarios y sugerir nuevos.
- **Traductores nativos** ES/EN/PT/FR para localización profesional.
- **Beta testers** rigurosos con foco en balance.

Contacto: bmichelcano@gmail.com + Discord (a abrir antes de Hito 1).

---

## Apéndice: De prompt de aprendizaje a producto

Este documento reemplazó al `inicial_prompt.md` original de febrero 2026, escrito como
ejercicio de exploración por un estudiante de C++ con dos semanas de estudio. Ese
documento sigue siendo válido como **memoria histórica del origen** y permanece
accesible en el historial git (commit anterior a esta reescritura).

La transición se justifica porque el proyecto cumplió y excedió la ambición original:
los 5 sistemas con sus variables están implementados, el motor simula causalmente,
los escenarios funcionan, los tests pasan. El siguiente capítulo ya no es de
aprendizaje técnico, es de producto.
