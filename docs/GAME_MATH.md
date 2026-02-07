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
