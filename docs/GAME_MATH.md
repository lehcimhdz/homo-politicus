# Modelo Matemático de Homo Politicus

Este documento detalla las fórmulas exactas que rigen la simulación.

## 1. Demografía (Population)
La población cambia anualmente basada en tasas fijas de natalidad y mortalidad.

$$
P_{t+1} = P_t + (P_t \times T_{natalidad}) - (P_t \times T_{mortalidad})
$$

### Tasa de Mortalidad Dinámica
La gente no muere a un ritmo fijo. Depende de su salud y estilo de vida.

$$
T_{mortalidad} = 0.008 + (Obesidad \times 0.02) - (CoberturaSalud \times 0.005)
$$

*   **Obesidad (`obesity_rate`)**: Crece **0.5%** anual si la inflación es baja (<3%). El "bienestar" engorda.
*   **Cobertura Salud (`health_coverage`)**: Reduce la mortalidad. ¡Invierte en hospitales!

### Depreciación del Sistema de Salud
Los hospitales no son eternos.
*   **Decadencia Natural**: -2% anual (Mantenimiento).
*   **Decadencia por Crisis**: Si PIB decrece, -2% EXTRA (-4% total).
*   **Estrategia**: Debes usar `invest_health` regularmente solo para mantener el sistema, o dejarlo caer para ahorrar dinero (a costa de muertes).

---

## 2. Economía (GDP)
El Producto Interno Bruto (PIB) crece exponencialmente basado en la tasa de crecimiento.

$$
GDP_{t+1} = GDP_t + (GDP_t \times T_{crecimiento})
$$

*   **PIB Inicial**: $500,000,000
*   **Tasa Crecimiento Base (`growth_rate`)**: 2.0% (`0.02`)
*   **Efecto Inversión**: Cada vez que inviertes en **Infraestructura** (`invest_infra`), la tasa de crecimiento aumenta permanentemente en **0.1%** (`0.001`).

---

## 3. Política (Popularidad)
La popularidad es un valor entre 0.0 (0%) y 1.0 (100%). Determina si ganas las elecciones.

### Factores Pasivos
El pueblo reacciona automáticamente a las condiciones económicas:
*   **Inflación Alta**: Si `inflation > 5%` $\rightarrow$ Popularidad **-2%** (`-0.02`).
*   **Desempleo Alto**: Si `unemployment > 10%` $\rightarrow$ Popularidad **-3%** (`-0.03`).

### Factores Activos (Inversiones y Fiscalidad)
| Acción | Comando | Coste (PIB) | Efecto Popularidad | Otros Efectos |
| :--- | :--- | :--- | :--- | :--- |
| **Subir Impuestos** | `tax+` | N/A | **-5%** (`-0.05`) | Recaudación +10%, Inflación +1% |
| **Bajar Impuestos** | `tax-` | N/A | **+3%** (`+0.03`) | Recaudación -10% |
| **Salud** | `invest_health` | $10M | **+2%** (`+0.02`) | Cobertura Salud +5% |
| **Seguridad** | `invest_security`| $10M | **+1%** (`+0.01`) | Homicidios -1.0/100k |

---

## 4. Probabilidad y Eventos (Caos)
Cada año, se genera un número aleatorio $R$ entre 0 y 99.

| Rango ($R$) | Evento | Probabilidad | Efectos Matemáticos |
| :--- | :--- | :--- | :--- |
| **0 - 1** | 🐼 **Pandemia** | ~2% | $Población \times 0.95$ (-5%)<br>$GDP \times 0.98$ (-2%) |
| **2 - 19** | 🤖 **Tecnología**| ~18% | $GDP \times 1.05$ (+5%) |
| **20 - 29** | 💰 **Corrupción**| 10% | $Popularidad - 0.10$ (-10%) |
| **Indep.** | 🥦 **Comida** | 5% | $Inflation + 0.01$ (+1%)<br>$Popularidad - 0.03$ (-3%) |
| **Indep.** | ☢️ **Nuclear** | 0.5% | $GDP \times 0.8$ (-20%)<br>$Radiación = 1.0$ (Efecto Permanente) |
| **Indep.** | 🚑 **Emergencia (MCI)** | 1% | Test de Estrés: `Heridos` vs `Hospitales`<br>Si Capacidad < Heridos $\rightarrow$ Popularidad -5% |
| **30 - 99** | (Ninguno) | 70% | Sin cambios. |

