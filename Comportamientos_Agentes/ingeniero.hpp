#ifndef COMPORTAMIENTOINGENIERO_H
#define COMPORTAMIENTOINGENIERO_H

#include <cstdlib>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <thread>

#include "comportamientos/comportamiento.hpp"

using namespace std;

class ComportamientoIngeniero : public Comportamiento {
public:
  // =========================================================================
  // CONSTRUCTORES
  // =========================================================================
  
  /**
   * @brief Constructor para niveles 0, 1 y 6 (sin mapa completo)
   * @param size Tamaño del mapa (si es 0, se inicializa más tarde)
   */
  ComportamientoIngeniero(unsigned int size = 0) : Comportamiento(size) {
    last_action = IDLE;
    tiene_zapatillas = false;

    mapa_suficiente = false;

    ultimas_posiciones = 0;
    ultimaF = -1;
    ultimaC = -1;
  }

  /**
   * @brief Constructor para niveles 2, 3, 4 y 5 (con mapa completo conocido)
   * @param mapaR Mapa de terreno conocido
   * @param mapaC Mapa de cotas conocido
   */
  ComportamientoIngeniero(std::vector<std::vector<unsigned char>> mapaR, 
                         std::vector<std::vector<unsigned char>> mapaC): 
                         Comportamiento(mapaR, mapaC) {
    hayPlan = false;
    tiene_zapatillas = false;
    
    planCalculado = false;
    estadoActual = BUSCANDO_PLAN;
    ultimaF = -1;
    ultimaC = -1;
    esperando_install = false;
  }

  ComportamientoIngeniero(const ComportamientoIngeniero &comport)
      : Comportamiento(comport) {}
  ~ComportamientoIngeniero() {}

  /**
   * @brief Bucle principal de decisión del agente.
   * Estudia los sensores y decide la siguiente acción.
   * 
   * EJEMPLO DE USO:
   * Action accion = think(sensores);
   * return accion; // El motor ejecutará esta acción
   */
  Action think(Sensores sensores);

  ComportamientoIngeniero *clone() {
    return new ComportamientoIngeniero(*this);
  }

  // =========================================================================
  // ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
  // =========================================================================

  struct EstadoI{
    ubicacion site;
    bool zapatillas;
    
    bool operator==(const EstadoI &st) const {
      return (site.f == st.site.f && site.c == st.site.c && 
            site.brujula == st.site.brujula && zapatillas == st.zapatillas);
    }
    
    bool operator<(const EstadoI &st) const {
      if (site.f != st.site.f) return site.f < st.site.f;
      if (site.c != st.site.c) return site.c < st.site.c;
      if (site.brujula != st.site.brujula) return site.brujula < st.site.brujula;
      return zapatillas < st.zapatillas;
    }
  };

  struct NodoI{
    EstadoI estado;
    list<Action> secuencia;
    int coste;

    bool operator==(const NodoI &node) const{
      return (estado == node.estado);
    }

    bool operator<(const NodoI &node) const{
      if(estado.site.f < node.estado.site.f) return true;
      else if ( (estado.site.f == node.estado.site.f) && (estado.site.c < node.estado.site.c) ) return true;
      else if ( ((estado.site.f == node.estado.site.f) && (estado.site.c == node.estado.site.c)) && (estado.site.brujula < node.estado.site.brujula) ) return true;
      else if ( ((estado.site.f == node.estado.site.f) && (estado.site.c == node.estado.site.c)) && 
                ((estado.site.brujula == node.estado.site.brujula) && (estado.zapatillas < node.estado.zapatillas)) ) return true;
      else return false;
    }
  };

  struct Paso{
    int fil;
    int col;
    int op; //-1 bajar casilla, 0 dejar como esta, 1 subir casilla
  };

  struct EstadoPaso{
    int f, c;
    int altura;

    bool operator<(const EstadoPaso &otro) const {
      if (f != otro.f) return f < otro.f;
      if (c != otro.c) return c < otro.c;
      return altura < otro.altura;
    }
  };

  struct NodoPaso {
    EstadoPaso estado;
    list<Paso> secuencia;
    int energia;
    int impacto;
    int g;
    int h;

    int f() const { 
      return g + h; 
    }
  };

  struct ComparaCoste {
  bool operator()(const NodoPaso &a, const NodoPaso &b) {
    int f_a = a.f();
    int f_b = b.f();

    if (f_a != f_b) {
      return f_a > f_b; 
    }

    return a.impacto > b.impacto;
  }
};

