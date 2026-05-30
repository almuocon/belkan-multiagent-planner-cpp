#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores) {
  Action accion = IDLE;


  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_2(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    //case 3: accion = ComportamientoTecnicoNivel_E(sensores); break;
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
}

//--------------------------------------- NIVEL 0 -------------------------------------------
//-------------------------------------------------------------------------------------------

/*
Ampliar el rango de visión más allá de las casillas adyacentes.
Escanea todo el vector de superficie (hasta la posición 15) buscando el objetivo ('U' o 'D').
Divide el cono de visión en tres sectores (Izquierda, Centro, Derecha). 
Si detecta el objetivo en el sector izquierdo, devuelve 1 (giro izquierda), si está en el centro 2 (avanzar), etc.
*/
/**
 * @brief Determina si a lo lejos hay algo interesante
 * @param superficie vector del espacio que hay alrededor
 * @param objetivo caracter de la casilla objetivo
 * @param i terreno que hay en la posición 1 de superficie (45 izq)
 * @param c terreno que hay en la posición 2 de superficie (justo delante)
 * @param d terreno que hay en la posición 3 de superficie (45 dch)
 * @return 2 si es mejor WALK, 1 para TURN_SL y 3 para TURN_SR. 0 no hay nada interesante.
 */
int VerLejosT(const vector<unsigned char> &superficie, char objetivo, char i, char c, char d) {
    // centro
    if (c != 'P') {
        if (superficie[2] == objetivo || superficie[6] == objetivo || superficie[12] == objetivo) 
            return 2;
    }

    // izquierda
    if (i != 'P') {
        if (superficie[1] == objetivo || superficie[4] == objetivo || 
            superficie[5] == objetivo || superficie[9] == objetivo || 
            superficie[10] == objetivo || superficie[11] == objetivo) 
            return 1;
    }

    // derecha
    if (d != 'P') {
        if (superficie[3] == objetivo || superficie[7] == objetivo || 
            superficie[8] == objetivo || superficie[13] == objetivo || 
            superficie[14] == objetivo || superficie[15] == objetivo) 
            return 3;
    }

    return 0;
}

/*
Busca en las posiciones 1 (izq), 2 (frente) y 3 (der) del sensor de superficie.
Prioridades:
Si ve la meta ('U'), devuelve la dirección para ir a ella.
Si no tiene zapatillas y ve unas ('D'), devuelve la dirección para recogerlas.

Nivel 1: También busca casillas desconocidas '?' para explorar.
*/
/**
 * @brief Determina la mejor opcion entre las 3 casillas que tiene delante.
 * @param i terreno que hay en la posición 1 de superficie (45 izq)
 * @param c terreno que hay en la posición 2 de superficie (justo delante)
 * @param d terreno que hay en la posición 3 de superficie (45 dch)
 * @param zap indica si estoy en posesión de las zapatillas
 * @param ni indica si la posicion 1 de superficie es '?'
 * @param nc indica si la posicion 2 de superficie es '?'
 * @param nd indica si la posicion 3 de superficie es '?'
 * @return 2 si es mejor WALK, 1 para TURN_SL y 3 para TURN_SR. 0 no hay nada interesante.
 */
int VeoCasillaInteresanteT(char i, char c, char d, bool zap, int nivel, bool ni, bool nc, bool nd) {
  
  if (nivel == 0) { 
    if (c == 'U') return 2;
    else if (i == 'U') return 1;
    else if (d == 'U') return 3;

    if (!zap) {
      if (c == 'D') return 2;
      else if (i == 'D') return 1;
      else if (d == 'D') return 3;
    }
    
    return 0;
  }

  if (nivel == 1) { 
    //Buscar zapatillas
    if (!zap) {
      if (c == 'D') return 2;
      else if (i == 'D') return 1;
      else if (d == 'D') return 3;
    }
    // Casillas desconocidas '?' 
    if (nc && c != 'P') return 2;
    if (ni && i != 'P') return 1;
    if (nd && d != 'P') return 3;

    // Camino y Sendero y el bosque si tiene zapatillas
    if (c == 'C' || c == 'S' || (zap && (c == 'B')) || c == 'H') return 2;
    if (i == 'C' || i == 'S' || (zap && (i == 'B')) || i == 'H') return 1;
    if (d == 'C' || d == 'S' || (zap && (d == 'B')) || d == 'H') return 3;
  }

  return 0;
}
/*
Comprueba la diferencia de altura entre la casilla del agente y la de destino.
Si la diferencia es <= 1, es viable.
Si no es viable, devuelve 'P'
*/
/**
 * @brief Determina si casilla viable por altura.
 * @param casilla tipo de terreno
 * @param dif diferencia de altura entre casillas
 * @return 'P' si no es accesible por altura y casilla en otro caso
 */
char ViablePorAlturaT(char casilla, int dif){
  if (abs(dif) <= 1)
    return casilla;
  else
    return 'P';
}

/*
Calcula las coordenadas de la casilla adyacente en esa dirección y consulta en la matriz mapaVisitados cuantas veces hemos pasado por alli.
Sirve para que, si el agente tiene tres caminos libres, elija el que menos ha pisado.
*/
/**
 * @brief Devuelve el número de visitas de una casilla adyacente según la dirección.
 * @param f Fila actual.
 * @param c Columna actual.
 * @param brujula Dirección hacia la que se quiere mirar (0-7).
 * @param mapa Matriz con el conteo de visitas.
 * @return Número de visitas o un valor muy alto si está fuera del mapa.
 */