### Efectos de Radiación y Ciencia
*   **Requisito Nuclear**: Solo tienes reactores si `Poder Industrial > 0.6`. Países agrarios están a salvo.
*   **Mitigación Científica**: Tu `educational_quality` mejora la seguridad.
    *   Educación Baja: Riesgo 0.5% (Chernobyl).
    *   Educación Alta: Riesgo 0.25% (Fukushima/Moderno).
*   **Accidente**: `food_radiation_prob = 1.0`. Efectos permanentes.

### Incidentes de Múltiples Víctimas (MCI)
Eventos puntuales (incendios, derrumbes) que ponen a prueba tu capacidad instalada.
*   **Heridos**: 500 - 2000 personas.
*   **Capacidad**: `Hospitales * 15`.
*   **Gestión**: Tienes 100 hospitales (Capacidad 1500). Si hay 2000 heridos, mueren 500 personas por falta de atención.

### Accidentes Industriales (Hidrocarburos/Químicos)
El precio del progreso.
*   **Probabilidad**: Calculada dinámicamente: $(DependenciaCombustibles \times 20) + (PoderIndustrial \times 10)$. Rango típico: 15-30%.
*   **Tipos**: Explosión de Oleoducto, Incendio en Refinería, Fuga Química.
*   **Efectos**:
    *   **Humanos**: Cientos de quemados graves ($10\%$ mortalidad inmediata).
    *   **Económicos**: -$50M (Infraestructura dañada).
    *   **Ambientales**: +100 CO2 (Contaminación).

### Colapso de Transporte
Cuando los puentes caen por falta de pintura.
*   **Probabilidad**: Inversa a la calidad de tus carreteras. Si `road_connectivity` es baja (50%), el riesgo sube al 5-6%.
*   **Efectos**:
    *   **Logística**: `road_connectivity` baja otro 5% (Círculo vicioso).
    *   **Víctimas**: 50-150 muertos.
    *   **Costo**: Frena el crecimiento del PIB (menos carreteras = menos comercio).

### Accidente Aéreo
Tragedia Nacional.
*   **Probabilidad**: Muy baja (`0.1%` o 1 en 1000). Variable: `aviation_accident_prob`.
*   **Efectos**:
    *   **Psicológico**: La popularidad cae 5% (Luto Nacional) aunque mueran pocas personas comparado con otros eventos.
    *   **Víctimas**: ~200-300.
    *   **Diferencia**: No afecta la economía tanto como un puente, pero afecta mucho la moral.

---

## 4. Educación y Sociedad
El saber es poder... y peligro.

### Bonus Económico (Tech Boom)
Si `literacy_rate > 0.90` y `educational_quality > 0.7`:
$$ Crecimiento_{Real} = Crecimiento_{Base} + 0.015 $$
*   Un bonus del +1.5% al PIB anual.

### La Trampa de la Pobreza (Matrícula)
La alfabetización tiene un límite físico: `primary_enrollment`.
*   **Erosión Económica**: Si `unemployment > 10%`, la matrícula primaria cae (Trabajo infantil).
*   **Impacto**: Si `literacy > enrollment`, la alfabetización decae automáticamente.
*   **Solución**: `invest_education` ahora también sube la matrícula (Transporte escolar/Becas).

### La Paradoja de la Educación (Oferta y Demanda)
No basta con educar; hay que dar empleo.
*   **Oferta Cualificada**: Promedio de Educación Secundaria y Universitaria.
*   **Demanda Laboral**: $Tech \times 1.2 + Financiero \times 1.0 + Industria \times 0.5$.
*   **Saturación**: Si `Oferta > Demanda`:
    *   **Desempleo Estructural**: Aumenta aunque la economía crezca (sobran abogados, faltan soldadores).
    *   **Estancamiento Salarial**: El salario mínimo cae.
    *   **Fuga de Cerebros (`brain_drain`)**: Los mejores se van. Si pasa del 40%, el PIB sufre.

    *   **Estancamiento Salarial**: El salario mínimo cae.
    *   **Fuga de Cerebros (`brain_drain`)**: Los mejores se van. Si pasa del 40%, el PIB sufre.

