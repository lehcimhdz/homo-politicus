# Homo Politicus - Simulator

**Homo Politicus** es un simulador político y económico de estrategia por turnos profundo y realista.
Asumes el papel de Presidente. Tu objetivo es sobrevivir a las elecciones democráticas mientras gestionas una economía interconectada y una sociedad viva.

---

## 🚀 Nuevas Mecánicas Avanzadas
Este simulador ya no es lineal. Cada decisión tiene consecuencias interconectadas.

### 🧠 Índice de Salud Mental y Suicidios
Tus ciudadanos sienten desesperación económica.
*   **Depresión**: Aumenta con el **Desempleo**, la **Inflación** y la **Corrupción**.
*   **Consecuencia**: Si la Salud Mental cae, la **Tasa de Suicidios** se dispara.
*   *Consejo*: No basta con tener hospitales; la gente necesita esperanza.

### 🎓 La Trampa de la Educación
Educar a tu población es un arma de doble filo.
1.  **Tech Boom**: Si tienes `Alfabetización > 90%` y `Calidad > 70%`, obtienes un **Bonus de +1.5% al PIB**.
2.  **La Trampa**:
    *   Si educas a la gente pero eres **Corrupto** -> **Protestas Masivas**.
    *   Si educas a la gente pero no creas **Industria** -> **Fuga de Cerebros** y **Desempleo Estructural**.
3.  **Techo de Cristal**: La alfabetización está limitada por la **Matrícula Primaria**. Si hay crisis (Desempleo > 10%), los niños dejan la escuela y el futuro se pierde.

### 📉 Mercado Laboral Dinámico (Target Drift)
El desempleo es un monstruo de 4 cabezas:
1.  **Ciclo**: Crecer reduce el paro.
2.  **Automatización**: Alta tecnología (`tech_power`) destruye empleos básicos.
3.  **Rigidez**: Sindicatos fuertes protegen salarios pero dificultan la contratación.
4.  **Drift**: El mercado laboral es lento. Tarda años en recuperarse de una crisis.

### 🧪 Presupuesto de Ciencia Inteligente (Maslow)
El gobierno invierte en ciencia según la prosperidad.
*   **Prosperidad**: Si la economía va bien, el presupuesto de I+D sube solo hasta el 4% del PIB.
*   **Austeridad**: En crisis, la ciencia es lo primero que se corta.

### 🏗️ Mantenimiento de Infraestructura
Todo se degrada un 5% al año.
*   Si `maintenance_level < 50%`:
    *   Riesgo de **Colapso de Puentes** (Maja el PIB).
    *   Riesgo de **Incendios/Explosiones** (Mata gente).
    *   Riesgo de **MCI (Mass Casualty Incidents)**: Si tus hospitales no dan abasto ante un desastre, el sistema colapsa.
*   *Comando*: `invest_maintenance` ($10M) para reparar.

---

## 🎮 Guía de Comandos

| Comando | Coste | Efecto Principal | Efecto Secundario |
| :--- | :--- | :--- | :--- |
| **`next`** | - | **Avanzar Año** | Ejecuta toda la simulación (PIB, Población, Eventos). |
| **`invest_infra`** | $50M | **+Crecimiento PIB** | Mejora carreteras. Vital a largo plazo. |
| **`invest_education`**| $20M | **+Alfabetización** | Sube calidad y matrícula. Cuidado con la fuga de cerebros. |
| **`invest_health`** | $10M | **+Salud / -Muertes** | Aumenta cobertura. Vital para pandemias. |
| **`invest_security`** | $10M | **-Crimen** | Reduce homicidios. Mejora popularidad. |
| **`invest_maintenance`**| $10M | **Repara Infra** | Evita desastres catastróficos (Puentes, Incendios). |
| **`tax+` / `tax-`** | - | **Ajuste Fiscal** | Sube/Baja recaudación a costa de popularidad. |
| **`exit`** | - | **Salir** | Guarda (mentalmente) y cierra. |

---

## Compile & Run

```bash
make
./HomoPoliticus
```

*Desarrollado para simular la complejidad real de gobernar.*