int VisitadoT(int f, int c, int brujula, const vector<vector<int>>& mapa) {
    int nf = f, nc = c;
    switch (brujula) {
        case 0: nf--; break;
        case 1: nf--; nc++; break;
        case 2: nc++; break;
        case 3: nf++; nc++; break;
        case 4: nf++; break;
        case 5: nf++; nc--; break;
        case 6: nc--; break;
        case 7: nf--; nc--; break;
    }
    if (nf >= 0 && nf < mapa.size() && nc >= 0 && nc < mapa[0].size()) return mapa[nf][nc];
    return 999999;
}

/*
alcanzar una casilla de Tratamiento de Residuos ('U')
*/
Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores) {
  static vector<vector<int>> mapaVisitados;

  //reinicio de estructuras (inicio)
  if (mapaVisitados.empty() || sensores.reset) {
    mapaVisitados.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
    tiene_zapatillas = false;
  }

  Action accion = IDLE;
  ActualizarMapa(sensores); //pone en el mapa global lo q ve el agente

  // registra la pos actual en la memoria de visitas para evitar bucles
  if (sensores.posF != -1) mapaVisitados[sensores.posF][sensores.posC]++;

  // comprueba el estado actual: exito (U) o zapatillas (D)
  if (sensores.superficie[0] == 'U') return IDLE;
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  //comportamiento si encuentra un muro o un agente (colision)
  if (sensores.choque) {
    if (aleatorio(1) == 0) return TURN_SL;
    else return TURN_SR;
  }

  char i, c, d;
  // evalua la viabilidad de las 3 casillas frontales considerando desniveles y agentes
  if (sensores.agentes[1] == '_') i = ViablePorAlturaT(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  else i = 'P';

  if (sensores.agentes[2] == '_') c = ViablePorAlturaT(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  else c = 'P';

  if (sensores.agentes[3] == '_') d = ViablePorAlturaT(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);
  else d = 'P';

  // filtrado de terreno
  if (i != 'C' && i != 'D' && i != 'U') i = 'P';
  if (c != 'C' && c != 'D' && c != 'U') c = 'P';
  if (d != 'C' && d != 'D' && d != 'U') d = 'P';

  // bloquea el paso si hay otro agente ocupando la casilla destino
  if (sensores.agentes[1] != '_') i = 'P';
  if (sensores.agentes[2] != '_') c = 'P';
  if (sensores.agentes[3] != '_') d = 'P';

  // 1. miramos si hay U o D 
  int pos = VeoCasillaInteresanteT(i, c, d, tiene_zapatillas, 0, false, false, false);

  // 2. si no hay U o D, elegimos el camino menos visitado
  if (pos == 0) {
    int v_i = (i != 'P') ? VisitadoT(sensores.posF, sensores.posC, (sensores.rumbo + 7) % 8, mapaVisitados) : 999999;
    int v_c = (c != 'P') ? VisitadoT(sensores.posF, sensores.posC, sensores.rumbo, mapaVisitados) : 999999;
    int v_d = (d != 'P') ? VisitadoT(sensores.posF, sensores.posC, (sensores.rumbo + 1) % 8, mapaVisitados) : 999999;

    //seleccionar la casilla con el valor mínimo de visitas
    if (v_c != 999999 || v_i != 999999 || v_d != 999999) {
      int min_v = min(v_c, min(v_i, v_d));

      vector<int> opciones;
      if (v_c == min_v) opciones.push_back(2);
      if (v_i == min_v) opciones.push_back(1);
      if (v_d == min_v) opciones.push_back(3);

      //si hay empate en visitas, elegir al azar
      pos = opciones[aleatorio(opciones.size() - 1)];
    }
  }

  // 3. buscar objetivos a larga distancia
  if (pos == 0) pos = VerLejosT(sensores.superficie, 'U', i, c, d);
  if (pos == 0 && !tiene_zapatillas) pos = VerLejosT(sensores.superficie, 'D', i, c, d);

  //aciones
  switch(pos) {
    case 2:
      if (c != 'P' && sensores.agentes[2] == '_') accion = WALK;
      else {
        // Bloqueo inesperado: girar para buscar nueva ruta
        if (aleatorio(1) == 0) accion = TURN_SL;
        else accion = TURN_SR;
      }
      break;
    case 1: accion = TURN_SL; break;
    case 3: accion = TURN_SR; break;
    default:
      if (aleatorio(1) == 0) accion = TURN_SL;
      else accion = TURN_SR;
      break;
  }

  last_action = accion;
  return accion;
}

//--------------------------------------- NIVEL 1 -------------------------------------------
//-------------------------------------------------------------------------------------------

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}

bool viableT(char t, bool tiene_zapatillas) {
    if (t == 'C' || t == 'S' || t == 'D' || t == 'U' || t == 'H') return true;
    if (tiene_zapatillas && (t == 'B')) return true;
    return false;
  };

//Nivel 1: mapear la mayor superficie posible del entorno desconocido
/**
 * @brief Comportamiento reactivo del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores) {
  static vector<vector<int>> mapaVisitados;
  static int giros_180 = 0;

  //inicialización de estructuras
  if (mapaVisitados.empty() || sensores.reset) {
    mapaVisitados.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
    tiene_zapatillas = false;
    giros_180 = 0;
  }

  //si se estaba en medio de un giro, girar
  if (giros_180 > 0) {
    giros_180--;
    return TURN_SL;
  }
  if (sensores.agentes[2] == 'i') {
    giros_180 = 3;
    return TURN_SL;
  }

  ActualizarMapa(sensores); //rellena el mapa con '?' y terrenos reales
  if (sensores.posF != -1) mapaVisitados[sensores.posF][sensores.posC]++;
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  //analisis de altura
  char i = ViablePorAlturaT(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAlturaT(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAlturaT(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);

  //analisis de terreno
  if (!viableT(i, tiene_zapatillas)) i = 'P';
  if (!viableT(c, tiene_zapatillas)) c = 'P';
  if (!viableT(d, tiene_zapatillas)) d = 'P';

  int seleccion = 0;

  //1. primero buscamos los objetivos 
  if (c == 'U' || (c == 'D' && !tiene_zapatillas)) seleccion = 2;
  else if (i == 'U' || (i == 'D' && !tiene_zapatillas)) seleccion = 1;
  else if (d == 'U' || (d == 'D' && !tiene_zapatillas)) seleccion = 3;

  // 2. Elegimos la casilla menos visitada 
  if (seleccion == 0) {
    int v_i = VisitadoT(sensores.posF, sensores.posC, (sensores.rumbo + 7) % 8, mapaVisitados);
    int v_c = VisitadoT(sensores.posF, sensores.posC, sensores.rumbo, mapaVisitados);
    int v_d = VisitadoT(sensores.posF, sensores.posC, (sensores.rumbo + 1) % 8, mapaVisitados);

    int min_v = 999999;

    //girar hacia lo menos visto
    if (i != 'P' && v_i < min_v) { min_v = v_i; seleccion = 1; }
    if (d != 'P' && v_d < min_v) { min_v = v_d; seleccion = 3; }
    if (c != 'P' && v_c <= min_v) { min_v = v_c; seleccion = 2; }
  }

  // Si no hay opciones viables, girar 
  if (seleccion == 0) seleccion = 1;

  switch (seleccion) {
    case 2: 
      if (sensores.agentes[2] == '_') return WALK; 
      else return TURN_SR;
    case 1: return TURN_SL;
    case 3: return TURN_SR;
    default: return TURN_SL;
  }
}

//--------------------------------------- NIVEL 2 -------------------------------------------
//-------------------------------------------------------------------------------------------

/*
Nivel 2: Mientras no vea al Ingeniero, permanece en reposo (IDLE) para no interferir en la navegación del mapa. 
Una vez que detecta al Ingeniero, activa su lógica reactiva del Nivel 0
*/
/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {
  bool veo_ingeniero = false;

  // escaneo de todos los sensores de agentes para localizar al ingeniero
  for (int i = 1; i < sensores.agentes.size(); i++) {
    if (sensores.agentes[i] == 'i') {
      veo_ingeniero = true;
      break;
    }
  }

  // Si el ingeniero está a la vista, el tecnico se mueve de forma segura sin chocar y explorando cerca del Ingeniero
  if (veo_ingeniero) {
    return ComportamientoTecnicoNivel_0(sensores);
  }

  return IDLE;
}

//--------------------------------------- NIVEL 3 -------------------------------------------
//-------------------------------------------------------------------------------------------

/*
Calcula las coordenadas que tendría la casilla que esta justo frente al agente 
*/
ComportamientoTecnico::EstadoT NextCasillaTecnico(const ComportamientoTecnico::EstadoT &st){
  ComportamientoTecnico::EstadoT siguiente = st;
  switch(st.site.brujula){
    case norte:
      siguiente.site.f = st.site.f - 1;
      break;

    case noreste:
      siguiente.site.f = st.site.f - 1;
      siguiente.site.c = st.site.c + 1;
      break;

    case este:
        siguiente.site.c = st.site.c + 1;
      break;

    case sureste:
      siguiente.site.f = st.site.f + 1;
      siguiente.site.c = st.site.c + 1;
      break;

    case sur:
        siguiente.site.f = st.site.f + 1;
      break;

    case suroeste:
      siguiente.site.f = st.site.f + 1;
      siguiente.site.c = st.site.c - 1;
      break;

    case oeste:
        siguiente.site.c = st.site.c - 1;
      break;

    case noroeste:
      siguiente.site.f = st.site.f - 1;
      siguiente.site.c = st.site.c - 1;
      break;
  }

  return siguiente;
}

/*
Es la que le dice al algoritmo A* si una casilla es transitable o no
*/
bool CasillaAccesibleTecnico(const ComportamientoTecnico::EstadoT &st, Action accion, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  //posicion a la que queremos ir  
  ComportamientoTecnico::EstadoT next = NextCasillaTecnico(st);
    int limite_altura = 1; 

    //evitamos salir de la matriz del mapa
    if (next.site.f < 0 || next.site.f >= terreno.size() || next.site.c < 0 || next.site.c >= terreno[0].size()) return false;

    //contro de obstaculos
    bool check1 = (terreno[next.site.f][next.site.c] != 'P' && terreno[next.site.f][next.site.c] != 'M');
    bool check2 = false;
    if (terreno[next.site.f][next.site.c] == 'B'){
      check2 = st.zapatillas;
    }else{
      check2 = true;
    }
    //no se puede planificar sobre casillas no descubiertas
    unsigned char casilla = terreno[next.site.f][next.site.c];
    if (casilla == '?') return false;

    //desnivel no supera el limite de 1
    int h_dest = (int)altura[next.site.f][next.site.c];
    int h_orig = (int)altura[st.site.f][st.site.c];
    bool check3 = (abs(h_dest - h_orig) <= limite_altura);

    //mov solo valido si se cumplen las tres cosas
    return (check1 && check2 && check3);
  }

/*
generar un nuevo estado (next) a partir de un estado actual (st) y una accion. 
Lo usan para imaginar el mapa y construir el arbol de busqueda.
Va comprobando si puede realizar esas acciones 
*/
ComportamientoTecnico::EstadoT applyT(Action accion, const ComportamientoTecnico::EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  ComportamientoTecnico::EstadoT next = st;
  switch(accion){
    case WALK:
      if (CasillaAccesibleTecnico(st, WALK, terreno, altura)){
        next = NextCasillaTecnico(st);
      }
      break;

    case TURN_SR:
      next.site.brujula = (Orientacion) ((next.site.brujula+1)%8);
      break;
    
    case TURN_SL:
      next.site.brujula = (Orientacion)((next.site.brujula+7)%8);
      break;
  }

  return next;
}

/*
Funcion que devuelve el coste de cada accion (tabla coste accion)
*/
int CosteAccion(Action accion, unsigned char casilla_ini, int alt_ini, int alt_fin){
  int coste = 0;
  int mod_altura = 0;

  if (alt_fin > alt_ini) mod_altura = 5;
  else if (alt_fin < alt_ini) mod_altura = -2;

  if( accion == WALK){
    if(casilla_ini == 'A'){
      coste = 60 + mod_altura;
    }else if( casilla_ini == 'H'){
      coste = 6 + mod_altura;
    }else if( casilla_ini == 'S' ){
      coste = 3 + mod_altura;
    }else{
      coste = 1;
    }
  }else if((accion == TURN_SL) || (accion == TURN_SR)){
    if(casilla_ini == 'A'){
      coste = 5;
    }else if( casilla_ini == 'H'){
      coste = 2;
    }else if( casilla_ini == 'S' ){
      coste = 1;
    }else{
      coste = 1;
    }
  }

  return coste;
}

/*
Calcula el número mínimo de pasos necesarios para llegar del punto A al punto B en un mapa donde el agente se puede mover en 8 direcciones
Distancia de Chebyshev
*/
int Heuristica(const ComportamientoTecnico::EstadoT &actual, const ComportamientoTecnico::EstadoT &final) {
  int diff_f = abs(actual.site.f - final.site.f);
  int diff_c = abs(actual.site.c - final.site.c);
  return max(diff_f, diff_c); 
}

/*
utiliza el algoritmo A* para encontrar el camino más barato en términos de energía
utiliza una heurística para estimar la distancia al objetivo y una función de Coste real que varía según el terreno que pisa
*/
list<Action> ComportamientoTecnico::Busqueda_T(const EstadoT &inicio, const EstadoT &final, 
                                 const vector<vector<unsigned char>> &terreno, 
                                 const vector<vector<unsigned char>> &altura){
  priority_queue<NodoT, vector<NodoT>, ComparaCoste> frontier;// cola con prioridad exploramos primero el nodo con menor f(n) = g(n) + h(n)
  set<EstadoT> explored; //vistitados
  
  //nodo inicial
  NodoT current_node;
  current_node.estado = inicio;
  current_node.secuencia = {};
  current_node.coste = 0; //g(n)coste acumulado
  current_node.h = Heuristica(inicio, final); //h(n) estimacion al destino
  frontier.push(current_node);
  
  while(!frontier.empty()){
    //extraemos el nodo con menor coste total estimado
    current_node = frontier.top();
    frontier.pop();

    //cond exito: hemos llegado al destino
    if( (current_node.estado.site.f == final.site.f) && (current_node.estado.site.c == final.site.c) ){
      return current_node.secuencia;
    }

    //poda: solo expandimos si no hemos pasado por este estado antes
    if (explored.find(current_node.estado) == explored.end()) {
      explored.insert(current_node.estado);
      
      // Zapatillas
      if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D') {
        current_node.estado.zapatillas = true;
      }

      //generacion de hijos
      // WALK
      NodoT child_walk = current_node;
      child_walk.estado = applyT(WALK, current_node.estado, terreno, altura);

      //si el mov es valido, cambia el estado
      if (!(child_walk.estado == current_node.estado)) {
        //calculamos el coste energetico segun terreno y desnivel
          int gasto = CosteAccion(WALK, terreno[current_node.estado.site.f][current_node.estado.site.c], 
                                  altura[current_node.estado.site.f][current_node.estado.site.c], 
                                  altura[child_walk.estado.site.f][child_walk.estado.site.c]);
          child_walk.coste += gasto; // Sumamos al coste acumulado g(n)
          child_walk.h = Heuristica(child_walk.estado, final); //se vuelve a calcular h
          child_walk.secuencia.push_back(WALK);

          if (explored.find(child_walk.estado) == explored.end()) {
              frontier.push(child_walk);
          }
      }

      //los giros consumen energia segun el terreno
      // TURN_SR
      NodoT child_SR = current_node;
      child_SR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);

      int gasto_SR = CosteAccion(TURN_SR, terreno[current_node.estado.site.f][current_node.estado.site.c], 0, 0);
      child_SR.coste += gasto_SR;
      child_SR.h = Heuristica(child_SR.estado, final);
      child_SR.secuencia.push_back(TURN_SR);

      if (explored.find(child_SR.estado) == explored.end()) {
          frontier.push(child_SR);
      }

      // TURN_SL
      NodoT child_SL = current_node;
      child_SL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);

      int gasto_SL = CosteAccion(TURN_SL, terreno[current_node.estado.site.f][current_node.estado.site.c], 0, 0);
      child_SL.coste += gasto_SL;
      child_SL.h = Heuristica(child_SL.estado, final);
      child_SL.secuencia.push_back(TURN_SL);

      if (explored.find(child_SL.estado) == explored.end()) {
          frontier.push(child_SL);
      }
    }
  }
  return {};
}

