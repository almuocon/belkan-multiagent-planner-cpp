Simulación de Agentes Autónomos: Búsqueda y Navegación en C++

Proyecto de Inteligencia Artificial desarrollado para la Universidad de Granada (ETSIIT).
El software modela un entorno de simulación 2D/3D donde dos agentes colaborativos (Ingeniero y Técnico) deben tomar decisiones estratégicas, explorar mapas de orografía compleja y planificar rutas óptimas mediante algoritmos reactivos y deliberativos.

Tecnologías y Algoritmos:

Lenguaje: C++ (C++11/17)

Algoritmos de Búsqueda: A* (Heurísticas guiadas por energía/distancia), Búsqueda en Grafos, Dijkstra.

Técnicas de IA: Agentes deliberativos, agentes reactivos basados en sensores, gestión de estados y restricciones topológicas (cotas de terreno).

Entorno: Linux / CMake / OpenGL (FreeGLUT).

Mi Contribución:
Sobre la base del motor y simulador proporcionado por el departamento CCIA de la UGR, implementé la lógica de control de los agentes dentro de la carpeta Comportamientos_Agentes:

Navegación Deliberativa Óptima: Diseño de algoritmos de búsqueda para resolver rutas en tiempo mínimo considerando restricciones de salto y desnivel.

Minimización de Coste Energético (Algoritmo A*): Implementación de una heurística ponderada para que el agente planifique su trayectoria minimizando el gasto de batería sobre superficies irregulares (agua, hierba, senderos).

Planificación Topológica de Recursos: Algoritmo para proyectar tramos de canalización respetando caída por gravedad y desniveles de cota.

Exploración Reactiva: Procesamiento de sensores visuales cónicos para esquivar obstáculos y actualizar mapas dinámicamente.

Compilación y Ejecución (Linux):

Compilación mediante CMake y script base:
./install.sh

Ejecución con interfaz gráfica (Ejemplo Nivel 2):
./practica2 -m ./mapas/mapa30.map -n 2

Ejecución en modo batch (consola para depuración/tests):
./practica2SG -m ./mapas/mapa30.map -n 3 -seed 1
