#include "ingeniero.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoIngeniero::think(Sensores sensores)
{
  Action accion = IDLE;

  // Decisión del agente según el nivel
  switch (sensores.nivel)
  {
  case 0:
    accion = ComportamientoIngenieroNivel_0(sensores);
    break;
  case 1:
    accion = ComportamientoIngenieroNivel_1(sensores);
    break;
  case 2:
    accion = ComportamientoIngenieroNivel_2(sensores);
    break;
  case 3:
    accion = ComportamientoIngenieroNivel_3(sensores);
    break;
  case 4:
    accion = ComportamientoIngenieroNivel_4(sensores);
    break;
  case 5:
    accion = ComportamientoIngenieroNivel_5(sensores);
    break;
  case 6:
    accion = ComportamientoIngenieroNivel_6(sensores);
    break;
  }

  return accion;
}

//--------------------------------------- NIVEL 0 -------------------------------------------
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
int VerLejosI(const vector<unsigned char> &superficie, char objetivo, char i, char c, char d) {
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
int VeoCasillaInteresanteI(char i, char c, char d, bool zap, int nivel, bool ni, bool nc, bool nd) {
  
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

    // Camino y Sendero
    if (c == 'C' || c == 'S') return 2;
    else if (i == 'C' || i == 'S') return 1;
    else if (d == 'C' || d == 'S') return 3;
  }

  return 0;
}
/*
Comprueba la diferencia de altura entre la casilla del agente y la de destino.
Si la diferencia es <= 1, es viable.
Si tiene zapatillas, puede superar desniveles de <= 2 
Si no es viable, devuelve 'P'
*/
/**
 * @brief Determina si casilla viable por altura.
 * @param casilla tipo de terreno
 * @param dif diferencia de altura entre casillas
 * @param zap indica si estoy en posesión de las zapatillas
 * @return 'P' si no es accesible por altura y casilla en otro caso
 */