/*
En este nivel, el tecnico traza un plan de tuberias en el que se gaste el menor gasto energetico
captura el estado actual y el objetivo (la Belkanita) para generar una lista de acciones de mínimo coste
ejecuta secuencialmente cada acción del plan almacenado
*/
/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  Action accion = IDLE;
  if (!hayPlan){
    EstadoT inicio, fin;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;
    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    plan = Busqueda_T(inicio, fin, mapaResultado, mapaCotas);
    VisualizaPlan(inicio.site, plan);
    hayPlan = (plan.size() != 0);
  }
  if( hayPlan && (plan.size() > 0) ){
    accion = plan.front();
    plan.pop_front();
  }
  if(plan.size() == 0){
    hayPlan = false;
  }

  return accion;
}

//--------------------------------------- NIVEL 4 -------------------------------------------
//-------------------------------------------------------------------------------------------

//en este nivel no tiene que hacer nada
/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}

//--------------------------------------- NIVEL 5 -------------------------------------------
//-------------------------------------------------------------------------------------------

/*
Esta funcion gestiona el comportamiento del tecnico como colaborador subordinado mediante una maquina de estados
*/
/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {

  //si el ingeniero llama al tecnico, actualizamos el destino y vamos
    if (sensores.venpaca) {
        destinoF = sensores.GotoF;
        destinoC = sensores.GotoC;
        accionesMov.clear();
        faseActual = EN_MARCHA;
        esperando_install = false;
    }

    if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

    //si se resetea, se queda quieto
    if (sensores.reset) {
        faseActual = PARADO;
        accionesMov.clear();
        esperando_install = false;
    }

    if (faseActual == PARADO) return IDLE;

    if (faseActual == EN_MARCHA) {
      //si hemos llegado a la casilla indicada por el ingeniero
        if (sensores.posF == destinoF && sensores.posC == destinoC) {
            faseActual = ORIENTANDOSE;
        } else {
            //si no hay plan o chocamos, recalculamos ruta
            if (accionesMov.empty() || sensores.choque) {
                EstadoT origen = {{sensores.posF, sensores.posC, sensores.rumbo}, tiene_zapatillas};
                EstadoT meta = {{destinoF, destinoC, norte}, tiene_zapatillas};
                accionesMov = Busqueda_T(origen, meta, mapaResultado, mapaCotas);

                //si el algoritmo A* falla, nos movemos usando los sensores
                if (accionesMov.empty()) {
                    ubicacion pF = Delante({sensores.posF, sensores.posC, sensores.rumbo});
                    ubicacion pI = Delante({sensores.posF, sensores.posC, (Orientacion)((sensores.rumbo+6)%8)});
                    ubicacion pD = Delante({sensores.posF, sensores.posC, (Orientacion)((sensores.rumbo+2)%8)});

                    bool frente_ok = (sensores.superficie[2] != 'M' && sensores.superficie[2] != 'P' &&
                                      sensores.superficie[2] != '?' &&
                                      abs(sensores.cota[2] - sensores.cota[0]) <= 1 &&
                                      sensores.agentes[2] == '_');

                    bool izq_ok = (sensores.superficie[1] != 'M' && sensores.superficie[1] != 'P' &&
                                  sensores.superficie[1] != '?' &&
                                  abs(sensores.cota[1] - sensores.cota[0]) <= 1 &&
                                  sensores.agentes[1] == '_');

                    bool der_ok = (sensores.superficie[3] != 'M' && sensores.superficie[3] != 'P' &&
                                  sensores.superficie[3] != '?' &&
                                  abs(sensores.cota[3] - sensores.cota[0]) <= 1 &&
                                  sensores.agentes[3] == '_');

                    // ir a zonas nuevas
                    if (frente_ok && !visitadas.count({pF.f, pF.c})) return WALK;
                    if (izq_ok && !visitadas.count({pI.f, pI.c})) return TURN_SL;
                    if (der_ok && !visitadas.count({pD.f, pD.c})) return TURN_SR;

                    // si todo visitado, moverse igual
                    if (frente_ok) return WALK;
                    if (izq_ok) return TURN_SL;
                    if (der_ok) return TURN_SR;

                    // último recurso
                    return TURN_SR;
                }
            }

            //ejecucion del plan
            if (!accionesMov.empty()) {
                Action sigAccion = accionesMov.front();
                //si el ingeniero esta delante, esperamos
                if (sigAccion == WALK && sensores.agentes[2] == 'i') return IDLE;
                
                accionesMov.pop_front();
                return sigAccion;
            }
        }
    }

    //sincronizacion
    if (faseActual == ORIENTANDOSE) {
      //el tecnico gira hasta encontrar al ingeniero
        if (sensores.enfrente && sensores.agentes[2] == 'i') {
            faseActual = PARADO;
            return INSTALL;
        }
        return TURN_SR;
    }

    return IDLE;
}