### Presupuesto de Investigación (I+D)
La ciencia es un lujo de países estables.
*   **Variable**: `research_spending_gdp`. Inicia en 0.5%.
*   **Dinámica (Maslow)**:
    *   **Prosperidad** (`Desempleo < 8%` Y `Inflación < 5%`): El gobierno invierte más (+0.1% anual) hasta llegar al 4%.
    *   **Crisis**: Si hay problemas, se corta el presupuesto (-0.2% anual). "La ciencia no se come".
*   **Efecto**: `Presupuesto * CalidadEducativa` -> Aumenta `tech_power`.

    *   **Crisis**: Si hay problemas, se corta el presupuesto (-0.2% anual). "La ciencia no se come".
*   **Efecto**: `Presupuesto * CalidadEducativa` -> Aumenta `tech_power`.

    *   **Efecto**: `Presupuesto * CalidadEducativa` -> Aumenta `tech_power`.

### Sindicatos Dinámicos (`union_strength`)
La fuerza laboral ya no es estática.
*   **Crece**: Si hay **Pleno Empleo** (Desempleo < 6%) o **Alta Inflación** (Necesidad de defensa).
*   **Decrece**: Si hay **Desempleo Masivo** (> 12%) o **Alta Informalidad** (Imposible organizar).

### Huelga General: Multicausal
Un paro nacional ya no es solo por dinero. Se activa por 3 factores acumulativos:
1.  **Económico**: Pérdida de poder adquisitivo (Inflación > Salarios).
2.  **Moral**: Corrupción alta (> 30%). "Huelga contra la casta".
3.  **Político**: Popularidad baja (< 25%). "Huelga para tumbar al gobierno".

### El Colapso de las Pensiones (`pension_sustainability`)

    *   **Efecto**: -3% PIB inmediato y subida forzosa de salarios (+5%).

### El Colapso de las Pensiones (`pension_sustainability`)
El sistema financiero más grande del país.
*   **Envejecimiento**: `aging_index` sube cada año (+0.2%), aumentando el gasto. (Japón Scenario).
*   **Ingresos**: Dependen de los trabajadores formales (`1 - Desempleo - Informalidad`).
*   **Crisis**:
    *   Si hay **Recesión**, los ingresos caen de golpe.
    *   Si el fondo se vacía (Sostenibilidad < 10%):
        *   **Bailout**: El gobierno imprime dinero para pagar.
        *   **Efecto**: Inflación +5% inmediata y pérdida masiva de popularidad.

        *   **Bailout**: El gobierno imprime dinero para pagar.
        *   **Efecto**: Inflación +5% inmediata y pérdida masiva de popularidad.

### Política Salarial (`wage+`, `wage-`)
El gobierno decide el salario mínimo, pero el mercado juzga.
*   **Target (Salario Digno)**: Se calcula como `PIB per Cápita * 0.4`.
*   **Zona Baja (< 80% Target)**:
    *   **Pobreza**: Aumenta.
    *   **Crecimiento**: Se frena (bajo consumo).
*   **Zona Alta (> 120% Target)**:
    *   **Inflación**: Se dispara (Espiral Precios-Salarios).
    *   **Informalidad**: Las empresas contratan en negro para no pagar.
    *   **Desempleo**: Aumenta.
*   *Comando*: Usa `wage+` con cuidado. Ganarás popularidad hoy, pero inflación mañana.

    *   **Desempleo**: Aumenta.
*   *Comando*: Usa `wage+` con cuidado. Ganarás popularidad hoy, pero inflación mañana.

### El Ciclo de la Miseria (Pobreza y Crimen)
`poverty_rate` ya no es estático.
1.  **Causas**:
    *   **Desempleo**: Alimenta la pobreza directamente (+0.5% por cada 1% de paro).
    *   **Inflación**: Erosiona los ahorros (+1% si Inflación > 10%).
    *   **Mitigación**: La Educación reduce la pobreza a largo plazo (Movilidad Social).
2.  **Consecuencias**:
    *   **Crimen**: La desesperación aumenta la `homicide_rate`.
        *   Fórmula: $Pobreza \times 2 + Desempleo \times 1 + Corrupción \times 0.5$.
    *   **Radicalización**: Si `pobreza > 30%`, la gente se polariza (+2% anual).

    *   **Radicalización**: Si `pobreza > 30%`, la gente se polariza (+2% anual).

