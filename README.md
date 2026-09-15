Simulación de Agentes Autónomos: Búsqueda y Navegación en C++

Proyecto académico de la asignatura Inteligencia Artificial (curso 2025/2026), E.T.S. de Ingenierías Informática y de Telecomunicación (ETSIIT), Universidad de Granada.

Créditos y Autoría del Software Base:
El entorno de simulación, el motor gráfico 2D/3D en OpenGL y la estructura base del proyecto pertenecen al Departamento de Ciencias de la Computación e Inteligencia Artificial (CCIA) de la Universidad de Granada.

Mi trabajo como estudiante se centra de forma exclusiva en el diseño e implementación del código contenido dentro de la carpeta Comportamientos_Agentes (archivos ingeniero.cpp, ingeniero.hpp, tecnico.cpp y tecnico.hpp).

Estado del Desarrollo y Niveles:
El proyecto formó parte de un proceso de aprendizaje continuo y no se encuentra completado al 100%:

Nivel 2 y 3 (Completados y funcionales): Implementación con éxito de algoritmos de búsqueda deliberativa y pathfinding mediante el algoritmo A* con minimización de consumo de energía sobre orografía con desniveles.

Nivel 4 (Funcional): Planificación topológica de canalizaciones de tuberías respetando restricciones de altura, desniveles y límites energéticos.

Niveles 0, 1 y 5 (Parciales): Pruebas de comportamientos reactivos y coordinación básica entre agentes para la instalación simultánea de elementos.

Nivel 6 (No implementado / Pendiente): Exploración y construcción en mapas completamente desconocidos sin completar.

Tecnologías y Algoritmos:

Lenguaje: C++

Algoritmos de Búsqueda: A* (Heurísticas guiadas por coste energético), Búsqueda en Grafos, Dijkstra.

Técnicas de IA: Agentes deliberativos, agentes reactivos basados en sensores, gestión de estados y restricciones topológicas.

Entorno: Linux / CMake / OpenGL (FreeGLUT).

Compilación y Ejecución (Linux):

Compilación mediante CMake y script base:
./install.sh

Ejecución con interfaz gráfica (Ejemplo Nivel 2):
./practica2 -m ./mapas/mapa30.map -n 2

Ejecución en modo batch (consola para depuración/tests):
./practica2SG -m ./mapas/mapa30.map -n 3 -seed 1