//--------------------------------------- NIVEL 6 -------------------------------------------
//-------------------------------------------------------------------------------------------

/*
Indica si tiene un obstaculo por donde no puede pasar, delante suya
*/
bool EsObstaculoDelante(Sensores s) {
    return s.superficie[2] == 'M' || s.superficie[2] == 'P';
}

/*
Mientras no reciba ordenes, investiga todo el mapa
Cuando lo llaman delega el control al comportamiento del nivel 5
(no me funciona)
*/
/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
    if (sensores.reset) {
        faseActual = PARADO;
        accionesMov.clear();
        visitadas.clear();
        destinoF = -1;
        destinoC = -1;
    }
    ActualizarMapa(sensores);
    if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

    //si nos llaman o o estamos yendo hacia el, ejecutamos la logica del nivel 5
    if (sensores.venpaca || faseActual != PARADO) {
        return ComportamientoTecnicoNivel_5(sensores);
    }

    //si no nos llaman: exploracion del mapa
    //se marca la posicion actual como visitada
    visitadas.insert({sensores.posF, sensores.posC});

    // comprobacion de altura y terreno
    //terreno
    char sup1 = sensores.superficie[1];
    char sup2 = sensores.superficie[2];
    char sup3 = sensores.superficie[3];
    //altura
    int dif1 = sensores.cota[1] - sensores.cota[0];
    int dif2 = sensores.cota[2] - sensores.cota[0];
    int dif3 = sensores.cota[3] - sensores.cota[0];

    //validamos si las casillas adyacentes son transitables
    bool frente_ok = (sup2 != 'M' && sup2 != 'P' && sup2 != '?' && abs(dif2) <= 1  && !sensores.choque && sensores.agentes[2] == '_');
    bool izq_ok = (sup1 != 'M' && sup1 != 'P' && sup1 != '?' && abs(dif1) <= 1 && sensores.agentes[1] == '_');
    bool der_ok = (sup3 != 'M' && sup3 != 'P' && sup3 != '?' && abs(dif3) <= 1 && sensores.agentes[3] == '_');

    if (!tiene_zapatillas) {
        frente_ok = frente_ok && (sup2 != 'B');
        izq_ok = izq_ok && (sup1 != 'B');
        der_ok = der_ok && (sup3 != 'B');
    }

    //si se encuentra con el ingeniero va hacia otro lado 
    if (sensores.agentes[2] == 'i') return TURN_SR;

    //casillas nuevas (no visitadas)
    ubicacion pF = Delante({sensores.posF, sensores.posC, sensores.rumbo});
    ubicacion pI = Delante({sensores.posF, sensores.posC, (Orientacion)((sensores.rumbo+7)%8)});
    ubicacion pD = Delante({sensores.posF, sensores.posC, (Orientacion)((sensores.rumbo+1)%8)});

    bool frente_nuevo = frente_ok && !visitadas.count({pF.f, pF.c});
    bool izq_nuevo = izq_ok && !visitadas.count({pI.f, pI.c});
    bool der_nuevo = der_ok && !visitadas.count({pD.f, pD.c});

    //toma de decisiones
    if (frente_nuevo) return WALK;
    if (izq_nuevo) return TURN_SL;
    if (der_nuevo) return TURN_SR;
    if (frente_ok) return WALK;

    //para salir de un posible bucle
    if (aleatorio(2) == 0) 
      return TURN_SR;
    else
      return TURN_SL;
}

