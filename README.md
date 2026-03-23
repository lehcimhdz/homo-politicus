# Homo Politicus

**Homo Politicus** is a deep, realistic turn-based political and economic strategy simulator written in C++17. You take the role of a nation's President and must manage complex, interconnected systems while maintaining enough popularity to win democratic elections every 4 years.

---

## Build & Run

```bash
# CMake (recommended)
cmake . && make
./HomoPoliticus

# Makefile (clang++)
make
./HomoPoliticus
```

**Requirements**: C++17-compatible compiler (clang++ or g++), CMake 3.x.

---

## Objective & Loss Condition

- **Objective**: Stay in power for as many years as possible.
- **Elections**: Every 4 years, popularity is evaluated. If it drops to ≤50%, the game ends.
- **Advancing time**: The `next` command simulates one full year of government.

---

## Full Command Reference

### Basic Commands

| Command | Effect |
| :--- | :--- |
| `next` | Advance one year. Runs the full simulation (economy, demographics, events, elections). |
| `exit` | Quit the game. |

### Fiscal & Wage Policy

| Command | Cost | Main Effect | Secondary Effects |
| :--- | :--- | :--- | :--- |
| `tax+` | — | Tax revenue +10% | Popularity −5%, Inflation +1% |
| `tax-` | — | Tax revenue −10% | Popularity +3% |
| `wage-` | — | Minimum wage −10% | Competitiveness +, Poverty +, Popularity − |

> **Note**: There is no `wage+` command. Unions automatically index wages when inflation exceeds 5% and union strength is high.

### Social Policy & Pensions

| Command | Main Effect | Secondary Effects |
| :--- | :--- | :--- |
| `retire+` | Raise retirement age | Pension sustainability +, Popularity −−− |
| `retire-` | Lower retirement age | Pension sustainability −, Popularity +++ |

### Human Rights & Freedoms

| Command | Main Effect | Cost / Risk |
| :--- | :--- | :--- |
| `worship+` | Freedom of worship +, Happiness +, Clerical influence − | Inter-religious tension + (temporarily) |
| `worship-` | Clerical control +, Inter-religious tension − | Latent radicalism +, Happiness − |
| `torture+` | Attack detection +0.15, Intelligence + | Radicalism +0.05, UN Score −0.15 |
| `torture-` | UN Score +, Terrorist recruitment − | Immediate intelligence capacity −0.10 |
| `disappear+` | Silences protests (absolute fear) | UN Score −−−, Radicalism +++ |
| `disappear-` | Truth Commission, legitimacy + | Protests temporarily increase |
| `minority+` | Innovation +, Radicalism −0.03, UN +0.05 | Popularity −0.03, Polarization +0.02 |
| `minority-` | Populist popularity +0.04 | Brain drain +, Structural poverty +, UN −0.08 |

### Press Freedom

| Command | Main Effect | Secondary Effects |
| :--- | :--- | :--- |
| `press+` | Innovation +0.05, Corruption − (watchdog effect) | Scandal risk → Popularity can drop −0.08 |
| `press-` | Narrative control, surface stability | Corruption +, Innovation −0.08 |

### International Policy

| Command | Cost | Main Effect | Secondary Effects |
| :--- | :--- | :--- | :--- |
| `diplomacy+` | $50M | UN Score +0.05 | Sanctions risk −0.05 |
| `diplomacy-` | — | Popularity +0.03 (nationalism) | UN Score −0.10, Sanctions risk +0.05 |

### Monetary Policy

| Command | Main Effect | Secondary Effects |
| :--- | :--- | :--- |
| `interest+` | Inflation −, Private investment − | If rate > 12%: construction sector freezes. If > 20%: banking crisis |
| `interest-` | Private investment +, Credit + | Inflation may rise |
| `print+` | Reserves +1% of GDP (printed money) | Inflation +. Blocked if CB autonomy > 0.6 |
| `autonomy+` | FDI +, Inflation anchors, Credibility + | Blocks `print+`, interfering with rates costs popularity |
| `autonomy-` | Full monetary policy control | Capital flight, investment − |
| `reform_currency` | Resets inflation to 2% (austerity plan) | Popularity −25%. Only useful during hyperinflation (>50%) |