### Transición Demográfica (`birth_rate`)
La demografía es el destino.
*   **Fórmula**: $Base (3.5\%) - Educación - Urbanización + Pobreza - Desempleo$.
*   **Efectos**:
    *   **Educación**: La mujer educada tiene menos hijos (-1.5%).
    *   **Urbanización**: La ciudad es cara para criar hijos (-1.0%).
    *   **Pobreza**: Las familias pobres tienen más hijos por supervivencia (+0.5%).
    *   **Incertidumbre**: El desempleo retrasa la natalidad (-0.2%).
*   **Consecuencia**: Si tienes éxito (País Rico/Educado), tu natalidad caerá, acelerando el **Envejecimiento** y la **Crisis de Pensiones**.

*   **Consecuencia**: Si tienes éxito (País Rico/Educado), tu natalidad caerá, acelerando el **Envejecimiento** y la **Crisis de Pensiones**.

### Mortalidad Dinámica (`death_rate`)
La muerte nos iguala a todos, pero las variables deciden cuándo.
*   **Base**: 0.5% (Biológica).
*   **Aceleradores**:
    *   **Envejecimiento**: +1.5% si el país es viejo.
    *   **Pobreza**: +0.5% (Falta de recursos).
    *   **Obesidad**: +0.5% (Enfermedades crónicas).
    *   **Contaminación**: +0.2% si CO2 > 5000.
*   **Freno**:
    *   **Cobertura de Salud**: -0.5% (Hospitales y medicinas).

*   **Freno**:
    *   **Cobertura de Salud**: -0.5% (Hospitales y medicinas).

### La Gran Migración (`urban_population_ratio`)
La gente se mueve a donde está el dinero.
*   **Atracción (Pull)**:
    *   Industria + Tech + Finanzas = Empleos Urbanos.
*   **Expulsión (Push)**:
    *   Alta Productividad Agrícola = Máquinas reemplazan campesinos.
    *   Pobreza Rural = Huida a la ciudad.
*   **Resistencia**:
    *   La gente mayor (`aging_index`) no migra.

*   **Estabilidad**: Los países más estables son los homogéneos (o muy rurales o muy urbanos).

### Densidad de Población y Territorio
El espacio es finito.
*   **Variable**: `land_area` (200,000 km² fijos).
*   **Densidad (`population_density`)**: Se recalcula cada año (`Población / Área`).
*   **Efectos**:
    *   **Innovación**: Si `> 100 hab/km²`, las ideas fluyen mejor (+Innovación).
    *   **Epidemias**: Si `> 200 hab/km²`, los virus se propagan más rápido (+Riesgo Epidemia).

    *   **Epidemias**: Si `> 200 hab/km²`, los virus se propagan más rápido (+Riesgo Epidemia).

### Migración (`net_migration_rate`)
La gente vota con los pies.
*   **Atracción (Pull)**:
    *   **Economía**: PIB per cápita > Promedio Global ($10k).
    *   **Libertad**: Libertad de Prensa/Culto y Derechos Civiles.
    *   **Seguridad**: Baja tasa de homicidios.
*   **Expulsión (Push)**:
    *   **Represión**: Tortura, Censura.
    *   **Pobreza**: Falta de oportunidades.
*   **Efecto (Brain Drain)**:
    *   Si la gente se va (`rate < -0.5%`) y tienes buena educación universitaria, pierdes Innovación (se van los listos).

*   **Efecto (Brain Drain)**:
    *   Si la gente se va (`rate < -0.5%`) y tienes buena educación universitaria, pierdes Innovación (se van los listos).

### Religión y Poder (`clerical_political_influence`)
Fe versus Razón.
*   **Tradición (Increasers)**:
    *   Población Rural y Pobreza alimentan la influencia religiosa.
*   **Secularización (Decreasers)**:
    *   Educación y Vida Urbana reducen la influencia de la iglesia.