// ------------------------------TUTORIAL-------------------------------
/*
ComportamientoTecnico::EstadoT NextCasillaTecnico(const ComportamientoTecnico::EstadoT &st){
  ComportamientoTecnico::EstadoT siguiente = st;
  switch(st.site.brujula){
    case norte:
      siguiente.site.f = st.site.f - 1;
      break;

    case noreste:
      siguiente.site.f = st.site.f - 1;
      siguiente.site.c = st.site.c + 1;
      break;

    case este:
        siguiente.site.c = st.site.c + 1;
      break;

    case sureste:
      siguiente.site.f = st.site.f + 1;
      siguiente.site.c = st.site.c + 1;
      break;

    case sur:
        siguiente.site.f = st.site.f + 1;
      break;

    case suroeste:
      siguiente.site.f = st.site.f + 1;
      siguiente.site.c = st.site.c - 1;
      break;

    case oeste:
        siguiente.site.c = st.site.c - 1;
      break;

    case noroeste:
      siguiente.site.f = st.site.f - 1;
      siguiente.site.c = st.site.c - 1;
      break;
  }

  return siguiente;
}

bool CasillaAccesibleTecnico(const ComportamientoTecnico::EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  ComportamientoTecnico::EstadoT next = NextCasillaTecnico(st);
  bool check1 = false, check2 = false, check3 = false;
  check1 = ( (terreno[next.site.f][next.site.c] != 'P') && (terreno[next.site.f][next.site.c] != 'M') );
  check2 = ( (terreno[next.site.f][next.site.c] != 'B') || ((terreno[next.site.f][next.site.c] != 'M') && st.zapatillas) );
  check3 = (abs( (altura[next.site.f][next.site.c] - altura[st.site.f][st.site.c])) <= 1);
  return ( (check1 && check2) && check3 );
}

ComportamientoTecnico::EstadoT applyT(Action accion, const ComportamientoTecnico::EstadoT &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  ComportamientoTecnico::EstadoT next = st;
  switch(accion){
    case WALK:
      if (CasillaAccesibleTecnico(st, terreno, altura)){
        next = NextCasillaTecnico(st);
      }
      break;

    case TURN_SR:
      next.site.brujula = (Orientacion) ((next.site.brujula+1)%8);
      break;
    
    case TURN_SL:
      next.site.brujula = (Orientacion)((next.site.brujula+7)%8);
      break;
  }

  return next;
}

bool Find(const ComportamientoTecnico::NodoT & st, const list<ComportamientoTecnico::NodoT> &lista){
  auto it = lista.begin();
  while( (it != lista.end()) && !((*it) == st)){
    it ++;
  }
  return (it != lista.end());
}


  /** 
   * @brief Primera aproximacion a la busqueda en anchura
   * 
   * @param inicio Estado Inicial de la busqueda.
   * @param final Estado Final de la busqueda.
   * @param terreno Matriz que contiene la informacion del terreno.
   * @param altura Matriz que contiene la altura del mapa
   * 
   * @return La secuencia de acciones para llegar al estado final
   * @note Devuelve un plan vacio si no es posible encontrar un plan valido
  

  list<Action> ComportamientoTecnico::B_Anchura(const EstadoT &inicio, const EstadoT &final, 
                                 const vector<vector<unsigned char>> &terreno, 
                                 const vector<vector<unsigned char>> &altura){
  NodoT current_node;
  list<NodoT> frontier;
  list<NodoT> explored;
  list<Action> path;

  current_node.estado = inicio;
  frontier.push_back(current_node);
  bool SolutionFound = ((current_node.estado.site.f == final.site.f) && (current_node.estado.site.c == final.site.c));

  while(!SolutionFound && !frontier.empty()){
    frontier.pop_front();
    explored.push_back(current_node);

    //Compruebo si estoy en una casilla que sa las zapatillas
    if(terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D'){
      current_node.estado.zapatillas = true;
    }

    NodoT child_Walk = current_node;
    child_Walk.estado = applyT(WALK, current_node.estado, terreno, altura);
    if( (child_Walk.estado.site.f == final.site.f) && (child_Walk.estado.site.c == final.site.c) ){
      //El hijo generado es solucion
      child_Walk.secuencia.push_back(WALK);
      current_node = child_Walk;
      SolutionFound = true;
    }
    else if( !Find(child_Walk, frontier) && !Find(child_Walk, explored) ){
      child_Walk.secuencia.push_back(WALK);
      frontier.push_back(child_Walk);
    }

    if(!SolutionFound){
      //El hijo resultante de aplicar la accion TURN_SR
      NodoT child_TurnSR = current_node;
      child_TurnSR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
      if( !Find(child_TurnSR, frontier) && !Find(child_TurnSR, explored) ){
        child_TurnSR.secuencia.push_back(TURN_SR);
        frontier.push_back(child_TurnSR);
      }
    }

    if(!SolutionFound){
      //El hijo resultante de aplicar la accion TURN_SL
      NodoT child_TurnSL = current_node;
      child_TurnSL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
      if( !Find(child_TurnSL, frontier) && !Find(child_TurnSL, explored) ){
        child_TurnSL.secuencia.push_back(TURN_SL);
        frontier.push_back(child_TurnSL);
      }
    }

    //Paso a evaluar el siguiente nodo en la lista "frontier"
    if( !SolutionFound and !frontier.empty()){
      current_node = frontier.front();
      SolutionFound = ( (current_node.estado.site.f == final.site.f) && (current_node.estado.site.c == final.site.c));
    }
  }

  if(SolutionFound){
    path = current_node.secuencia;
  }

  return path;
}

/** 
   * @brief Segunda aproximacion a la busqueda en anchura
   * 
   * @param inicio Estado Inicial de la busqueda.
   * @param final Estado Final de la busqueda.
   * @param terreno Matriz que contiene la informacion del terreno.
   * @param altura Matriz que contiene la altura del mapa
   * 
   * @return La secuencia de acciones para llegar al estado final
   * @note Devuelve un plan vacio si no es posible encontrar un plan valido
   * @note Explored pasa a ser implementado mediante un "set" eb lugar de un "list"
  
list<Action> ComportamientoTecnico::B_Anchura_V2(const EstadoT &inicio, const EstadoT &final, 
                                 const vector<vector<unsigned char>> &terreno, 
                                 const vector<vector<unsigned char>> &altura){
  NodoT current_node;
  list<NodoT> frontier;
  set<NodoT> explored;
  list<Action> path;

  current_node.estado = inicio;
  frontier.push_back(current_node);
  bool SolutionFound = ((current_node.estado.site.f == final.site.f) && (current_node.estado.site.c == final.site.c));

  while(!SolutionFound && !frontier.empty()){
    frontier.pop_front();
    explored.insert(current_node);

    //Compruebo si estoy en una casilla que sa las zapatillas
    if(terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D'){
      current_node.estado.zapatillas = true;
    }

    NodoT child_Walk = current_node;
    child_Walk.estado = applyT(WALK, current_node.estado, terreno, altura);
    if( (child_Walk.estado.site.f == final.site.f) && (child_Walk.estado.site.c == final.site.c) ){
      //El hijo generado es solucion
      child_Walk.secuencia.push_back(WALK);
      current_node = child_Walk;
      SolutionFound = true;
    }
    else if( explored.find(child_Walk) == explored.end() ){
      child_Walk.secuencia.push_back(WALK);
      frontier.push_back(child_Walk);
    }

    if(!SolutionFound){
      //El hijo resultante de aplicar la accion TURN_SR
      NodoT child_TurnSR = current_node;
      child_TurnSR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
      if( explored.find(child_TurnSR) == explored.end() ){
        child_TurnSR.secuencia.push_back(TURN_SR);
        frontier.push_back(child_TurnSR);
      }
    }

    if(!SolutionFound){
      //El hijo resultante de aplicar la accion TURN_SL
      NodoT child_TurnSL = current_node;
      child_TurnSL.estado = applyT(TURN_SL, current_node.estado, terreno, altura);
      if( explored.find(child_TurnSL) == explored.end() ){
        child_TurnSL.secuencia.push_back(TURN_SL);
        frontier.push_back(child_TurnSL);
      }
    }

    //Paso a evaluar el siguiente nodo en la lista "frontier"
    if( !SolutionFound and !frontier.empty()){
      current_node = frontier.front();
      while ( (explored.find(current_node) != explored.end()) && !frontier.empty() ){
        frontier.pop_front();
        current_node = frontier.front();
      }
    }
  }

  if(SolutionFound){
    path = current_node.secuencia;
  }

  return path;
}
*/

