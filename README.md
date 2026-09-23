# Simulación de Agentes Autónomos: Búsqueda y Navegación en C++

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Build System](https://img.shields.io/badge/Build-CMake-064F8C.svg)](https://cmake.org/)
[![Graphics Engine](https://img.shields.io/badge/Graphics-OpenGL%20%2F%20FreeGLUT-orange.svg)](https://www.opengl.org/)
[![License: Academic](https://img.shields.io/badge/License-Academic-lightgrey.svg)](LICENSE)

Implementación de comportamientos para dos agentes autónomos colaborativos (Ingeniero y Técnico) en un entorno discreto 2.5D con cotas de elevación, restricciones topológicas y optimización de recursos energéticos.

---

## Contexto Académico

Proyecto desarrollado durante el **segundo curso del Grado en Ingeniería Informática** en la **Universidad de Granada (UGR)** para la asignatura *Inteligencia Artificial* (curso 2025/2026), impartida en la **E.T.S. de Ingenierías Informática y de Telecomunicación (ETSIIT)**.

* **Entorno base:** El simulador gráfico (motor 2D/3D con OpenGL/FreeGLUT), el modelo sensorial y la infraestructura de simulación por turnos fueron suministrados por el **Departamento de Ciencias de la Computación e Inteligencia Artificial (DECSAI / CCIA)** de la UGR.
* **Autoría del trabajo propio:** Diseño, lógica algorítmica e implementación íntegra de la toma de decisiones dentro del directorio `Comportamientos_Agentes/` (`ingeniero.hpp`, `ingeniero.cpp`, `tecnico.hpp` y `tecnico.cpp`).

---

## Aspectos Técnicos y Decisiones de Diseño

* **Búsqueda heurística informada ($A^*$):** Planificación deliberativa guiada por funciones de coste real dependientes de la superficie (agua, hierba, senderos) y diferenciales de cota, usando como heurística la distancia de Chebyshev en espacios con 8 orientaciones de movimiento.
* **Minimización de consumo energético y restricciones físicas:** Modelado de saltos y pasos condicionados por la posesión de zapatillas y desniveles del terreno ($\Delta h \le 1$ o $\le 2$), integrando costes dinámicos de movimiento y penalizaciones por giros.
* **Diseño y canalización topológica:** Algoritmo de planificación de canalizaciones que optimiza la ruta de transporte garantizando caída por gravedad, balanceando coste de instalación e impacto ecológico mediante nivelación del terreno (`DIG` / `RAISE`).
* **Máquinas de estados finitos (FSM) y coordinación multiagente:** Protocolos de comunicación y sincronización punto a punto (`COME`, invocación por sensores `venpaca` y alineación ortogonal frontal mutua) para la ejecución atómica y simultánea de acciones cooperativas (`INSTALL`).
* **Exploración reactiva con memoria espacial:** Estrategias de reconocimiento basadas en conos de visión cónicos/ortogonales (vectores sensoriales de 16 elementos) combinadas con matrices de frecuencias de visita para evitar bucles en entornos parcialmente observados o desconocidos.

---

## Estado de los Niveles

| Nivel | Rol / Objetivo | Enfoque | Estado |
| :--- | :--- | :--- | :--- |
| **Nivel 0** | Llegada reactiva individual a bases de tratamiento de residuos (`U`) | Reactivo con memoria de visitas | **Completado** |
| **Nivel 1** | Cartografiado y exploración reactiva de sendas (`C` y `S`) en mapa desconocido | Reactivo / Priorización de `?` | **Completado** |
| **Nivel 2** | Localización óptima del objetivo en número de turnos para el Ingeniero | Búsqueda deliberativa en grafo | **Completado** |
| **Nivel 3** | Ruta de mínimo consumo de energía para el Técnico | Algoritmo $A^*$ con heurística de Chebyshev | **Completado** |
| **Nivel 4** | Planificación de red de tuberías (gravedad + impacto ecológico acotado) | Búsqueda $A^*$ sobre transformaciones de cota | **Completado** |
| **Nivel 5** | Coordinación, nivelación del terreno y montaje físico de la red | FSM colaborativa (Ingeniero + Técnico) | **Funcional** |
| **Nivel 6** | Exploración y canalización integral en mapas totalmente ciegos | Híbrido (Reactivo + $A^*$) | **En desarrollo / Parcial** |

---

## Estructura del Código Propio

```text
Comportamientos_Agentes/
├── ingeniero.hpp  # Definición de estructuras de búsqueda (EstadoI, NodoI, NodoPaso) y FSM del Ingeniero
├── ingeniero.cpp  # Implementación de A* ecológico, BFS de maniobra, FSM del nivel 5 y sensores de cota
├── tecnico.hpp    # Definición de estados para A* energético (EstadoT, NodoT) y fases del Técnico
└── tecnico.cpp    # Algoritmo A* dependiente del terreno, FSM de seguimiento colaborativo y exploración
```

---

## Compilación y Ejecución

### Requisitos previos (Linux / Ubuntu)
```bash
sudo apt update
sudo apt install build-essential cmake freeglut3-dev libjpeg-dev libxmu-dev libxi-dev libboost-all-dev
```

### Compilación
El proyecto incluye un script de instalación y construcción automática:
```bash
./install.sh
```

### Modos de Ejecución

* **Modo Gráfico (Simulador interactivo):**
  ```bash
  # Ejecución general con selector de mapa y nivel
  ./practica2

  # Lanzar directamente un nivel concreto (ej. Nivel 2 en mapa30):
  ./practica2 -m ./mapas/mapa30.map -n 2
  ```

* **Modo Consola / Batch (Sin interfaz, ideal para depuración o tests):**
  ```bash
  ./practica2SG -m ./mapas/mapa30.map -n 3 -seed 1
  ```

---

## Créditos y Agradecimientos

* **Desarrollo de comportamientos:** Almudena O. ([@almuocon](https://github.com/almuocon)) — Estudiante de Ingeniería Informática en la **Universidad de Granada (ETSIIT - UGR)**.
* **Entorno y motor de simulación:** Departamento de Ciencias de la Computación e Inteligencia Artificial (**DECSAI / CCIA**), Universidad de Granada.