*   **Efectos**:
    *   **Natalidad**: La religión frena la caída de la natalidad (+Familia).
    *   **Derechos**: Alta influencia religiosa erosiona la `minority_protection`.
    *   **Estabilidad**: (Implícito) Ayuda a mantener el orden en sociedades pobres.

    *   **Derechos**: Alta influencia religiosa erosiona la `minority_protection`.
    *   **Estabilidad**: (Implícito) Ayuda a mantener el orden en sociedades pobres.

### Tensión Religiosa (`interreligious_tension`)
El peligro del fanatismo.
*   **Radicalismo (`radicalism_prob`)**:
    *   Se alimenta de la Pobreza, la Represión y la Influencia Clerical excesiva.
    *   Se combate con Educación y Libertad de Culto (Tolerancia).
*   **Tensión**:
    *   Surge cuando hay Radicalismo + Diversidad (`freedom_of_worship`).
    *   **Seguridad**: Una buena inteligencia (`attack_detection`) puede mitigar la violencia.
*   **Efecto**: Aumenta la **Polarización** y el riesgo de conflicto civil.

*   **Efecto**: Aumenta la **Polarización** y el riesgo de conflicto civil.

### Radicalismo y Terrorismo (`radicalism_prob`)
El precio del extremismo.
*   **Umbral de Peligro**: Si `radicalism > 15%`, comienzan los atentados.
*   **Ataques Terroristas**:
    *   **Probabilidad**: `(Radicalismo - 0.15) * (1 - Inteligencia)`.
    *   **Consecuencias**: Muertes, Caída del PIB (-0.5%), Pánico (Polarización +2%).
*   **Defensa**:
    *   Invertir en `attack_detection_prob` (Espionaje/Policía) reduce drásticamente el riesgo real, incluso si el radicalismo es alto.

    *   Invertir en `attack_detection_prob` (Espionaje/Policía) reduce drásticamente el riesgo real, incluso si el radicalismo es alto.

### Políticas de Libertad de Culto
Tú decides el nivel de tolerancia.
*   `worship+` (Estado Laico/Liberal):
    *   **Beneficios**: Aumenta Felicidad (`mental_health`) y reduce el poder clerical.
    *   **Costos**: Aumenta temporalmente la `interreligious_tension` (choque de ideas) y el riesgo de terrorismo si hay radicalismo previo.
*   `worship-` (Estado Teocrático/Control):
    *   **Beneficios**: Reduce la `interreligious_tension` (Homogeneidad forzada).
    *   **Costos**: Aumenta el Radicalismo latente (resistencia) y golpea la Felicidad.

    *   **Costos**: Aumenta el Radicalismo latente (resistencia) y golpea la Felicidad.

### Derechos Humanos y Tortura (`torture_index`)
El fin justifica los medios?
*   `torture+` (Mano Dura):
    *   **Beneficios**: La Inteligencia (`attack_detection`) sube bruscamente (+0.15). Atrapas a los terroristas.
    *   **Costos**: El Radicalismo aumenta (+0.05). Tu prestigio internacional (`un_score`) se desploma.
*   `torture-` (Estado de Derecho):
    *   **Beneficios**: Recuperas prestigio en la ONU y reduces el reclutamiento terrorista.
    *   **Costos**: Pierdes capacidad de inteligencia inmediata (-0.10).

    *   **Beneficios**: Recuperas prestigio en la ONU y reduces el reclutamiento terrorista.
    *   **Costos**: Pierdes capacidad de inteligencia inmediata (-0.10).

### Desapariciones Forzadas (`forced_disappearances`)
Sembrar el terror.
*   `disappear+` (Noche y Niebla):
    *   **Objetivo**: Silenciar las calles. Si tienes `mobilizations` masivas, esto las detiene en seco (Miedo absoluto).
    *   **Costo**: Destruyes tu legitimidad internacional (`un_score`) y generas un odio profundo (`radicalism` ++).
*   `disappear-` (Comisión de la Verdad):
    *   **Objetivo**: Sanar las heridas de la sociedad.
    *   **Riesgo**: La gente pierde el miedo. Al investigar el pasado, las protestas aumentarán temporalmente ("¡Juicio y Castigo!").

    *   **Riesgo**: La gente pierde el miedo. Al investigar el pasado, las protestas aumentan temporalmente ("¡Juicio y Castigo!").

