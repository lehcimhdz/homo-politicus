# Mathematical Model of Homo Politicus

This document details the exact formulas and mechanisms governing the simulation. Every year (`next`) executes all these equations in order.

---

## Table of Contents

1. [Demographics](#1-demographics)
2. [Economy & GDP](#2-economy--gdp)
3. [Inflation](#3-inflation)
4. [Central Bank & Monetary Policy](#4-central-bank--monetary-policy)
5. [Sovereign Debt & Default](#5-sovereign-debt--default)
6. [Labor Market](#6-labor-market)
7. [Education & Society](#7-education--society)
8. [Social Welfare & Mental Health](#8-social-welfare--mental-health)
9. [Security, Religion & Human Rights](#9-security-religion--human-rights)
10. [Popularity & Elections](#10-popularity--elections)
11. [Random Events](#11-random-events)

---

## 1. Demographics

### Population Dynamics

$$P_{t+1} = P_t + (P_t \times T_{birth}) - (P_t \times T_{death})$$

**Dynamic Death Rate:**

$$T_{death} = 0.005 + (Aging \times 0.015) + (Poverty \times 0.005) + (Obesity \times 0.005) + CO2_{factor} - (HealthCoverage \times 0.005)$$

Where $CO2_{factor} = +0.002$ if CO2 > 5,000.

**Birth Rate (Demographic Transition):**

$$T_{birth} = 0.035 - (Education \times 0.015) - (Urbanization \times 0.010) + (Poverty \times 0.005) - (Unemployment \times 0.002)$$

- Education and urban life reduce birth rates.
- Poverty increases them (family survival strategy).
- Long-term consequence: a prosperous nation sees birth rates fall → aging → pension crisis.

**Health System Depreciation:**
- Natural decay: −2% per year.
- During recession (GDP shrinks): −2% additional (total −4%).

### Migration & Diaspora

$$T_{migration} = f(GDP\_per\_capita, Freedom, Security, Repression, Poverty)$$

- The diaspora grows when net migration is negative.
- Annual assimilation: −2% of the existing diaspora.

**Remittances:**

$$Remittances = Diaspora \times \$2{,}000 \text{ per person/year}$$

- If international sanctions are active: 90% blocked.
- If remittances > 5% of GDP: poverty −1% per year directly.

**Population Density:**

$$Density = \frac{Population}{Fixed\_Area} \quad (Area = 200{,}000 \text{ km}^2)$$

- Density > 100 people/km² → higher innovation.
- Density > 200 people/km² → higher epidemic risk.

### Urbanization

The urban/rural ratio shifts based on:
- **Urban pull**: industry + technology + finance = city jobs.
- **Rural push**: high agricultural productivity (machines) + rural poverty.
- **Brake**: aging population migrates less.

Political polarization peaks when the urban/rural split is 50/50.

---

## 2. Economy & GDP

### Growth Formula (Cobb-Douglas Type)

$$Growth_{Real} = Base + Global_{Effect} + Labor_{Factor} + Capital_{Bonus} + TFP_{Bonus} - Tax_{Drag} - Interest_{Drag} + Consumption_{Modifier}$$

**1. Labor (Human Capital):**

$$Labor = (1 - Unemployment) \times (Literacy \times 0.4 + SecondaryEd \times 0.3 + Health \times 0.3)$$

If unemployment > 5%, an Okun's Law penalty is applied.

**2. Physical Capital:**

$$Capital = road\_connectivity + port\_capacity + industrial\_power + financial\_power$$

If Capital > 0.7 → +1% growth bonus.

**3. Total Factor Productivity (TFP):**

$$TFP = innovation\_index + tech\_power - corruption$$

Corruption directly subtracts from efficiency.

**Global Economic Cycles:**

$$Global\_Cycle = \sin\left(\frac{2\pi \times t}{12}\right)$$

- Export-oriented economies (high industry + tech) amplify the cycle: +1.5× during booms, −1.5× during recessions.
- Closed economies: grow slower but steadily.

**Tech Boom:**

$$\text{If } Literacy > 0.90 \text{ and } EducationalQuality > 0.70 \Rightarrow Growth_{Real} += 0.015$$

### Balance of Payments (International Reserves)

$$\Delta Reserves = (Exports - Imports) + (FDI - Capital\_Flight) - Debt\_Service$$

- **Exports**: depend on industry, technology, and the global cycle.
- **Imports**: depend on domestic consumption (higher purchasing power → more imports).
- **FDI**: increases if `un_score > 0.7` and CB autonomy is high.

**Currency Crisis** if Reserves < 0:
- Devaluation: inflation +15% (reduced to +2% if CB is autonomous).
- GDP −5%.
- Forced bailout (increases debt).

### Consumption Engine

$$Purchasing\_Power = \frac{MinimumWage + RemittancesPerCapita}{GDP\_per\_Capita \times 0.4}$$

| Value | Effect |
| :--- | :--- |
| < 0.8 (low) | Growth slows −1% (insufficient demand) |
| 0.8 – 1.2 (normal) | No additional effect |
| > 1.2 (high) | Growth +1%, inflationary risk |

### Laffer Curve (Taxation)

$$Effective\_Tax = \frac{Tax\_Revenue}{GDP}$$

- If `Effective_Tax > 25%`: investors flee → potential growth −0.1% per 1% of excess taxes.
- Taxes reduce disposable income → less consumption → slower economy.

---

## 3. Inflation

### Accumulated Forces

$$Inflation_{t+1} = Inflation_t + Emission + Demand_{Pull} + Cost_{Shock} - Rate_{Cooling} - CB_{Credibility}$$

| Component | Formula | Condition |
| :--- | :--- | :--- |
| **Monetary emission** | +variable | Whenever `print+` is used |
| **Demand pull** | +pressure | If purchasing power > 1.1 |
| **Exchange rate shock** | +5% (pass-through) | If exchange rate stability is low |
| **Reserve shock** | +3% | If reserves < $20M |
| **Rate cooling** | −(rate × 0.5) | Always active |
| **CB credibility** | −0.5% | If autonomy > 0.7 |

### Wage-Price Spiral

**Trigger**: inflation > 5% **and** `union_strength > 0.3`

1. Unions demand automatic wage indexation.
2. Firms raise prices to cover new wage costs (+1% extra inflation).
3. A vicious cycle forms that is hard to break without an incomes policy.

**Forced indexation:**

$$MinWage_{new} = MinWage_{current} \times (1 + inflation \times 0.8)$$

### Hyperinflation

**Threshold**: inflation > 50%.

Effects:
- GDP −10% per year.
- Private investment halts.
- Polarization surges.

**Only exit**: `reform_currency` → inflation reset to 2%, popularity −25%.

---

## 4. Central Bank & Monetary Policy

### Central Bank Autonomy

| Level | Advantages | Disadvantages |
| :--- | :--- | :--- |
| **High (> 0.6)** | FDI multiplies, inflation anchors, risk premium falls | `print+` blocked, interfering with rates costs popularity |
| **Medium (0.3–0.6)** | Moderate balance | — |
| **Low (< 0.3)** | Full monetary policy control | Capital flight, high structural inflation |

### Fiscal Dominance

If fiscal deficit **and** autonomy < 0.3:

$$Forced\_Emission = Fiscal\_Deficit$$

The CB automatically prints the deficit even without using `print+`. With high autonomy, the deficit is instead covered by new debt issuance.

### Seigniorage

`print+` emits 1% of GDP:
- Immediately credited to reserves.
- Generates proportional inflationary pressure.

### Credit Channel (Interest Rate)

$$Private\_Investment = 5\% \times (1 - 3 \times Rate)$$

| Rate | Additional Effect |
| :--- | :--- |
| > 12% | Construction sector freezes (+0.5% unemployment) |
| > 20% | Banking crisis: debtors default, banks collapse (−4% GDP) |

### Exchange Rate Stability (Dynamic)

$$Stability = CB\_Autonomy \times 0.4 + ReservesBuffer \times 0.1 + TradeSupplus \times 0.1$$

- Low stability → constant imported inflation (pass-through up to +5% annually).

---

## 5. Sovereign Debt & Default

### Credit Rating

The credit rating is a dynamic enum updated annually:

```
AAA → AA → A → BBB → BB → B → CCC → CC → C → SD → D
```

**Factors determining the rating:**
- Debt/GDP ratio (main driver)
- GDP growth rate
- Inflation
- Central bank autonomy

### Risk Premium

$$Premium = \max\left(0, (Debt/GDP - 0.60) \times 20\%\right)$$

At 120% debt/GDP, the premium reaches 12% extra over the base rate.

### Default Probability

Ranges from 1% to 100% based on fiscal health. Sovereign default triggers when:

$$Debt/GDP > 1.20 \text{ and } Reserves < 0$$

**Default consequences:**
- GDP drops −10% instantly.
- FDI completely stops.
- Popularity −30%.
- Polarization +20%.

### Debt Sustainability

Debt service is deducted from reserves each year. Debt grows when:
- There is a fiscal deficit (high autonomy → issues new debt).
- Currency crisis bailout is needed.
- Pension system bailout is needed.

---

## 6. Labor Market

### Unemployment (Target Drift)

Unemployment does not change abruptly: it drifts gradually toward a target.

**Unemployment target:**

$$Target_{unemployment} = Base - f(GDP\_growth) + f(automation) + f(wage\_rigidity)$$

| Factor | Effect |
| :--- | :--- |
| Positive GDP growth | Target unemployment ↓ (Okun's Law) |
| High `tech_power` | Destroys basic jobs ↑ |
| Strong unions | Wage rigidity → harder to hire ↑ |
| Minimum wage > 120% of target | Informality ↑, formal employment ↓ |

### Union Strength

$$union\_strength \nearrow$$ if: unemployment < 6% (full employment) or high inflation (real wage defense).

$$union\_strength \searrow$$ if: unemployment > 12% (impossible to organize) or high informality.

### Wage Policy

**Living Wage Target:**

$$Target_{wage} = GDP\_per\_Capita \times 0.40$$

| Wage vs. Target | Consequences |
| :--- | :--- |
| < 80% of target | Poverty ↑, growth slows (low consumption) |
| 80%–120% of target | Balanced zone |
| > 120% of target | Inflation ↑, informality ↑, unemployment ↑ |

### Informality & Pensions

- High informality → fewer contributors → pension system deteriorates.
- Aging (`aging_index` +0.2%/year) → more retirees → greater pressure on the fund.

**Pension system collapse** if sustainability < 10%:
- Forced bailout: inflation +5%, massive popularity loss.

### General Strike: Multi-Causal

Triggered by the accumulation of three factors:

1. **Economic**: loss of purchasing power (inflation > wages).
2. **Moral**: high corruption (> 30%): "strike against the establishment."
3. **Political**: low popularity (< 25%): "strike to bring down the government."

---

## 7. Education & Society

### Literacy & Enrollment

$$Literacy_{max} = primary\_enrollment$$

- If `unemployment > 10%`, primary enrollment drops (child labor).
- If `literacy > enrollment`: literacy automatically decays (generational degradation).

### The Education Paradox

$$Qualified\_Supply = \frac{SecondaryEd + UniversityEd}{2}$$

$$Labor\_Demand = Tech \times 1.2 + Finance \times 1.0 + Industry \times 0.5$$

If `Supply > Demand`:
- Structural unemployment (surplus of professionals, shortage of tradespeople).
- Wage stagnation.
- `brain_drain` increases: if it exceeds 40%, GDP suffers.

### R&D Budget (Maslow Logic)

| Condition | Effect on research budget |
| :--- | :--- |
| Prosperity: unemployment < 8% and inflation < 5% | +0.1% per year (up to 4% of GDP max) |
| Crisis (either condition fails) | −0.2% per year ("science is a luxury") |

$$tech\_power_{t+1} = tech\_power_t + (research\_spending \times educational\_quality)$$

### Instability Trap

$$\text{If } Literacy > 0.90 \text{ and } Corruption > 0.30 \Rightarrow Polarization\uparrow, Protests\uparrow, Popularity\downarrow$$

An educated population will not tolerate corruption.

### Religion & Clerical Influence

- **Increases** with: rural population, poverty.
- **Decreases** with: education, urbanization.
- **Effects**: slows the birth rate decline (+family), erodes `minority_protection` when very high.

### Radicalism

$$radicalism_{target} = f(Poverty, Repression, ClericalInfluence) - f(Education, FreedomOfWorship)$$

**Terrorism threshold**: if radicalism > 15%:

$$P_{attack} = (Radicalism - 0.15) \times (1 - attack\_detection\_prob)$$

---

## 8. Social Welfare & Mental Health

### Mental Health Index

$$Target_{MH} = 1.0 - (Unemployment \times 2) - (Inflation \times 1.5) - (Corruption \times 0.5)$$

The actual index drifts 20% toward the `Target` each year (psychological recovery is gradual):

$$MH_{t+1} = MH_t + 0.20 \times (Target_{MH} - MH_t)$$

### Suicide Rate

$$T_{suicide} = \frac{0.00014}{MH}$$

- Historical baseline: 14 per 100,000 inhabitants.
- If MH falls to 0.5 (collective despair): the rate **doubles**.
- Suicide deaths are added to natural mortality.

### The Misery Cycle (Poverty & Crime)

**Causes of poverty:**
- Unemployment: +0.5% for every 1% of unemployment.
- Inflation > 10%: erodes savings (+1% poverty).
- Education reduces poverty long-term (social mobility).
- Remittances > 5% of GDP: poverty −1% per year.

**Consequences of poverty:**

$$homicide\_rate = Poverty \times 2 + Unemployment \times 1 + Corruption \times 0.5$$

- If poverty > 30%: polarization +2% per year.

### Obesity

$$obesity\_rate \nearrow \text{ if } Inflation < 3\%$$

Prosperity makes people overweight: +0.5% per year during low-inflation periods (affordable processed food).

---

## 9. Security, Religion & Human Rights

### Torture & Intelligence

| Command | Immediate Effect |
| :--- | :--- |
| `torture+` | `attack_detection` +0.15, radicalism +0.05, UN −0.15 |
| `torture-` | `attack_detection` −0.10, UN +, radicalism − |

Core dilemma: torture works short-term but creates more radicals long-term.

### Forced Disappearances

| Command | Effect |
| :--- | :--- |
| `disappear+` | Stops mobilizations through fear; UN −−−, radicalism +++ |
| `disappear-` | Truth Commission; people lose fear → temporary protests |

### Press Freedom

| Command | Innovation | Corruption | Popularity |
| :--- | :--- | :--- | :--- |
| `press+` | +0.05 | − (watchdog) | Scandal risk −0.08 |
| `press-` | −0.08 | + (impunity) | Stable short-term |

### Minority Rights

| Command | Innovation | Radicalism | UN | Popularity | Brain Drain |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `minority+` | +0.002 (if >0.8) | −0.03 | +0.05 | −0.03 | — |
| `minority-` | — | — | −0.08 | +0.04 | +0.005 (if <0.3) |

### Diplomacy & Sanctions

$$\text{If } un\_score < 0.3 \Rightarrow P_{sanctions} \text{ grows annually}$$

**Effects of active sanctions:**
- GDP −5% immediately.
- Inflation +2% (import blockade).
- Remittances blocked at 90%.

**High reputation reward:**

$$\text{If } un\_score > 0.7 \Rightarrow GDP\_Growth += FDI\_bonus \text{ (up to +1.5\%)}$$

---

## 10. Popularity & Elections

### Passive Factors (Automatic)

| Condition | Penalty |
| :--- | :--- |
| Inflation > 5% | −2% (`−0.02`) per year |
| Unemployment > 10% | −3% (`−0.03`) per year |

### Active Factors (Commands)

| Command | Δ Popularity |
| :--- | :--- |
| `tax+` | −5% |
| `tax-` | +3% |
| `invest_health` | +2% |
| `invest_security` | +1% |
| `invest_infra` | +1% |
| `invest_education` | +1% |
| `retire-` | +++ (large) |
| `retire+` | −−− (large) |
| `minority-` | +4% (populism) |
| `minority+` | −3% (culture war) |
| `diplomacy-` | +3% (nationalism) |
| `reform_currency` | −25% |

### Elections

Every 4 years (`turnCount % 4 == 0`):

$$\text{Victory if } Popularity > 0.50$$

$$\text{Defeat (Game Over) if } Popularity \leq 0.50$$

---

## 11. Random Events

### Probability System

Each year, multiple independent rolls are made using the Mersenne Twister generator (`std::mt19937`).

### Event Table

| Event | Probability | Condition | Effects |
| :--- | :--- | :--- | :--- |
| **Pandemic** | 2% | — | Population ×0.95, GDP ×0.98 |
| **Tech breakthrough** | ~10% | — | GDP ×1.05 |
| **Corruption scandal** | 10% | — | Popularity −10% |
| **Food contamination** | 5% | — | Inflation +1%, Popularity −3% |
| **Nuclear accident** | 0.5% | Industrial power > 0.6 | GDP ×0.8, `food_radiation = 1.0` (permanent) |
| **MCI (mass casualty)** | 1% | — | 500–2,000 casualties vs. hospital capacity |
| **Industrial accident** | 15–30% dynamic | — | −$50M, +100 CO2, burns and deaths |
| **Transport collapse** | Inverse to road quality | — | Roads −5%, 50–150 deaths |
| **Aviation crash** | 0.1% | — | ~250 deaths, Popularity −5% |
| **International sanctions** | Cumulative | UN < 0.3 | GDP −5%, Inflation +2% |
| **Terrorist attack** | (radicalism − 0.15) × (1 − intelligence) | Radicalism > 15% | Deaths, GDP −0.5%, Polarization +2% |

### Mass Casualty Incident (MCI)

$$Hospital\_Capacity = Hospitals \times 15$$

$$Casualties\_generated \in [500, 2000]$$

$$\text{If } Casualties > Capacity \Rightarrow Deaths = Casualties - Capacity, \text{ Popularity} \downarrow$$

### Nuclear Mitigation

- Country without industry (`industrial_power < 0.6`): no nuclear risk.
- High educational quality: probability halved to 0.25% (0.5% base → 0.25% with optimal education).

### Industrial Accident Probability (Dynamic)

$$P_{accident} = (fossil\_fuel\_dependency \times 20\%) + (industrial\_power \times 10\%)$$

Typical range: 15%–30% per year in industrialized countries with high fossil fuel dependency.