### Public Investment

| Command | Cost | Main Effect | Secondary Effects |
| :--- | :--- | :--- | :--- |
| `invest_health` | $10M | Health coverage +5% | Popularity +2%, Mortality − |
| `invest_security` | $10M | Homicides −1.0/100k | Popularity +1% |
| `invest_infra` | $50M | GDP growth +0.1% (permanent) | Improves roads and trade |
| `invest_education` | $20M | Literacy +, Enrollment + | Educational quality +. Watch for brain drain if no industry |

---

## Game Systems

### Economy (Cobb-Douglas Model)

GDP grows as a function of three pillars:
- **Labor**: Employed population × human capital (education + health)
- **Physical capital**: Infrastructure (roads, ports) + industrial power
- **Total Factor Productivity (TFP)**: Innovation + technology − corruption (as efficiency loss)

Growth is also modulated by **global economic cycles** (12-year sine wave): export-oriented economies grow faster during global booms and suffer more during recessions.

### Inflation & Wage-Price Spiral

Inflation results from accumulated forces:
1. **Monetary emission** (`print+`)
2. **Demand pull** (purchasing power > 1.1× of the living wage)
3. **Cost shock** (exchange rate instability → imported inflation)
4. **Cooling** via high interest rates

If **inflation > 5%** and **unions are strong (> 0.3)**, automatic wage indexation kicks in, creating a vicious cycle that is hard to break.

### Sovereign Debt & Credit Rating

- **Rating**: AAA → D dynamic enum updated annually based on debt/GDP, growth, inflation, and CB autonomy
- **Risk premium**: If debt > 60% of GDP, extra interest is charged (up to +20% at 120%)
- **Sovereign default**: If debt > 120% **and** reserves < 0 → GDP drops 10%, FDI stops, popularity −30%

### Central Bank

| Autonomy | Advantages | Disadvantages |
| :--- | :--- | :--- |
| **High (> 0.6)** | Multiplies FDI, anchors inflation, reduces risk premium | Blocks `print+`, interfering costs popularity |
| **Low (< 0.3)** | Full control, `print+` enabled | Capital flight, higher structural inflation |

If there is a fiscal deficit with low autonomy, the CB automatically prints the difference (**fiscal dominance**).

### Education: The Knowledge Trap

1. **Tech Boom**: If `literacy > 90%` and `quality > 70%` → +1.5% annual GDP growth.
2. **Corruption trap**: Educated population + high corruption → mass protests.
3. **Structural trap**: Education without industry → brain drain and structural unemployment.
4. **Enrollment ceiling**: Literacy cannot exceed primary enrollment. If unemployment > 10%, children drop out of school (child labor).

### Dynamic Labor Market