/**
 * @brief Comportamiento del técnico para el Nivel E (especial).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
/*
 Action ComportamientoTecnico::ComportamientoTecnicoNivel_E(Sensores sensores) {
  Action accion = IDLE;
  if (!hayPlan){
    //Invocar al metododo de busqueda
    EstadoT inicio, fin;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = tiene_zapatillas;
    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    plan = B_Anchura_V2(inicio, fin, mapaResultado, mapaCotas);
    VisualizaPlan(inicio.site, plan);
    hayPlan = (plan.size() != 0);
  }
  if( hayPlan && (plan.size() > 0) ){
    accion = plan.front();
    plan.pop_front();
  }
  if(plan.size() == 0){
    hayPlan = false;
  }

  return accion;
}
*/
// ------------------------------FIN DEL TUTORIAL-------------------------------


// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoTecnico::ActualizarMapa(Sensores sensores) {

  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo) {
    case norte:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
          mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
        }
      break;
    case noreste:
      mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
      mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
      mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
      mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
      mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
      mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
      mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
      mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
      mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
      mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
      mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
      mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
      mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
      mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
      mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
      mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
      mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
      mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
      mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
      mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
      mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
      mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
      mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
      mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
      mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
      mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
      mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
      mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
      mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
      mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
      break;
    case este:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
          mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
        }
      break;
    case sureste:
      mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
      mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
      mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
      mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
      mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
      mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
      mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
      mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
      mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
      mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
      mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
      mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
      mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
      mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
      mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
      mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
      mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
      mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
      mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
      mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
      mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
      mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
      mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
      mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
      mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
      mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
      mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
      mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
      mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
      mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
      break;
    case sur:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
          mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
        }
      break;
    case suroeste:
      mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
      mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
      mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
      mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
      mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
      mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
      mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
      mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
      mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
      mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
      mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
      mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
      mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
      mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
      mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
      mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
      mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
      mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
      mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
      mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
      mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
      mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
      mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
      mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
      mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
      mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
      mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
      mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
      mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
      mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
      break;
    case oeste:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
          mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
        }
      break;
    case noroeste:
      mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
      mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
      mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
      mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
      mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
      mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
      mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
      mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
      mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
      mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
      mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
      mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
      mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
      mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
      mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
      mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
      mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
      mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
      mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
      mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
      mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
      mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
      mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
      mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
      mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
      mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
      mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
      mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
      mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
      mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
      break;
  }
}



/**
 * @brief Determina si una casilla es transitable para el técnico.
 * En esta práctica, si el técnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) return false;
  return es_camino(mapaResultado[f][c]);  // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--; break;                        // norte
    case 1: delante.f--; delante.c++; break;     // noreste
    case 2: delante.c++; break;                     // este
    case 3: delante.f++; delante.c++; break;     // sureste
    case 4: delante.f++; break;                        // sur
    case 5: delante.f++; delante.c--; break;     // suroeste
    case 6: delante.c--; break;                     // oeste
    case 7: delante.f--; delante.c--; break;     // noroeste
  }
  return delante;
}


/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}



/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
                                            const list<Action> &plan)
{
   listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}

