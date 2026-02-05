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

## 5. Elecciones Democráticas
Cada 4 años (`turnCount % 4 == 0`), se evalúa la continuidad del gobierno.

*   **Condición de Victoria**: $Popularidad > 0.50$ ($50\%$)
*   **Condición de Derrota**: $Popularidad \le 0.50$ ($50\%$)
    *   Consecuencia: `isRunning = false` (Game Over).