Unemployment is driven by four forces:
1. **Economic cycle**: Growth reduces unemployment (Okun's Law)
2. **Automation**: High technology destroys basic jobs
3. **Wage rigidity**: Strong unions protect wages but make hiring harder
4. **Inertia (drift)**: Labor markets take years to recover from a crisis

### Demographics & Migration

- **Demographic transition**: More education and urbanization → lower birth rates → aging → pension crisis
- **Net migration**: People emigrate under repression, poverty, or insecurity. Brain drain reduces innovation.
- **Diaspora & remittances**: Emigration builds a diaspora sending remittances ($2,000/person/year). If remittances exceed 5% of GDP, poverty drops directly.

### Security & Human Rights

- **Terrorism**: If radicalism > 15%, attacks begin. Intelligence (`attack_detection`) can mitigate them.
- **Torture dilemma**: Boosts attack detection short-term but increases radicalism long-term.
- **Forced disappearances**: Silence protests through fear, but destroy legitimacy and fuel latent hatred.

### Infrastructure & Maintenance

Everything degrades **5% annually**. If `maintenance_level < 50%`:
- Risk of bridge collapse (−GDP, loss of connectivity)
- Risk of fires and explosions (civilian casualties)
- Risk of Mass Casualty Incidents (MCI): if hospitals lack capacity → preventable deaths

### Random Events

| Event | Base Probability | Effects |
| :--- | :--- | :--- |
| Pandemic | 2% | Population −5%, GDP −2% |
| Tech breakthrough | ~10% | GDP +5% |
| Corruption scandal | 10% | Popularity −10% |
| Food contamination | 5% | Inflation +1%, Popularity −3% |
| Nuclear accident | 0.5% (if industry > 0.6) | GDP −20%, permanent food radiation |
| MCI (collapse/fire) | 1% | Hospital capacity test vs 500–2,000 casualties |
| Industrial accident | 15–30% dynamic | −$50M, +100 CO2, burns and deaths |
| Transport collapse | Inverse to road quality | Roads −5%, 50–150 deaths |
| Aviation crash | 0.1% | ~250 deaths, Popularity −5% |

---

## Key Tensions & Dilemmas

The game has no simple solutions. Core dilemmas:

| Decision | Immediate Benefit | Long-term Cost |
| :--- | :--- | :--- |
| Raise minimum wage | Popularity +, consumption + | Inflationary spiral, labor informality |
| Print money | Deficit covered, reserves + | Runaway inflation |
| Lower retirement age | Popularity +++ | Pension system collapses |
| Censor the press | Narrative control | Rampant corruption, dead innovation |
| Torture / disappearances | Short-term security | Radicalism, international isolation |
| High CB autonomy | Macroeconomic stability | No fiscal room to maneuver |
| Invest in education | Long-term growth | Brain drain if no industry exists |

---

## Advanced Mechanics

### Hyperinflation
If inflation > 50%, GDP collapses −10% annually and investment halts. The only exit is `reform_currency`, which costs −25% popularity.

### Banking Crisis
If the interest rate exceeds 20%, debtors default and banks collapse (−4% GDP).

### Currency Crisis
If international reserves < 0, a massive devaluation occurs (+15% inflation, −5% GDP). An autonomous central bank reduces the shock to just 2%.

### Pension System Collapse
The pension fund collapses if sustainability < 10%. The government is forced to print money for the bailout: inflation +5% and massive popularity loss.

### Instability Trap
If `literacy > 90%` **and** `corruption > 30%`, an educated population will not tolerate corruption: mass protests and popularity drop.

---

## Code Architecture

```
homo-politicus/
├── include/
│   ├── Game.hpp          # Main class: game loop, annual update
│   ├── Country.hpp       # Country data structures (185+ variables)
│   └── EventManager.hpp  # Random event system
├── src/
│   ├── main.cpp          # Entry point
│   ├── Game.cpp          # Simulation logic (~1,600 lines)
│   ├── Country.cpp       # Initialization and state display
│   └── EventManager.cpp  # Event implementation (~166 lines)
├── docs/
│   └── GAME_MATH.md      # Complete mathematical documentation
├── CMakeLists.txt
└── Makefile
```

**Country structure** (struct `Country`):

| System | Approx. Variables | Description |
| :--- | :--- | :--- |
| `WelfareSocietySystem` | ~80 | Health, education, employment, demographics, religion, rights |
| `EconomicFinancialSystem` | ~20 | GDP, inflation, debt, central bank, reserves |
| `PoliticalInstitutionalSystem` | ~20 | Popularity, corruption, polarization, protests |
| `PowerSecuritySystem` | ~20 | Security, intelligence, international relations |
| `InfrastructureFutureSystem` | ~25 | Infrastructure, technology, energy, environment |

---

*Built to simulate the real complexity of governing. Every decision has interconnected consequences.*
