# Homo Politicus

**Homo Politicus** es un simulador político y económico de estrategia por turnos escrito en C++.
Asumes el papel de Presidente de una nación ficticia. Tu objetivo: mantenerte en el poder ganando elecciones cada 4 años, mientras gestionas la economía, el bienestar social y evitas el caos.

## 🚀 Cómo Empezar

### Requisitos
- Compilador de C++ (clang++ o g++)
- Make

### Compilación
Abre tu terminal en la carpeta del proyecto y ejecuta:
```bash
make
```

### Ejecución
```bash
./HomoPoliticus
```

---

## 🎮 Cómo Jugar

El juego es **por turnos**. En cada año, puedes tomar decisiones antes de avanzar.

### Comandos Principales
| Comando | Acción | Efecto |
| :--- | :--- | :--- |
| **`next`** | Avanzar Año | Pasa el tiempo. La población crece, el PIB cambia, ocurren eventos. |
| **`exit`** | Salir | Cierra el juego. |

### 💰 Gestión Fiscal
| Comando | Acción | Efecto |
| :--- | :--- | :--- |
| **`tax+`** | Subir Impuestos | 📈 +10% Recaudación, 📉 -5% Popularidad, 📈 +1% Inflación. |
| **`tax-`** | Bajar Impuestos | 📉 -10% Recaudación, 📈 +3% Popularidad. |

### 🏗️ Presupuesto e Inversión
Gasta tu PIB para mejorar el país permanentemente.

| Comando | Coste | Efecto |
| :--- | :--- | :--- |
| **`invest_health`** | $10M | Mejora la Cobertura de Salud (+5%) y Popularidad (+2%). |
| **`invest_security`**| $10M | Reduce la Tasa de Homicidios (-1.0) y mejora Popularidad (+1%). |
| **`invest_infra`** | $50M | **Aumenta la Tasa de Crecimiento (+0.1%)**. La mejor inversión a largo plazo. |

---

## ⚙️ Mecánicas del Juego

### 🗳️ Elecciones
Cada **4 años** se celebran elecciones democráticas.
- Si tu **Popularidad > 50%**: Ganas la reelección.
- Si tu **Popularidad <= 50%**: Pierdes y el juego termina (**GAME OVER**).

### 🎲 Eventos Aleatorios
Cada año hay un 30% de probabilidad de que ocurra un evento:
- **Pandemia**: La población muere y la economía se contrae.
- **Escándalo de Corrupción**: Tu popularidad cae drásticamente.
- **Avance Tecnológico**: El PIB crece rápidamente.

### 📊 Economía
- **PIB (GDP)**: Crece anualmente basado en la `growth_rate`.
- **Población**: Crece basado en nacimientos vs muertes.

---

## 📂 Estructura del Código
- `src/main.cpp`: Punto de entrada.
- `src/Game.cpp`: Bucle principal y lógica de comandos.
- `src/Country.cpp`: Datos del país.
- `src/EventManager.cpp`: Lógica de eventos aleatorios.
- `include/`: Archivos de cabecera (.hpp).