char ViablePorAlturaI( char casilla, int dif, bool zap){
  if (abs(dif) <= 1 or (zap and abs(dif) <= 2))
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
int VisitadoI(int f, int c, int brujula, const vector<vector<int>>& mapa) {
    int nf = f;
    int nc = c;

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

    if (nf >= 0 && nf < mapa.size() && nc >= 0 && nc < mapa[0].size()) {
        return mapa[nf][nc];
    }
    
    return 999999;
}

//Nivel 0: alcanzar una casilla de Tratamiento de Residuos ('U')
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_0(Sensores sensores)
{
  static vector<vector<int>> mapaVisitados;
  static int giros_180 = 0;
  
  if (mapaVisitados.empty() || sensores.reset) {
    mapaVisitados.assign(mapaResultado.size(), vector<int>(mapaResultado[0].size(), 0));
    tiene_zapatillas = false;
    giros_180 = 0; // contador de turnos restantes para completar media vuelta
  }

  // Si estamos en medio de un giro de 180, continuarlo
  if (giros_180 > 0) {
    giros_180--;
    last_action = TURN_SL;
    return TURN_SL;
  }

  Action accion = IDLE;
  ActualizarMapa(sensores); //pone en el mapa global lo q ve el agente

  // registra la pos actual en la memoria de visitas para evitar bucles
  if (sensores.posF != -1) mapaVisitados[sensores.posF][sensores.posC]++;

  // comprueba el estado actual: exito (U) o zapatillas (D)
  if (sensores.superficie[0] == 'U') return IDLE;
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  // reaccion ante choque
  if (sensores.choque) {
    if (aleatorio(1) == 0) return TURN_SL;
    else return TURN_SR;
  }

  // Si el tecnico esta justo delante, iniciar giro de 180
  if (sensores.agentes[2] == 't') {
    giros_180 = 3;
    last_action = TURN_SL;
    return TURN_SL;
  }

  char i, c, d;
  // evalua la viabilidad de las 3 casillas frontales considerando desniveles y agentes
  if (sensores.agentes[1] == '_')
    i = ViablePorAlturaI(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  else i = 'P';

  if (sensores.agentes[2] == '_')
    c = ViablePorAlturaI(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  else c = 'P';

  if (sensores.agentes[3] == '_')
    d = ViablePorAlturaI(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);
  else d = 'P';

  // filtrado de terreno
  if (i != 'C' && i != 'D' && i != 'U') i = 'P';
  if (c != 'C' && c != 'D' && c != 'U') c = 'P';
  if (d != 'C' && d != 'D' && d != 'U') d = 'P';

  if (sensores.agentes[1] != '_') i = 'P';
  if (sensores.agentes[2] != '_') c = 'P';
  if (sensores.agentes[3] != '_') d = 'P';

  // 1. miramos si hay U o D 
  int pos = VeoCasillaInteresanteI(i, c, d, tiene_zapatillas, 0, false, false, false);

  // 2. si no hay objetivos, elegimos el camino menos visitado
  if (pos == 0) {
    int v_i = (i != 'P') ? VisitadoI(sensores.posF, sensores.posC, (sensores.rumbo + 7) % 8, mapaVisitados) : 999999;
    int v_c = (c != 'P') ? VisitadoI(sensores.posF, sensores.posC, sensores.rumbo, mapaVisitados) : 999999;
    int v_d = (d != 'P') ? VisitadoI(sensores.posF, sensores.posC, (sensores.rumbo + 1) % 8, mapaVisitados) : 999999;

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
  if (pos == 0) pos = VerLejosI(sensores.superficie, 'U', i, c, d);
  if (pos == 0 && !tiene_zapatillas) pos = VerLejosI(sensores.superficie, 'D', i, c, d);

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
/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoIngeniero::es_camino(unsigned char c) const
{
  return (c == 'C' || c == 'D' || c == 'U');
}

//Nivel 1: mapear la mayor superficie posible del entorno desconocido
/**
 * @brief Comportamiento reactivo del ingeniero para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_1(Sensores sensores) {
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
    last_action = TURN_SL;
    return TURN_SL;
  }

  ActualizarMapa(sensores); //rellena el mapa con '?' y terrenos reales
  if (sensores.posF != -1) mapaVisitados[sensores.posF][sensores.posC]++;
  if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

  //ver si se puede ir por las casillas adyacentes
  char i = ViablePorAlturaI(sensores.superficie[1], sensores.cota[1] - sensores.cota[0], tiene_zapatillas);
  char c = ViablePorAlturaI(sensores.superficie[2], sensores.cota[2] - sensores.cota[0], tiene_zapatillas);
  char d = ViablePorAlturaI(sensores.superficie[3], sensores.cota[3] - sensores.cota[0], tiene_zapatillas);

  // Caminos, Senderos, Zapatillas y Tuberías
  if (i != 'C' && i != 'S' && i != 'D' && i != 'U') i = 'P';
  if (c != 'C' && c != 'S' && c != 'D' && c != 'U') c = 'P';
  if (d != 'C' && d != 'S' && d != 'D' && d != 'U') d = 'P';

  //deteccion de colision frontal con el otro agente
  if (sensores.agentes[2] == 't') {
    giros_180 = 3; 
    last_action = TURN_SL;
    return TURN_SL;
  }

  int seleccion = 0;

  //consulta de memoria: obtiene cuantas visitas tienen las opciones izquierda, frente y derecha
  int v_i = VisitadoI(sensores.posF, sensores.posC, (sensores.rumbo + 7) % 8, mapaVisitados);
  int v_c = VisitadoI(sensores.posF, sensores.posC, sensores.rumbo, mapaVisitados);
  int v_d = VisitadoI(sensores.posF, sensores.posC, (sensores.rumbo + 1) % 8, mapaVisitados);

  int min_v = 999999;

  // selecciona la dirección menos transitada para fomentar la exploración
  if (c != 'P') {
    min_v = v_c;
    seleccion = 2;
  }

  // Izquierda es menos visitada, cambiamos
  if (i != 'P' && v_i < min_v) {
    min_v = v_i;
    seleccion = 1;
  }

  // derecha es aún menos visitada, cambiamos
  if (d != 'P' && v_d < min_v) {
    min_v = v_d;
    seleccion = 3;
  }

  // si la zona cercana está muy visitada, buscamos '?' lejanos
  if (seleccion == 0 || min_v > 5) {
      int seleccion_interesante = VeoCasillaInteresanteI(i, c, d, tiene_zapatillas, 1, true, true, true);
      if (seleccion_interesante != 0) seleccion = seleccion_interesante;
  }

  // si no hay opciones claras, forzar giro
  if (seleccion == 0) seleccion = 1;

  Action accion = IDLE;
  switch (seleccion) {
    case 2: 
      if (sensores.agentes[2] == '_') accion = WALK;
      else accion = TURN_SR; 
      break;
    case 1: accion = TURN_SL; break;
    case 3: accion = TURN_SR; break;
    default: accion = TURN_SL; break;
  }

  last_action = accion;
  return accion;
}

//--------------------------------------- NIVEL 2 -------------------------------------------
/*
calcular cuáles serían las coordenadas (fila, columna) de la casilla que está justo delante del agente, 
dependiendo de hacia dnde esté mirando (su brújula)

Entrada: Un estado con una posición y una orientación.
Salida: Un nuevo estado con las coordenadas actualizadas, pero manteniendo la misma orientación.
*/
ComportamientoIngeniero::EstadoI NextCasillaIngeniero(const ComportamientoIngeniero::EstadoI &st){
  ComportamientoIngeniero::EstadoI siguiente = st;
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
decide si una acción es legal o si el ingeniero se chocaría contra un muro o intentaria un salto imposible
*/
bool CasillaAccesibleIngeniero(const ComportamientoIngeniero::EstadoI &st, Action accion, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
    ComportamientoIngeniero::EstadoI next1 = NextCasillaIngeniero(st);
    
    //comprueba que la casilla de enfrente esta en la matriz
    if (next1.site.f < 0 || next1.site.f >= terreno.size() || next1.site.c < 0 || next1.site.c >= terreno[0].size()) return false;

    //si la accion que se quiere realizar es saltar:
    if (accion == JUMP) {
      //validacion de la casilla intermedia (terreno por el que se pueda pasar)
        if (terreno[next1.site.f][next1.site.c] == 'M' || 
            terreno[next1.site.f][next1.site.c] == 'P' ||
            terreno[next1.site.f][next1.site.c] == 'B') return false;

        //validacion de la casilla a la que se quiere llegar (dentro del mapa, altura correcta)
        ComportamientoIngeniero::EstadoI next2 = NextCasillaIngeniero(next1);
        if (next2.site.f < 0 || next2.site.f >= terreno.size() || next2.site.c < 0 || next2.site.c >= terreno[0].size()) return false;

        bool t_ok = (terreno[next2.site.f][next2.site.c] != 'P' && 
                     terreno[next2.site.f][next2.site.c] != 'M' &&
                     terreno[next2.site.f][next2.site.c] != 'B');

        int limite = (st.zapatillas) ? 2 : 1;
        bool a_ok = (abs((int)altura[next2.site.f][next2.site.c] - 
                         (int)altura[st.site.f][st.site.c]) <= limite);

        return (t_ok && a_ok);
    }

    //si la accion que se quiere realizar es andar:
    if (accion == WALK) {
      //comprueba el terreno que sea accesible
        bool t_ok = (terreno[next1.site.f][next1.site.c] != 'P' && 
                     terreno[next1.site.f][next1.site.c] != 'M' && 
                     terreno[next1.site.f][next1.site.c] != 'B');

        //comprueba altura
        int limite = (st.zapatillas) ? 2 : 1;
        bool a_ok = (abs((int)altura[next1.site.f][next1.site.c] - (int)altura[st.site.f][st.site.c]) <= limite);

        return (t_ok && a_ok);
    }

    return true;
}

/*
generar un nuevo estado (next) a partir de un estado actual (st) y una accion. 
Lo usan el BFS y A* para imaginar el mapa y construir el arbol de busqueda.
Va comprobando si puede realizar esas acciones 
*/
ComportamientoIngeniero::EstadoI applyI(Action accion, const ComportamientoIngeniero::EstadoI &st, const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura){
  ComportamientoIngeniero::EstadoI next = st;
  switch(accion){
    case WALK:
      if (CasillaAccesibleIngeniero(st, WALK, terreno, altura)){
        next = NextCasillaIngeniero(st);
      }
      break;

    case JUMP:
      if( CasillaAccesibleIngeniero(st, JUMP, terreno, altura)){
        next = NextCasillaIngeniero(st);
        next = NextCasillaIngeniero(next);
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
verifica si un estado especifico ya ha sido procesado o se encuentra en el conjunto de nodos explorados
*/
bool FindI2(const ComportamientoIngeniero::NodoI & st, const set<ComportamientoIngeniero::NodoI> &lista){
  return lista.find(st) != lista.end();
}

/*
funcion busqueda de caminos para el nivel 2 del ingeniero
*/
  list<Action> ComportamientoIngeniero::Busqueda_I(const EstadoI &inicio, const EstadoI &final, 
                                 const vector<vector<unsigned char>> &terreno, 
                                 const vector<vector<unsigned char>> &altura){
  NodoI current_node;
  list<NodoI> frontier; //nodos por explorar
  set<NodoI> explored; //nodos ya visitados
  list<Action> path;

  current_node.estado = inicio;
  frontier.push_back(current_node);

  //comprobamos si estamos ya en la meta
  bool SolutionFound = (current_node.estado.site.f == final.site.f && 
                        current_node.estado.site.c == final.site.c);

  while (!SolutionFound && !frontier.empty()) {
    frontier.pop_front(); //sacamos el primer nodo de la lista
    explored.insert(current_node); //marcamos el estado actual como visitado

    //hijos: generamos los estados resultantes de cada acción posible

    // Hijo WALK
    NodoI child_Walk = current_node;
    child_Walk.estado = applyI(WALK, current_node.estado, terreno, altura);
    //si el hijo pisa zapatillas, su estado futuro cambia
    if (terreno[child_Walk.estado.site.f][child_Walk.estado.site.c] == 'D'){
      child_Walk.estado.zapatillas = true;
    }
    bool solucionEncontrada = (child_Walk.estado.site.f == final.site.f && child_Walk.estado.site.c == final.site.c);
    if (solucionEncontrada) {
      child_Walk.secuencia.push_back(WALK);
      current_node = child_Walk;
      SolutionFound = true;
    }
    else if (!FindI2(child_Walk, explored)) { // añadimos a la cola si no ha sido explorado
      child_Walk.secuencia.push_back(WALK);
      frontier.push_back(child_Walk);
    }

    // Hijo JUMP (igual que walk)
    if (!SolutionFound) {
      NodoI child_Jump = current_node;
      child_Jump.estado = applyI(JUMP, current_node.estado, terreno, altura);
      if (terreno[child_Jump.estado.site.f][child_Jump.estado.site.c] == 'D'){
        child_Jump.estado.zapatillas = true;
      }
      if (child_Jump.estado.site.f == final.site.f && child_Jump.estado.site.c == final.site.c) {
        child_Jump.secuencia.push_back(JUMP);
        current_node = child_Jump;
        SolutionFound = true;
      }
      else if (!FindI2(child_Jump, explored)) {
        child_Jump.secuencia.push_back(JUMP);
        frontier.push_back(child_Jump);
      }
    }

    //hijos de giros
    if (!SolutionFound) {
      // Hijo TURN_SR
      NodoI child_TurnSR = current_node;
      child_TurnSR.estado = applyI(TURN_SR, current_node.estado, terreno, altura);
      if (!FindI2(child_TurnSR, explored)) {
        child_TurnSR.secuencia.push_back(TURN_SR);
        frontier.push_back(child_TurnSR);
      }

      // Hijo TURN_SL
      NodoI child_TurnSL = current_node;
      child_TurnSL.estado = applyI(TURN_SL, current_node.estado, terreno, altura);
      if (!FindI2(child_TurnSL, explored)) {
        child_TurnSL.secuencia.push_back(TURN_SL);
        frontier.push_back(child_TurnSL);
      }
    }

    // Seleccion del siguiente nodo a expandir, orden de llegada = Anchura
    if (!SolutionFound && !frontier.empty()) {
      current_node = frontier.front();
      while (FindI2(current_node, explored) && !frontier.empty()) {
        frontier.pop_front();
        current_node = frontier.front();
      }
    }
  }

  if (SolutionFound)
    path = current_node.secuencia;
  return path;
}
/*
 piensa en todas las posibilidades y traza la ruta perfecta (la más corta en pasos) antes de moverse
*/
 /**
 * @brief Comportamiento del ingeniero para el Nivel 2 (búsqueda).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_2(Sensores sensores)
{
  Action accion = IDLE;

  //fase 1: si no hay plan calculado o si el anterior ha terminado
  if (!hayPlan) {
    //estado inicial
    EstadoI inicio;
    inicio.site.f = sensores.posF;
    inicio.site.c = sensores.posC;
    inicio.site.brujula = sensores.rumbo;
    inicio.zapatillas = (mapaResultado[sensores.posF][sensores.posC] == 'D' || tiene_zapatillas);
    
    //objetivo: belkanita
    EstadoI fin;
    fin.site.f = sensores.BelPosF;
    fin.site.c = sensores.BelPosC;
    
    //generamos la lista de acciones (el plan)
    plan = Busqueda_I(inicio, fin, mapaResultado, mapaCotas);
    VisualizaPlan(inicio.site, plan);
    //si hubo exito, activamos hayPlan para empezar a movernos
    hayPlan = (plan.size() != 0);
  }

  //fase 2: ejecutamos una a una las acciones del plan
  if (hayPlan && plan.size() > 0) {
    accion = plan.front();

    if (accion == WALK && sensores.agentes[2] == 't') return IDLE;
    if (accion == JUMP && sensores.agentes[6] == 't') return IDLE;

    //si el camino esta libre sacamos la accion de la lista para ejecutarla
    plan.pop_front();
  }

  //fase 3: si ya hemos acabado, desactivamos hayPlan
  if (plan.size() == 0) hayPlan = false;

  return accion;
}

//--------------------------------------- NIVEL 3 -------------------------------------------
/*
En este nivel el ingeniero lo que hace es quitarse del camino del tecnico
*/
/**
 * @brief Comportamiento del ingeniero para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_3(Sensores sensores)
{
  Action accion = IDLE;

  //si el otro agente esta delante giramos
  if (sensores.agentes[2] == 't') {
    return TURN_SR;
  }

  //para ver si el tecnico esta a la vista
  bool tecnico_a_la_vista = false;
  for (int i = 1; i <= 15; i++) {
    if (sensores.agentes[i] == 't') {
      tecnico_a_la_vista = true;
      break;
    }
  }

  //solo nos movemos si el tecnico está cerca
  if (tecnico_a_la_vista) {
    unsigned char delante = sensores.superficie[2];
    int desnivel = abs((int)sensores.cota[2] - (int)sensores.cota[0]);
    int max_salto = (tiene_zapatillas) ? 2 : 1;

    //condiciones que se deben cumplir para poder avanzar (terreno, casilla libre y altura)
    bool puedo_avanzar = (delante != 'P' && delante != 'M' && delante != 'B' && 
                          sensores.agentes[2] == '_' && desnivel <= max_salto);

    if (puedo_avanzar) {
      accion = WALK; 
    } else {
      accion = TURN_SR; 
    }
   
  //si no esta a la vista nos quedamos quietos
  } else {
    accion = IDLE;
  }

  return accion;
}

//--------------------------------------- NIVEL 4 -------------------------------------------
/*
Calcula el coste ecologico de cada accion (tabla de coste ecologico dig, raise, install)
*/
int Impacto_ecologico(Action accion, unsigned char casilla_ini){
  int impacto = 0;
  if( accion == INSTALL ){
    if(casilla_ini == 'A'){
      impacto += 50;
    } else if(casilla_ini == 'H'){
      impacto += 45;
    } else if(casilla_ini == 'S'){
      impacto += 25;
    } else if((casilla_ini == 'C') || (casilla_ini == 'U')){
      impacto += 15;
    } else{
      impacto += 30;
    }
  }else if (accion == RAISE) {
    if(casilla_ini == 'H'){
      impacto += 55;
    } else if(casilla_ini == 'S'){
      impacto += 30;
    } else if((casilla_ini == 'C') || (casilla_ini == 'U')){
      impacto += 10;
    } else{
      impacto += 40;
    }
  } else if( accion == DIG ){
    if(casilla_ini == 'H'){
      impacto += 65;
    } else if(casilla_ini == 'S'){
      impacto += 40;
    } else if((casilla_ini == 'C') || (casilla_ini == 'U')){
      impacto += 25;
    } else{
      impacto += 50;
    }
  }

  return impacto;
}
 
/*
Calcula el coste de cada accion (tabla de coste accion)
*/
int CosteAccionI(Action accion, unsigned char casilla_ini, int alt_ini, int alt_fin){
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
  }else if( accion == JUMP){
    if(casilla_ini == 'A'){
      coste = 90 + mod_altura;
    }else if( casilla_ini == 'H'){
      coste = 10 + mod_altura;
    }else if( casilla_ini == 'S' ){
      coste = 4 + mod_altura;
    }else{
      coste = 3 + mod_altura;
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
  }else if(accion == INSTALL){
    if(casilla_ini == 'A'){
      coste = 60;
    }else if( casilla_ini == 'H'){
      coste = 45;
    }else if( casilla_ini == 'S' ){
      coste = 25;
    }else if( (casilla_ini == 'C') || (casilla_ini == 'U')){
      coste = 15;
    }else{
      coste = 30;
    }
  }else if (accion == RAISE){
    if( casilla_ini == 'H'){
      coste = 55;
    }else if( casilla_ini == 'S' ){
      coste = 30;
    }else if( (casilla_ini == 'C') || (casilla_ini == 'U')){
      coste = 10;
    }else{
      coste = 40;
    }
  }else if(accion == DIG){
    if( casilla_ini == 'H'){
      coste = 65;
    }else if( casilla_ini == 'S' ){
      coste = 40;
    }else if( (casilla_ini == 'C') || (casilla_ini == 'U')){
      coste = 25;
    }else{
      coste = 50;
    }
  }

  return coste;
}

/*
Calcula la Distancia Manhattan minima entre la posicion actual y el conjunto de todas las 'U' disponibles en el mapa
*/
int calcularH(int f, int c, const vector<pair<int, int>>& listaU) {
  int min_dist = 999999;

  //para cada planta, calcula la distancia que hay de ella al agente
  for (const auto& planta : listaU) {
    int dist = abs(f - planta.first) + abs(c - planta.second);
    if (dist < min_dist) 
      min_dist = dist;
  }
  //devuelve la menor distancia de todas (la planta mas cercana)
  return min_dist;
}

/*
Comprueba que la gravedad se cumpla, la belkanita solo puede bajar y una casilla
*/
bool Gravedad(int altura_ini, int altura_fin){
  int dif = altura_ini - altura_fin;
  if(dif == 0 || dif == 1){
    return true;
  }else{
    return false;
  }
}

/*
El objetivo es encontrar un trazado de tuberias eficiente desde la belkanita hasta la 'U'
Emplea un algoritmo A* (impacto ecológico y energético)
Los estados son secuencias de construccion
Usa la heuristica de distancia Manhattan
TIene en cuenta la gravedad y los presupuestos ecologicos
*/
list<ComportamientoIngeniero::Paso> ComportamientoIngeniero::Busqueda_Plan_PasoI(Sensores sensores){
                    
  //cola con prioridad para explorar primero el nodo con menor (f = g + h)
  priority_queue<NodoPaso, vector<NodoPaso>, ComparaCoste> frontier;
  map<EstadoPaso, int> explored;

  int max_energia = sensores.energia;
  int max_impacto = sensores.max_ecologico;

  //identificamos todas las plantas 'U' para la heurística
  vector<pair<int, int>> lista_plantas_U;
  for (int f = 0; f < mapaResultado.size(); f++) {
    for (int c = 0; c < mapaResultado[f].size(); c++) {
      if (mapaResultado[f][c] == 'U') {
        lista_plantas_U.push_back({f, c});
      }
    }
  }
  
  //Configuracion del nodo inicial
  unsigned char casilla_origen = mapaResultado[sensores.BelPosF][sensores.BelPosC];
  
  //probamos las 3 acciones en la casilla (dig, raise, install)
  for (int op_ini = -1; op_ini <= 1; op_ini++) {
    //calculo de la altura
    int altura_bel = mapaCotas[sensores.BelPosF][sensores.BelPosC] + op_ini;

    if (altura_bel < 0 || altura_bel > 9) continue;
    if (casilla_origen == 'A' && op_ini != 0) continue;

    //estado
    NodoPaso inicio;
    EstadoPaso es;
    es.f = sensores.BelPosF;
    es.c = sensores.BelPosC;
    es.altura = altura_bel;
    inicio.estado = es;

    //secuencia
    Paso primer_paso;
    primer_paso.fil = sensores.BelPosF;
    primer_paso.col = sensores.BelPosC;
    primer_paso.op = op_ini;
    inicio.secuencia.push_back(primer_paso);

    //energia e impacto
    int c_ini = 0;
    int i_ini = 0;

    if (op_ini == -1) { // DIG
      c_ini += CosteAccionI(DIG, casilla_origen, mapaCotas[es.f][es.c], altura_bel);
      i_ini += Impacto_ecologico(DIG, casilla_origen);
    } else if (op_ini == 1) { // RAISE
      c_ini += CosteAccionI(RAISE, casilla_origen, mapaCotas[es.f][es.c], altura_bel);
      i_ini += Impacto_ecologico(RAISE, casilla_origen);
    }

    // INSTALL
    c_ini += CosteAccionI(INSTALL, casilla_origen, altura_bel, altura_bel);

    inicio.energia = c_ini;
    inicio.impacto = i_ini;

    //f = g + h 'g' es la profundidad (pasos) y 'h' es la distancia estimada a la meta
    if (inicio.energia <= max_energia && inicio.impacto <= max_impacto) {
      inicio.g = 1;
      inicio.h = calcularH(es.f, es.c, lista_plantas_U); // distancia Manhattan
      frontier.push(inicio);
    }
  }
  //fin nodo inicial


  //bucle de busqueda
  while(!frontier.empty()){
    NodoPaso current_node = frontier.top();
    frontier.pop();
    
    //cond de exito
    if (mapaResultado[current_node.estado.f][current_node.estado.c] == 'U'){
      return current_node.secuencia;
    }

    // solo exploramos si no hemos pasado por aquí o si este camino es más ecológico
    if (explored.find(current_node.estado) == explored.end() || current_node.impacto < explored[current_node.estado]){
      explored[current_node.estado] = current_node.impacto;

      int df[] = {-1, 1, 0, 0};
      int dc[] = { 0, 0, 1, -1};

      for (int i = 0; i < 4; i++) {
          int sig_f = current_node.estado.f + df[i];
          int sig_c = current_node.estado.c + dc[i];

          if (sig_f >= 0 && sig_f < mapaResultado.size() && sig_c >= 0 && sig_c < mapaResultado[0].size()) {
            
            //verificacion de limites y de terreno
            unsigned char casilla_destino = mapaResultado[sig_f][sig_c];
            if ((casilla_destino != 'M' && casilla_destino != 'P') && casilla_destino != 'B'){

              // Para cada casilla vecina, probamos DIG, NADA o RAISE
              for (int op = -1; op <= 1; op++) {
                if (casilla_destino == 'A' && op != 0) continue; 

                int altura_final = mapaCotas[sig_f][sig_c] + op;
                
                // gravedad y limite de altura
                if (Gravedad(current_node.estado.altura, altura_final) && altura_final >= 0 && altura_final <= 9) {
                    int altura_dest = mapaCotas[sig_f][sig_c];
                    int coste = 0;
                    int impacto = 0;

                    if (op == -1) { // DIG
                      coste += CosteAccionI(DIG, casilla_destino, altura_dest, altura_final);
                      impacto += Impacto_ecologico(DIG, casilla_destino);
                    } else if (op == 1) { // RAISE
                      coste += CosteAccionI(RAISE, casilla_destino, altura_dest, altura_final);
                      impacto += Impacto_ecologico(RAISE, casilla_destino);
                    }

                    // INSTALL
                    unsigned char casilla_actual = mapaResultado[current_node.estado.f][current_node.estado.c];
                    coste += CosteAccionI(INSTALL, casilla_destino, altura_final, altura_final);
                    impacto += Impacto_ecologico(INSTALL, casilla_actual);
                    impacto += Impacto_ecologico(INSTALL, casilla_destino);

                    if ( (current_node.energia + coste <= max_energia) && (current_node.impacto + impacto <= max_impacto) ){
                      EstadoPaso hijo_e;
                      hijo_e.f = sig_f; hijo_e.c = sig_c; hijo_e.altura = altura_final;
                      
                      if (explored.find(hijo_e) == explored.end() || (current_node.impacto + impacto) < explored[hijo_e]){
                        NodoPaso hijo;
                        hijo.estado = hijo_e;
                        hijo.g = current_node.g + 1;
                        hijo.h = calcularH(sig_f, sig_c, lista_plantas_U);
                        hijo.energia = current_node.energia + coste;
                        hijo.impacto = current_node.impacto + impacto;

                        hijo.secuencia = current_node.secuencia;
                        Paso p = {sig_f, sig_c, op};
                        hijo.secuencia.push_back(p);

                        frontier.push(hijo);
                      }
                    }
                  }
                }
            }
          }
      }
    }
  }

  return {};
}

/*
el ingeniero en este nivel diseña una red de tuberias
*/
/**
 * @brief Comportamiento del ingeniero para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_4(Sensores sensores)
{
  if (!hayPlan) {
    planTuberias = Busqueda_Plan_PasoI(sensores);
    VisualizaRedTuberias(planTuberias);
    hayPlan = true; 
  }
  
  return IDLE;
}
//--------------------------------------- NIVEL 5 -------------------------------------------

/*
Es una implementacion de la Busqueda en Anchura adaptada para encontrar el camino más corto en número de acciones (pasos y giros) entre dos puntos del mapa
*/
list<Action> ComportamientoIngeniero::BuscarCaminoBFS(int fO, int cO, int rO, int fD, int cD) {
  // definicion de un estado: posicion y orientacion  
  struct State { 
        int f, c, r; 
        bool operator<(const State &o) const { 
            if (f!=o.f) return f < o.f; 
            if (c!=o.c) return c < o.c; 
            return r < o.r; 
        } 
    };

    queue<pair<State, list<Action>>> q; //cola de exploracion
    set<State> vis; //mapa de visitados
    //punto de origen con un camino vacío
    q.push({{fO, cO, rO}, {}});

    while (!q.empty()) {
      //extraemos el primer elemento de la cola
        auto [curr, path] = q.front(); 
        q.pop();

        //cond exito: posicipn coincide con el destino
        if (curr.f == fD && curr.c == cD) return path;

        //si ya lo hemos visitado, se ignora
        if (vis.count(curr)) continue;
        vis.insert(curr);

        //Generacion de hijos
        // Giro a la izquierda
        State sl = curr; sl.r = (sl.r + 7) % 8;
        auto psl = path; psl.push_back(TURN_SL);
        q.push({sl, psl});

        // Giro a la derecha
        State sr = curr; sr.r = (sr.r + 1) % 8;
        auto psr = path; psr.push_back(TURN_SR);
        q.push({sr, psr});

        // WALK
        //vectores de movimiento para las 8 direcciones posibles
        int df[] = {-1,-1,0,1,1,1,0,-1};
        int dc[] = {0,1,1,1,0,-1,-1,-1};
        int nf = curr.f + df[curr.r];
        int nc = curr.c + dc[curr.r];

        //casilla esta dentro de los límites
        if (nf >= 0 && nf < (int)mapaResultado.size() && nc >= 0 && nc < (int)mapaResultado[0].size()) {
            char t = mapaResultado[nf][nc]; //terreno
            int dif = abs((int)mapaCotas[nf][nc] - (int)mapaCotas[curr.f][curr.c]); //dif altura
            bool h_ok = tiene_zapatillas ? dif <= 2 : dif <= 1; //vemos si ingeniero puede superar el desnivel
            //si es transitable añadimos el estado
            if (t != 'P' && t != 'M' && t != 'B' && h_ok) {
                State ws = {nf, nc, curr.r};
                auto wp = path; wp.push_back(WALK);
                q.push({ws, wp});
            }
        }
    }
    return {};
}
/*
en este nivel usamos una maquina de estados para llevar a cabo el plan trazado paso a paso
Pasos:
  Planificar la red (A*).
  Desplazarse a la siguiente casilla del tramo (BFS).
  Nivelar el terreno (DIG/RAISE).
  Orientarse hacia donde viene el Técnico.
  Llamar y esperar al Técnico para realizar la instalación conjunta (INSTALL).
*/
/**
 * @brief Comportamiento del ingeniero para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_5(Sensores sensores) {
    if (sensores.reset) {
        planCalculado = false;
        estadoActual = BUSCANDO_PLAN;
        listaTuberias.clear();
        listaMovimiento.clear();
        ultimaF = -1;
        ultimaC = -1;
    }

    ActualizarMapa(sensores);
    if (sensores.superficie[0] == 'D') tiene_zapatillas = true;

    //fase 1: planificacion
    if (!planCalculado) {
        listaTuberias = Busqueda_Plan_PasoI(sensores); //red de todas las tuberias
        if (!listaTuberias.empty()) {
            VisualizaRedTuberias(listaTuberias);
            planCalculado = true;
            estadoActual = MOVIENDOSE;
        }
        return IDLE;
    }

    if (listaTuberias.empty()) return IDLE;
    Paso tramoActual = listaTuberias.front(); //tramo de la tuberia en la que se trabaja ahora

    //maquina de estados
    switch (estadoActual) {
        case MOVIENDOSE: {
          //si ya estamos en la casilla donde va la tuberia
            if (sensores.posF == tramoActual.fil && sensores.posC == tramoActual.col) {
                listaMovimiento.clear();
                //nivelamos el terreno si hace falta
                if (tramoActual.op == 1)  { listaTuberias.front().op = 0; return RAISE; }
                if (tramoActual.op == -1) { listaTuberias.front().op = 0; return DIG; }
                
                //decidir si hay que orientarse hacia el tramo anterior
                if (ultimaF != -1) {
                    estadoActual = ORIENTANDOSE;
                } else {
                    estadoActual = LLAMANDO_TECNICO;
                }
                return IDLE;
            }

            //si no estamos en la casilla, usamos BFS para encontrar el camino más corto hasta ella
            if (listaMovimiento.empty()) {
                listaMovimiento = BuscarCaminoBFS(sensores.posF, sensores.posC, sensores.rumbo, tramoActual.fil, tramoActual.col);
            }

            //ejecucion de los pasos del bfs sin chocarnos contra el tecnico
            if (!listaMovimiento.empty()) {
                Action sig = listaMovimiento.front();
                if (sig == WALK && sensores.agentes[2] == 't') return IDLE;
                listaMovimiento.pop_front();
                return sig;
            }
            return IDLE;
        }

        case LLAMANDO_TECNICO: { //registramos la posición actual como ultima para el siguiente tramo
            ultimaF = sensores.posF;
            ultimaC = sensores.posC;
            listaTuberias.pop_front();
            if (listaTuberias.empty()) return IDLE;
            listaMovimiento.clear();
            estadoActual = MOVIENDOSE; //pasamos al siguiente punto del plan
            return COME; //llamamos al tecnico para que venga a nuestra posicion
        }

        case ORIENTANDOSE: { // calculamos la direccion hacia la que debe mirar el ingeniero para recibir al tecnico
            int df = ultimaF - sensores.posF;
            int dc = ultimaC - sensores.posC;
            Orientacion objetivo;
            if      (df == -1) objetivo = norte;
            else if (df ==  1) objetivo = sur;
            else if (dc ==  1) objetivo = este;
            else               objetivo = oeste;

            if (sensores.rumbo == objetivo) {
                estadoActual = ESPERANDO_TECNICO;
                return IDLE;
            }
            // giramos por el camino mas corto
            int dr = ((int)objetivo - (int)sensores.rumbo + 8) % 8;
            return (dr <= 4) ? TURN_SR : TURN_SL;
        }

        case ESPERANDO_TECNICO: // si el tecnico ya esta enfrente, realizamos la instalacion.
            if (sensores.enfrente && sensores.agentes[2] == 't') {
                listaMovimiento.clear();
                ultimaF = sensores.posF;
                ultimaC = sensores.posC;

                //si quedan mas tuberias, seguimos, si no, volvemos a buscar plan
                if (!listaTuberias.empty()) {
                    estadoActual = LLAMANDO_TECNICO;
                } else {
                    estadoActual = BUSCANDO_PLAN;
                }
                return INSTALL;
            }
            return IDLE;

        default:
            return IDLE;
    }
}

//--------------------------------------- NIVEL 6 -------------------------------------------
/*
determinar si el agente ha explorado lo suficiente como para empezar a tomar decisiones basadas en el mapa global o si debe seguir explorando
*/
bool hay_suficiente_mapa(vector<vector<unsigned char>> &mapaResultado) {
    int visibles = 0;

    for (int i = 0; i < mapaResultado.size(); i++) {
        for (int j = 0; j < mapaResultado[0].size(); j++) {
            if (mapaResultado[i][j] != '?') visibles++;
        }
    }

    return visibles > 200;
}

/*
detectar si el camino frontal esta bloqueado por obstaculos
*/
bool HayObstaculoDelante(Sensores s) {
    return s.superficie[2] == 'M' || s.superficie[2] == 'P';
}

/*
El Ingeniero comienza con la exploracion, moviendose por el mapa desconocido priorizando '?'. 
En cuanto detecta 'U', el agente cambia a planificar, calculando la red de tuberías mediante A*. 
Una vez trazado el plan, delega en el Nivel 5 para construir la red
*/
/**
 * @brief Comportamiento del ingeniero para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoIngeniero::ComportamientoIngenieroNivel_6(Sensores sensores) {
    if (sensores.reset) {
        estadoNivel6 = EXPLORANDO;
        planCalculado = false;
        visitadas.clear();
        ultimas_posiciones = 0;
        ultimaF = -1;
        ultimaC = -1;
    }
    ActualizarMapa(sensores);
    if (sensores.superficie[0] == 'D') tiene_zapatillas = true;
    visitadas.insert({sensores.posF, sensores.posC});

    // fase 1: exploracion
    if (estadoNivel6 == EXPLORANDO) {
      //Si encontramos una 'U' en cualquier parte del mapa descubierto, dejamos de explorar y pasamos a planificar la red.
        for (int i = 0; i < mapaResultado.size(); i++) {
            for (int j = 0; j < mapaResultado[0].size(); j++) {
                if (mapaResultado[i][j] == 'U') {
                    estadoNivel6 = PLANIFICANDO;
                    return IDLE;
                }
            }
        }

        // movimiento exploratorio
        ubicacion pF = Delante({sensores.posF, sensores.posC, sensores.rumbo});
        bool muroF = (sensores.superficie[2] == 'M' || sensores.superficie[2] == 'P' || sensores.superficie[2] == 'B');
        bool alturaOK = abs(sensores.cota[2] - sensores.cota[0]) <= (tiene_zapatillas ? 2 : 1);
         
        //si el frente es transitable
        if (!muroF && alturaOK) {
            if (mapaResultado[pF.f][pF.c] == '?') return WALK;
            if (aleatorio(10) < 8) return WALK; 
        }

        //si a la izquierda hay terreno desconocido, giramos
        ubicacion pI = Delante({sensores.posF, sensores.posC, (Orientacion)((sensores.rumbo+6)%8)});
        if (mapaResultado[pI.f][pI.c] == '?') return TURN_SL;

        //si no hay nada nuevo cerca, giro aleatorio
        if (aleatorio(10) < 5) 
          return TURN_SR;
        else 
          return TURN_SL;
    }

    // fase 2: planificacion
    if (estadoNivel6 == PLANIFICANDO) {
        //calcular la red de tuberías hacia la 'U' que hemos descubierto
        listaTuberias = Busqueda_Plan_PasoI(sensores);
        if (!listaTuberias.empty()) {
            VisualizaRedTuberias(listaTuberias);
            planCalculado = true;
            estadoActual = MOVIENDOSE;
            ultimaF = -1;
            ultimaC = -1;
            listaMovimiento.clear();
            estadoNivel6 = EJECUTANDO; // Si hay exito, construimos.
        } else {
            estadoNivel6 = EXPLORANDO; //si no se pudo trazar plan, seguimos explorando
        }
        return IDLE;
    }

    // fase 3: ejecucion
    if (estadoNivel6 == EJECUTANDO) {
        return ComportamientoIngenieroNivel_5(sensores);
    }

    return IDLE;
}

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoIngeniero::ActualizarMapa(Sensores sensores)
{
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo)
  {
  case norte:
    for (int j = 1; j < 4; j++)
      for (int i = -j; i <= j; i++)
      {
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
      for (int i = -j; i <= j; i++)
      {
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
      for (int i = -j; i <= j; i++)
      {
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
      for (int i = -j; i <= j; i++)
      {
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
 * @brief Determina si una casilla es transitable para el ingeniero.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable (no es muro ni precipicio).
 */
bool ComportamientoIngeniero::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas)
{
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size())
    return false;
  return es_camino(mapaResultado[f][c]); // Solo 'C', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el ingeniero: desnivel máximo 1 sin zapatillas, 2 con zapatillas.
 * @param actual Estado actual del agente (fila, columna, orientacion, zap).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoIngeniero::EsAccesiblePorAltura(const ubicacion &actual, bool zap)
{
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size())
    return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (zap && desnivel > 2)
    return false;
  if (!zap && desnivel > 1)
    return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoIngeniero::Delante(const ubicacion &actual) const
{
  ubicacion delante = actual;
  switch (actual.brujula)
  {
  case 0:
    delante.f--;
    break; // norte
  case 1:
    delante.f--;
    delante.c++;
    break; // noreste
  case 2:
    delante.c++;
    break; // este
  case 3:
    delante.f++;
    delante.c++;
    break; // sureste
  case 4:
    delante.f++;
    break; // sur
  case 5:
    delante.f++;
    delante.c--;
    break; // suroeste
  case 6:
    delante.c--;
    break; // oeste
  case 7:
    delante.f--;
    delante.c--;
    break; // noroeste
  }
  return delante;
}

/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoIngeniero::PintaPlan(const list<Action> &plan)
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
 * @brief Imprime las coordenadas y operaciones de un plan de tubería.
 *
 * @param plan  Lista de pasos (fila, columna, operación),
 *              donde operacion = -1 (DIG), operación = 1 (RAISE).
 */
void ComportamientoIngeniero::PintaPlan(const list<Paso> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    cout << it->fil << ", " << it->col << " (" << it->op << ")\n";
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
void ComportamientoIngeniero::VisualizaPlan(const ubicacion &st,
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

/**
 * @brief Convierte un plan de tubería en la lista de casillas usada
 *        por el sistema de visualización.
 *
 * @param st    Estado de partida (no utilizado directamente).
 * @param plan  Lista de pasos del plan de tubería.
 */
void ComportamientoIngeniero::VisualizaRedTuberias(const list<Paso> &plan)
{
  listaCanalizacionTuberias.clear();
  auto it = plan.begin();
  while (it != plan.end())
  {
    listaCanalizacionTuberias.push_back({it->fil, it->col, it->op});
    it++;
  }
}