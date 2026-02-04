# Arquitectura y Conceptos de C++ en Homo Politicus

Este documento ha sido creado para explicar, con fines didácticos, por qué el proyecto está organizado de esta manera y qué conceptos de C++ estamos utilizando.

## 1. Estructura del Proyecto (Carpetas)

En proyectos profesionales de C++, no ponemos todos los archivos juntos. Usamos una estructura estándar:

*   **`src/` (Source)**: Aquí van los archivos `.cpp` (implementación). Es donde escribimos el código real, la lógica de *cómo* funcionan las cosas.
*   **`include/`**: Aquí van los archivos `.hpp` (headers/cabeceras). Es donde definimos *qué* cosas existen (declaraciones).
*   **`Makefile`**: Es nuestro "chef". Contiene la receta para cocinar (compilar) todos esos archivos sueltos y convertirlos en un solo programa ejecutable (`HomoPoliticus`).

**¿Por qué separar?**
Para mantener el orden. Cuando el proyecto crezca a 100 archivos, agradecerás saber que todo el código lógico está en `src` y todas las definiciones en `include`.

## 2. Archivos Header (.hpp) vs. Source (.cpp)

Esta es una de las dudas más comunes al aprender C++.

### El `.hpp` (El Contrato o Menú)
Imagina que el archivo `.hpp` es el menú de un restaurante. Te dice qué platos hay (funciones, variables), pero no te trae la comida ni te dice cómo se cocina.
*   **Ejemplo**: En `Game.hpp` decimos "Existe una función llamada `run()`".

### El `.cpp` (La Cocina)
Es donde realmente ocurre la acción. Aquí escribimos el código que hace lo que prometimos en el `.hpp`.
*   **Ejemplo**: En `Game.cpp` escribimos el código `while(isRunning) { ... }` que hace funcionar a `run()`.

**Beneficio**: Permite que otros archivos "conozcan" las clases incluyendo solo el `.hpp` sin necesidad de recompilar todo el código complejo cada vez.

## 3. Las Clases Principales

Hemos dividido el problema en dos grandes partes (Clases):

### A. La Clase `Game` ("El Motor")
*   **Responsabilidad**: Su único trabajo es *hacer que el juego funcione*.
*   Maneja el tiempo (el bucle `while`).
*   Maneja la visualización (`render`).
*   No le importa *qué* país es, solo le importa *que hay* un país y que debe actualizarlo.
*   Es el "Jefe de Orquesta".

### B. La Clase `Country` ("El Modelo de Datos")
*   **Responsabilidad**: Almacenar la información del país.
*   Aquí vive la población, el PIB, la popularidad.
*   No sabe nada sobre gráficos ni bucles de tiempo. Solo sabe sobre política y economía.

## 4. El Uso de `struct` (Estructuras)

En `Country.hpp`, verás que usamos bloques `struct`, como `WelfareSocietySystem`.

```cpp
struct WelfareSocietySystem {
    double pandemic_prob;
    double literacy_rate;
    // ...
};
```

**¿Por qué?**
Teníamos más de 50 variables. Si las pusiéramos todas sueltas en la clase `Country`, tendríamos una lista gigante y desordenada:
`country.pandemic_prob`, `country.gdp`, `country.literacy_rate`...

Al agruparlas en estructuras temáticas, el código se vuelve más legible y lógico (usamos la notación de punto `.`):
*   `country.welfare.pandemic_prob` (Probabilidad de pandemia DENTRO del sistema de bienestar).
*   `country.economy.gdp` (PIB DENTRO del sistema económico).

Es como organizar archivos en carpetas en tu computadora en lugar de tenerlos todos en el Escritorio.

## 5. El Bucle del Juego (Game Loop)

En `main.cpp`, verás que el código es muy corto. Solo crea un `Game` y llama a `run()`. Esto es una buena práctica: `main` debe ser limpio.

El método `Game::run()` es un bucle infinito (`while(isRunning)`).
1.  **Process Events**: ¿El usuario tocó alguna tecla?
2.  **Update**: Calculamos los cambios matemáticos (creció la población, bajó el PIB).
3.  **Render**: Mostramos los resultados en pantalla.
4.  **Sleep**: Esperamos un poco para no saturar el procesador (y para que puedas leer el texto).

¡Así es como funcionan casi todos los videojuegos, desde Pac-Man hasta Call of Duty!