   /** 
   * @brief Busqueda en nivel 2
   * 
   * @param inicio Estado Inicial de la busqueda.
   * @param final Estado Final de la busqueda.
   * @param terreno Matriz que contiene la informacion del terreno.
   * @param altura Matriz que contiene la altura del mapa
   * 
   * @return La secuencia de acciones para llegar al estado final
   * @note Devuelve un plan vacio si no es posible encontrar un plan valido
  */
  list<Action> Busqueda_I(const EstadoI &inicio, const EstadoI &final, 
                                 const vector<vector<unsigned char>> &terreno, 
                                 const vector<vector<unsigned char>> &altura);

   /** 
   * @brief Busqueda en nivel 3
   * 
   * @param inicio Estado Inicial de la busqueda. 
   * @param final Estado Final de la busqueda.
   * @param terreno Matriz que contiene la informacion del terreno.
   * @param altura Matriz que contiene la altura del mapa
   * 
   * @return La secuencia de pasos para llegar al estado final
   * @note Devuelve un plan vacio si no es posible encontrar un plan valido
  */
  list<Paso> Busqueda_Plan_PasoI(Sensores sensores);

  // Funciones específicas para cada nivel (para ser implementadas por el alumno)
  /**
   * @brief Implementación del Nivel 0.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_0(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 1.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_1(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 2.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */ 
  Action ComportamientoIngenieroNivel_2(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 3.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_3(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 4.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_4(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 5.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_5(Sensores sensores);
  
  /**
   * @brief Implementación del Nivel 6.
   * @param sensores Datos actuales de los sensores del agente.
   * @return Acción a realizar.
   */
  Action ComportamientoIngenieroNivel_6(Sensores sensores);

protected:
  // =========================================================================
  // FUNCIONES PROPORCIONADAS
  // =========================================================================

  /**
   * @brief Actualiza la información del mapa interno basándose en los sensores.
   * IMPORTANTE: Esta función ya está implementada. Actualiza mapaResultado y mapaCotas
   * con la información de los 16 sensores (casilla actual + 15 casillas alrededor).
   */
  void ActualizarMapa(Sensores sensores);

  /**
   * @brief Comprueba si una casilla es transitable.
   * @param f Fila de la casilla.
   * @param c Columna de la casilla.
   * @param tieneZapatillas Indica si el agente posee zapatillas.
   * @return true si la casilla es transitable (no es muro ni precipicio).
   */
  bool EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas);

  /**
   * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
   * REGLAS: Desnivel máximo 1 sin zapatillas, 2 con zapatillas.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return true si el desnivel con la casilla de delante es admisible.
   */
  bool EsAccesiblePorAltura(const ubicacion &actual, bool zap);

  /**
   * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
   * @param actual Estado actual del agente (fila, columna, orientacion).
   * @return Estado con la fila y columna de la casilla de enfrente.
   */
  ubicacion Delante(const ubicacion &actual) const;

  bool es_camino(unsigned char c) const;

  /**
 * @brief Imprime por consola la secuencia de acciones de un plan para un agente.
 * @param plan  Lista de acciones del plan.
 */
  void PintaPlan(const list<Action> &plan);


/**
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 * @param plan  Lista de pasos (fila, columna, operación).
 */
  void PintaPlan(const list<Paso> &plan);


  /**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa gráfico.
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
  void VisualizaPlan(const ubicacion &st, const list<Action> &plan);

  /**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
  void VisualizaRedTuberias(const list<Paso> &plan);



private:
  // =========================================================================
  // VARIABLES DE ESTADO (PUEDEN SER EXTENDIDAS POR EL ALUMNO)
  // =========================================================================
  Action last_action;
  bool tiene_zapatillas;
  list<Action> plan;
  bool hayPlan;
  list<Paso> planTuberias;

  
  // Nivel 5
  enum EstadoIng { 
    BUSCANDO_PLAN, 
    PREPARANDO_CASILLA, 
    LLAMANDO_TECNICO,
    MOVIENDOSE,
    ORIENTANDOSE,
    ESPERANDO_TECNICO
  };
  EstadoIng estadoActual;

  // listas para guardar los datos
  list<Paso> listaTuberias; 
  list<Action> listaMovimiento;
    
  // posicion ingeniero
  int ultimaF, ultimaC; 
  bool planCalculado;
  bool esperando_install;

  // funciones de busqueda
  list<Paso> PlanificarTuberias(int belF, int belC, int maxEco);
  list<Action> BuscarCaminoBFS(int fO, int cO, int rO, int fD, int cD);

  enum EstadoNivel6 { EXPLORANDO, PLANIFICANDO, EJECUTANDO };
  EstadoNivel6 estadoNivel6;
  set<pair<int,int>> visitadas;
  bool mapa_suficiente;

  // ====== DEBUG / ANTIBLOQUEO ======
  int ultimas_posiciones;


};

#endif