### Derechos de las Minorías (`minority_protection`)
La diversidad como motor o como chivo expiatorio.
*   `minority+` (Leyes Antidiscriminación):
    *   **Beneficios**:
        *   **Innovación**: Si `protection > 0.8`, la diversidad de ideas aumenta la innovación (+0.002).
        *   **Paz Social**: Reduce el Radicalismo (-0.03) y mejora el prestigio internacional (+0.05).
    *   **Costos**:
        *   **Guerra Cultural**: Sectores conservadores reaccionan. Popularidad baja (-0.03) y Polarización sube (+0.02).
*   `minority-` (Nacionalismo Excluyente):
    *   **Beneficios**:
        *   **Popularidad Populista**: +0.04 inmediato (Culpar al "otro" funciona).
    *   **Costos**:
        *   **Fuga de Cerebros**: Si `protection < 0.3`, las minorías (a menudo comerciantes o intelectuales) huyen (+0.005 Brain Drain).
        *   **Pobreza Estructural**: Si `protection < 0.4`, la exclusión económica aumenta la pobreza (+0.005).
        *   **Paria Internacional**: El prestigio ONU cae fuerte (-0.08).

### Diplomacia y Sanciones (`un_score`)
Tu reputación en el mundo tiene un precio.

#### Comandos Diplomáticos
*   `diplomacy+` (Lobby Internacional):
    *   **Costo**: $50M (Gasto directo del PIB).
    *   **Efecto**: `un_score` +0.05. Reduce el riesgo de sanciones (-0.05).
*   `diplomacy-` (Soberanía Nacionalista):
    *   **Beneficio**: Popularidad +0.03 (Nacionalismo).
    *   **Costo**: `un_score` -0.10. Aumenta el riesgo de sanciones (+0.05).

#### Consecuencias Económicas
1.  **Sanciones Internacionales (Castigo)**:
    *   **Riesgo**: Aumenta si `un_score < 0.3`.
    *   **Efecto**: Si se activan, el PIB cae un **5%** inmediato y la inflación sube un **2%** (Bloqueo de importaciones).
2.  **Inversión Extranjera (Premio)**:
    *   **Condición**: `un_score > 0.7`.
    *   **Efecto**: El crecimiento del PIB recibe un bonus (hasta +1.5%) basado en la confianza global.

### Libertad de Expresión (`freedom_of_expression`)
La verdad os hará libres... pero infelices.
*   `press+` (Prensa Libre):
    *   **Beneficios**: La Innovación florece (+0.05) y la Corrupción baja (Efecto Watchdog).
    *   **Costos**: Los escándalos salen a la luz. Tu popularidad puede caer (-0.08) si descubren trapos sucios.
*   `press-` (Censura):
    *   **Beneficio**: Controlas la narrativa. La popularidad se mantiene estable ("Aquí no pasa nada").
    *   **Costo**: La Corrupción se dispara (Impunidad) y la Innovación muere (-0.08). Creas una olla a presión.

### Trampa de la Inestabilidad
Una población educada no tolera la corrupción.
*   Si `literacy > 0.90` Y `corruption > 0.30`:
    *   Aumenta `polarization_index`.
    *   Aumentan las protestas (`marches`).
    *   Baja la popularidad.

---

## 5. Bienestar Social y Salud Mental
El estado de ánimo de la nación es dinámico y reactivo.

### Índice de Salud Mental ($MH$)
$$ Target_{MH} = 1.0 - (Desempleo \times 2) - (Inflación \times 1.5) - (Corrupción \times 0.5) $$
*   **Drift**: El índice real se mueve 20% hacia el `Target` cada año (la gente tarda en recuperar la esperanza).

### Tasa de Suicidio
$$ Tasa_{Suicidio} = \frac{0.00014}{MH} $$
*   **Base**: 14 por 100,000 habitantes.
*   **Dinámica**: Si el $MH$ cae a 0.5 (Desesperación), la tasa de suicidios se DUPLICA. Estos muertos se suman a la mortalidad natural.

---

## 5. Elecciones Democráticas
Cada 4 años (`turnCount % 4 == 0`), se evalúa la continuidad del gobierno.

*   **Condición de Victoria**: $Popularidad > 0.50$ ($50\%$)
*   **Condición de Derrota**: $Popularidad \le 0.50$ ($50\%$)
    *   Consecuencia: `isRunning = false` (Game Over).
