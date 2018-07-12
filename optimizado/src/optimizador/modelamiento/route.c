#include "route.h"
#include <stdlib.h>
#include <stdio.h>
#include "../optimizacion/optimizacion.h"

/////////////////////////////////////////////////////////////////////////
//                     Funciones privadas                              //
/////////////////////////////////////////////////////////////////////////

/** Crea un LNodo sin pareja y lo rellena con el nodo dado */
static LNode* single_l_node_init(Node* node)
{
  // Creo el l_nodo
  LNode* l_node = malloc(sizeof(LNode));

  // Inicializo sus variables
  l_node -> node = node;
  l_node -> actual_weight = 0;
  l_node -> delta_weight = 0;
  l_node -> last = NULL;
  l_node -> next = NULL;
  l_node -> open_orders_count = 0;
  l_node -> pair = NULL;

  // Retorno el l_nodo
  return l_node;
}

/** Agrega un nodo a la ruta al final */
static void route_append_node(Route* route, Node* node)
{
  // Creo el LNodo para el nodo
  LNode* l_node = single_l_node_init(node);

  // Caso en que la lista esta vacia
  if (route -> nodes -> start == NULL)
  {
    route -> nodes -> start = l_node;
    route -> nodes -> end = l_node;
  }
  // Caso normal
  else
  {
    // Agrego los links
    route -> nodes -> end -> next = l_node;
    l_node -> last = route -> nodes -> end;
    // Pongo el nodo al final
    route -> nodes -> end = l_node;
  }

  // Si es nodo delivery o end, lo linkeo con su pickup o start
  // Caso end
  if (node -> node_type == END)
  {
    route -> nodes -> start -> pair = l_node;
    l_node -> pair = route -> nodes -> start;
  }
  // Caso delivery
  else if (node -> node_type == DELIVERY)
  {
    // itero hacia atras buscando el pickup correspondiente
    for (LNode* ln = route -> nodes -> end; ln; ln = ln -> last)
    {
      // Si es el pickup correspondiente, los uno
      if (ln -> node -> id + order_count == node -> id)
      {
        ln -> pair = l_node;
        l_node -> pair = ln;
      }
    }
  }
}

/** Crea y rellena un diccionario a partir de una ruta de la planificaion base */
static Dictionary* create_dictionary(Route* route)
{
  // Creo el diccionario
  Dictionary* dict = dictionary_init();

  // Agrego al diccionario los vuelos
  // Itero sobre los nodos
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    int macro1 = ln -> node -> father -> id;
    int macro2 = ln -> next -> node -> father -> id;
    // Si cambio de macronodo
    if (macro1 != macro2)
    {
      // Agrego el par al diccionario
      dictionary_add(dict, macro1, macro2);
    }
  }

  // // Imprimo el diccionario
  // dictionary_print(dict);

  // Retorno el diccionario
  return dict;
}

/** Calcula el costo de cancelacion de toda la ruta */
static double get_cancellation_cost(Route* route)
{
  // En la ruta de la planificacion base, no hay costo de cancelacion
  if (route -> is_bp) return 0;

  // Dejo los contadores en 0
  dictionary_reset(route -> cancelled_flight);
  // Itero sobre la ruta
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Los ids de los macro nodos
    int id1 = ln -> node -> father -> id;
    int id2 = ln -> next -> node -> father -> id;
    // Si son distintos, sumo 1 a la ruta
    if (id1 != id2)
    {
      dictionary_create_flight(route -> cancelled_flight, id1, id2);
    }
  }
  // Itero sobre el diccionario contando cuales vuelos estan en 0
  int cancelled_count = 0;
  for (int i = 0; i < route -> cancelled_flight -> array_lenght; i++)
  {
    // Si es una celda valida
    if (route -> cancelled_flight -> cells[i])
    {
      // Si no se ha realizado este vuelo
      if (route -> cancelled_flight -> cells[i] -> fly_count == 0)
      {
        // Cuento una ruta cancelada
        cancelled_count++;
      }
    }
  }
  // Calculo el costo de cancelacion
  return cancelled_count * penalization_cost;
}

/** Copia un l_nodo con todo lo que tiene */
static LNode* l_node_copy(LNode* ln)
{
  // Creo el l_nodo
  LNode* copy = malloc(sizeof(LNode));

  // Copio sus variables
  copy -> actual_weight = ln -> actual_weight;
  copy -> delta_weight = ln -> delta_weight;
  copy -> node = ln -> node;
  copy -> open_orders_count = ln -> open_orders_count;

  // Dejo las referencias en null
  copy -> next = NULL;
  copy -> last = NULL;
  copy -> pair = NULL;

  // Retorno la copia
  return copy;
}

/** Copia una lista de nodos */
static List* list_copy(List* list)
{
  // Creo la lista nueva
  List* copy = malloc(sizeof(List));

  // Copio el inicio
  copy -> start = l_node_copy(list -> start);
  copy -> end = copy -> start;
  // Itero sobre la lista copiando los lnodos y ligandolos
  for (LNode* ln = list -> start -> next; ln; ln = ln -> next)
  {
    // Copio el lnodo
    LNode* new = l_node_copy(ln);

    // Lo conecto al anterior
    copy -> end -> next = new;
    new -> last = copy -> end;

    // Dejo el nuevo nodo como le ultimo
    copy -> end = new;

    // Si es nodo delivery, busco el pickup y lo conecto
    if (new -> node -> node_type == DELIVERY)
    {
      // itero hacia atras buscando el pickup correspondiente
      for (LNode* l_node = new -> last; l_node; l_node = l_node -> last)
      {
        // Si es el pickup correspondiente, los uno
        if (l_node -> node -> id + order_count == new -> node -> id)
        {
          l_node -> pair = new;
          new -> pair = l_node;
        }
      }
    }

    // Si es nodo end, lo conecto con el start
    if (new -> node -> node_type == END)
    {
      new -> pair = copy -> start;
      copy -> start -> pair = new;
    }
  }

  // Retorno la copia
  return copy;
}

/** Destruye una lista */
static void list_destroy(List* list)
{
  LNode* ln = list -> start -> next;
  while (ln)
  {
    free(ln -> last);
    ln = ln -> next;
  }
  free(list -> end);
  free(list);
}


/////////////////////////////////////////////////////////////////////////
//                     Funciones publicas                              //
/////////////////////////////////////////////////////////////////////////



// FUNCIONES BASICAS DE RUTAS

/** Crea una ruta nueva vacia */
Route* route_init(Airplane* airplane)
{
  // Creo la ruta y su lista de nodos
  Route* route = malloc(sizeof(Route));
  route -> nodes = malloc(sizeof(List));
  List* list = route -> nodes;

  // Inicializo los valores de la ruta
  route -> valid = true;

  // Creo los Lnodos inicial y final vacios
  list -> start = NULL;
  list -> end = NULL;

  // Calculo su funcion objetivo
  route -> objective_function = 0;

  // Le agrego el avion correspondiente
  route -> airplane = airplane;

  // Retorno la ruta
  return route;
}

/** Imprime la ruta */
void route_print(Route* route, FILE* file)
{
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    fprintf(file, "%d ", ln -> node -> id);
  }
  Node* end = route -> nodes -> end -> node;

  fprintf(file, "%d\n", end -> id);

  // Imprimo los tiempos
  fprintf(file, "Tiempos:\n");
  for (LNode* ln = route -> nodes -> start; ln; ln = ln -> next)
  {
    fprintf(file ,"(%lf, %lf) ", ln -> arrive_time, ln -> leave_time);
  }
  fprintf(file, "\n");

  // Imprimo los pesos
  fprintf(file, "Pesos:\n");
  for (LNode* ln = route -> nodes -> start; ln; ln = ln -> next)
  {
    fprintf(file ,"(q: %lf, Q: %lf)", ln -> delta_weight, ln -> actual_weight);
  }
  fprintf(file, "\n");
}

/** Copia una ruta */
Route* route_copy(Route* route, Map* map)
{
  // Creo la ruta nueva
  Route* copy = malloc(sizeof(Route));

  // Copio las variables simples de copiar
  copy -> airplane = route -> airplane;
  copy -> objective_function = route -> objective_function;

  // No es la la planificaicon base
  copy -> is_bp = false;

  // Copio el diccionario
  copy -> cancelled_flight = dictionary_copy(route -> cancelled_flight);

  // Copio la lista de nodos
  copy -> nodes = list_copy(route -> nodes);

  // Calculo la fast_of si se esta copiando una ruta original
  if (route -> is_bp) copy -> fast_of = fast_of(copy, map);
  else copy -> fast_of = route -> fast_of;

  // Veo si es valida
  route -> valid = assign_time(copy) && assign_weights(copy);

  // Retorno la copia
  return copy;
}

/** Libera la memoria de una ruta */
void route_destroy(Route* route)
{
  dictionary_destroy(route -> cancelled_flight);
  list_destroy(route -> nodes);
  free(route);
}

// FUNCIONES DE LA PLANIFICACION BASE

/** Crea la planificacion base a partir del archivo */
BP* bp_init(char* bp_file_path, Map* map)
{
  // Por simplicidad
  Airplane** airplanes = map -> airplanes;

  // Creo la planificacion
  BP* bp = malloc(sizeof(BP));

  // Abro el archivo con la planificacion
  FILE* bp_file = fopen(bp_file_path, "r");

  // Leo la cantidad de rutas
  fscanf(bp_file, "%d\n", &bp -> route_count);

  // Creo el arreglo de rutas
  bp -> routes = malloc(sizeof(Route*) * bp -> route_count);

  // Leo las rutas y las completo
  for (int i = 0; i < bp -> route_count; i++)
  {
    // Creo la ruta vacia
    bp -> routes[i] = route_init(airplanes[i]);

    // Leo cuantos nodos tiene la ruta
    int node_count;
    fscanf(bp_file, "%d", &node_count);

    // Itero sobre los nodos y los agrego a la ruta
    for (int j = 0; j < node_count; j++)
    {
      // Leo el id del nodo
      int node_id;
      fscanf(bp_file, "%d", &node_id);

      // Agrego el nodo a la ruta
      route_append_node(bp -> routes[i], map -> nodes[node_id]);
    }

    // Indico que la ruta es valida y que es de la planificacion base
    bp -> routes[i] -> valid = true;
    bp -> routes[i] -> is_bp = true;

    // Creo su diccionario
    bp -> routes[i] -> cancelled_flight = create_dictionary(bp -> routes[i]);

    // Calculo su funcion objetivo
    bp -> routes[i] -> objective_function = objective_function(bp -> routes[i], map);

    // Calculo su funcion objetivo greedy
    bp -> routes[i] -> fast_of = fast_of(bp -> routes[i], map);
  }

  // Cierro el archivo
  fclose(bp_file);

  // Retorno la planificacion
  return bp;
}

/** Libera la memoria asociada a la planificacion base */
void bp_destroy(BP* bp)
{
  for (int k = 0; k < airplanes_count; k++)
  {
    route_destroy(bp -> routes[k]);
  }
  free(bp -> routes);
  free(bp);
}

// FUNCIONES QUE ALTERAN LAS RUTAS Y CAMBIAN LA FUNCION OBJETIVO RAPIDA

/** Inserta un nodo en la posicion siguiente al lnodo dado */
void l_node_insert_next(Route* route, LNode* original_l_node, LNode* new_l_node)
{
  LNode* next = original_l_node -> next;

  // Cambio las referencias del l_nodo nuevo
  new_l_node -> last = original_l_node;
  new_l_node -> next = next;
  // Cambio la referencia del lnodo anterior
  original_l_node -> next = new_l_node;
  // Cambio la referencia del siguiente al este
  next -> last = new_l_node;

  ////////////////////////////////////////////////////////////////////////
  //                   Actualizo la FO greedy                           //
  ////////////////////////////////////////////////////////////////////////

  /////// Variables utiles ///////
  // id del macronodo padre
  Macronode* original_mn = original_l_node -> node -> father;
  // id del macronodo padre del nuevo nodo
  Macronode* new_mn = new_l_node -> node -> father;
  // id del macronodo siguiente
  Macronode* next_mn = new_l_node -> next -> node -> father;

  /////// Tarifa y dual_pi ///////
  // Si es nodo pickup
  if (new_l_node -> node -> node_type == PICKUP)
  {
    // Si su tarifa es superior al dual_pi
    double diff = new_l_node -> node -> fee - new_l_node -> node -> dual_pi;
    if (diff > 0)
    {
      // Agrego la tarifa y dual pi de todo el peso
      route -> fast_of += diff * new_l_node -> node -> total_weight;
    }
  }

  /////// Costos de vuelos ///////
  // Elimino el costo de la arista antigua
  route -> fast_of += distance(original_mn, next_mn);
  // Agrego el costo de las aristas nuevas
  route -> fast_of -= distance(original_mn, new_mn);
  route -> fast_of -= distance(new_mn, next_mn);

  /////// Costos de penalizacion por cancelacion ///////
  // Si estoy cancelando un vuelo de la PB, agrego el costo
  if (dictionary_cancel_flight(route -> cancelled_flight, original_mn -> id, next_mn -> id))
  {
    route -> fast_of -= penalization_cost;
  }
  // Si estoy agregando un vuelo de la PB que habia cancelado, elimino el costo
  if (dictionary_create_flight(route -> cancelled_flight, original_mn -> id, new_mn -> id))
  {
    route -> fast_of += penalization_cost;
  }
  if (dictionary_create_flight(route -> cancelled_flight, new_mn -> id, next_mn -> id))
  {
    route -> fast_of += penalization_cost;
  }
}

/** Saca el l_node de la lista conectando sus hermanos entre si */
void l_node_pop(Route* route, LNode* l_node)
{
  // Elimino el nodo
  if (l_node -> last)
  {
    l_node -> last -> next = l_node -> next;
  }
  if (l_node -> next)
  {
    l_node -> next -> last = l_node -> last;
  }

  ////////////////////////////////////////////////////////////////////////
  //                   Actualizo la FO greedy                           //
  ////////////////////////////////////////////////////////////////////////

  /////// Tarifa y costo dual ///////
  // Elimino la tarifa y el costo dual solo si es positivo
  if (l_node -> node -> node_type == PICKUP)
  {
    double diff = l_node -> node -> fee - l_node -> node -> dual_pi;
    if (diff > 0)
    {
      route -> fast_of -= diff * l_node -> node -> total_weight;
    }
  }

  /////// Costo de viaje ///////
  // Elimino el costo de las aristas viejas
  Macronode* last = l_node -> last -> node -> father;
  Macronode* actual = l_node -> node -> father;
  Macronode* next = l_node -> next -> node -> father;
  route -> fast_of += distance(last, actual);
  route -> fast_of += distance(actual, next);

  // Agrego el costo de la nueva
  route -> fast_of -= distance(last, next);

  /////// Costos de cancelacion ///////
  // Agrego el costo de cancelacion de los vuelos (si es el caso)
  if (dictionary_cancel_flight(route -> cancelled_flight, last -> id, actual -> id))
  {
    route -> fast_of -= penalization_cost;
  }
  if (dictionary_cancel_flight(route -> cancelled_flight, actual -> id, next -> id))
  {
    route -> fast_of -= penalization_cost;
  }
  // Elimino el costo de cancelacion si es el caso
  if (dictionary_create_flight(route -> cancelled_flight, last -> id, next -> id))
  {
    route -> fast_of += penalization_cost;
  }
}

/** Intercambia dos nodos dentro de una ruta */
void l_nodes_swap(Route* route, LNode* ln1, LNode* ln2)
{
  // Caso en que estan pegados
  if (ln2 -> next == ln1)
  {
    LNode* aux = ln1;
    ln1 = ln2;
    ln2 = aux;
  }
  if (ln1 -> next == ln2)
  {
    // Obtengo el nodo anterior y el siguiente
    LNode* last = ln1 -> last;
    LNode* next = ln2 -> next;
    // Cambio las referencias de los bordes a los nodos a cambiar
    last -> next = ln2;
    next -> last = ln1;
    // Cambio las referencias de los nodos a cambiar a los bordes
    ln1 -> next = next;
    ln2 -> last = last;
    // Cambio las referencias entre ellos
    ln1 -> last = ln2;
    ln2 -> next = ln1;

    ////////////////////////////////////////////////////////////////////////
    //                   Actualizo la FO greedy                           //
    ////////////////////////////////////////////////////////////////////////

    /////// Tarifa y costo dual ///////
    // Se mantienen igual

    /////// Costo de viaje ///////
    Macronode* last_M = last -> node -> father;
    Macronode* mn1 = ln1 -> node -> father;
    Macronode* mn2 = ln2 -> node -> father;
    Macronode* next_M = next -> node -> father;
    // Elimino el costo de los nodos viejos
    route -> fast_of += distance(last_M, mn1);
    route -> fast_of += distance(mn2, next_M);
    // Agrego el costo nuevo
    route -> fast_of -= distance(last_M, mn2);
    route -> fast_of -= distance(mn1, next_M);

    /////// Costos de cancelacion ///////
    // Agrego los costos de los vuelos cancelados
    if (dictionary_cancel_flight(route -> cancelled_flight, last_M -> id, mn1 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mn2 -> id, next_M -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mn1 -> id, mn2 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    // Elimino el costo de cancelacion de los vuelos nuevos
    if (dictionary_create_flight(route -> cancelled_flight, last_M -> id, mn2 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mn1 -> id, next_M -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mn2 -> id, mn1 -> id))
    {
      route -> fast_of += penalization_cost;
    }
  }
  // Caso en que estan separados
  else
  {
    // Obtengo los bordes de ambos
    LNode* last1 = ln1 -> last;
    LNode* next1 = ln1 -> next;
    LNode* last2 = ln2 -> last;
    LNode* next2 = ln2 -> next;

    // Cambio las referencias de los bordes a los nodos
    last1 -> next = ln2;
    next1 -> last = ln2;
    last2 -> next = ln1;
    next2 -> last = ln1;
    // Cambio las referencias de los nodos a los bordes
    ln1 -> next = next2;
    ln1 -> last = last2;
    ln2 -> next = next1;
    ln2 -> last = last1;

    ////////////////////////////////////////////////////////////////////////
    //                   Actualizo la FO greedy                           //
    ////////////////////////////////////////////////////////////////////////

    /////// Tarifa y costo dual ///////
    // Se mantienen igual

    /////// Costo de viaje ///////
    Macronode* last1_M = last1 -> node -> father;
    Macronode* next1_M = next1 -> node -> father;
    Macronode* last2_M = last2 -> node -> father;
    Macronode* next2_M = next2 -> node -> father;
    Macronode* mn1 = ln1 -> node -> father;
    Macronode* mn2 = ln2 -> node -> father;
    // Elimino el costo de los nodos viejos
    route -> fast_of += distance(last1_M, mn1);
    route -> fast_of += distance(mn1, next1_M);
    route -> fast_of += distance(last2_M, mn2);
    route -> fast_of += distance(mn2, next2_M);
    // Agrego el costo nuevo
    route -> fast_of -= distance(last1_M, mn2);
    route -> fast_of -= distance(mn2, next1_M);
    route -> fast_of -= distance(last2_M, mn1);
    route -> fast_of -= distance(mn1, next2_M);

    /////// Costos de cancelacion ///////
    // Agrego los costos de los vuelos cancelados
    if (dictionary_cancel_flight(route -> cancelled_flight, last1_M -> id, mn1 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mn1 -> id, next1_M -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, last2_M -> id, mn2 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mn2 -> id, next2_M -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    // Elimino el costo de cancelacion de los vuelos nuevos
    if (dictionary_create_flight(route -> cancelled_flight, last1_M -> id, mn2 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mn2 -> id, next1_M -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, last2_M -> id, mn1 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mn1 -> id, next2_M -> id))
    {
      route -> fast_of += penalization_cost;
    }
  }
}

/** Inserta una subruta en la ruta en la posicion dada */
void sub_route_insert_next(Route* route, LNode* position, LNode* pickup)
{
  // Actualizo los punteros de la subruta a la ruta
  pickup -> last = position;
  pickup -> pair -> next = position -> next;
  // Acutalizo los punteros de la ruta a la subruta
  position -> next -> last = pickup -> pair;
  position -> next = pickup;

  ////////////////////////////////////////////////////////////////////////
  //                   Actualizo la FO greedy                           //
  ////////////////////////////////////////////////////////////////////////

  /////// Variables utiles ////// /
  // id del macronodo padre
  Macronode* last_mn = position -> node -> father;
  // id del macronodo padre del nuevo pickup
  Macronode* pickup_mn = pickup -> node -> father;
  // id del macronodo padre del nuevo delivery
  Macronode* delivery_mn = pickup -> pair -> node -> father;
  // id del macronodo siguiente
  Macronode* next_mn = pickup -> pair -> next -> node -> father;

  /////// Tarifa y dual_pi ///////
  // No se actualizan (porque solo se inserta cuando se saco la subruta de otro lado)

  /////// Costos de vuelos ///////
  // Elimino el costo de la arista antigua
  route -> fast_of += distance(last_mn, next_mn);
  // Agrego el costo de las aristas nuevas
  route -> fast_of -= distance(last_mn, pickup_mn);
  route -> fast_of -= distance(delivery_mn, next_mn);

  /////// Costos de penalizacion por cancelacion ///////
  // Si estoy cancelando un vuelo de la PB, agrego el costo
  if (dictionary_cancel_flight(route -> cancelled_flight, last_mn -> id, next_mn -> id))
  {
    route -> fast_of -= penalization_cost;
  }
  // Si estoy agregando un vuelo de la PB que habia cancelado, elimino el costo
  if (dictionary_create_flight(route -> cancelled_flight, last_mn -> id, pickup_mn -> id))
  {
    route -> fast_of += penalization_cost;
  }
  if (dictionary_create_flight(route -> cancelled_flight, delivery_mn -> id, next_mn -> id))
  {
    route -> fast_of += penalization_cost;
  }
}

/** Elimina una subruta de la ruta */
void sub_route_pop(Route* route, LNode* pickup)
{
  // Variables que ayudan
  LNode* delivery = pickup -> pair;
  LNode* start = pickup -> last;
  LNode* end = delivery -> next;

  // Saco la subruta truncada
  start -> next = end;
  end -> last = start;

  ////////////////////////////////////////////////////////////////////////
  //                   Actualizo la FO greedy                           //
  ////////////////////////////////////////////////////////////////////////

  /////// Tarifa y costo dual ///////
  // No se actualiza (porque se va a volver a insertar en otro lado)

  /////// Costo de viaje ///////
  // Elimino el costo de las aristas viejas
  Macronode* last_mn = start -> node -> father;
  Macronode* pickup_mn = pickup -> node -> father;
  Macronode* delivery_mn = pickup -> pair -> node -> father;
  Macronode* next_mn = end -> node -> father;
  route -> fast_of += distance(last_mn, pickup_mn);
  route -> fast_of += distance(delivery_mn, next_mn);

  // Agrego el costo de la nueva
  route -> fast_of -= distance(last_mn, next_mn);

  /////// Costos de cancelacion ///////
  // Agrego el costo de cancelacion de los vuelos (si es el caso)
  if (dictionary_cancel_flight(route -> cancelled_flight, last_mn -> id, pickup_mn -> id))
  {
    route -> fast_of -= penalization_cost;
  }
  if (dictionary_cancel_flight(route -> cancelled_flight, delivery_mn -> id, next_mn -> id))
  {
    route -> fast_of -= penalization_cost;
  }
  // Elimino el costo de cancelacion si es el caso
  if (dictionary_create_flight(route -> cancelled_flight, last_mn -> id, next_mn -> id))
  {
    route -> fast_of += penalization_cost;
  }
}

/** Intercambia dos subrutas truncadas */
void sub_route_swap(Route* route, LNode* ln1, LNode* ln2)
{
  // Caso en que estan pegados
  if (ln2 -> pair -> next == ln1)
  {
    LNode* aux = ln1;
    ln1 = ln2;
    ln2 = aux;
  }
  if (ln1 -> pair -> next == ln2)
  {
    // Obtengo el nodo anterior y el siguiente
    LNode* last = ln1 -> last;
    LNode* next = ln2 -> pair -> next;
    // Cambio las referencias de los bordes a los nodos a cambiar
    last -> next = ln2;
    next -> last = ln1 -> pair;
    // Cambio las referencias de los nodos a cambiar a los bordes
    ln1 -> pair -> next = next;
    ln2 -> last = last;
    // Cambio las referencias entre ellos
    ln1 -> last = ln2 -> pair;
    ln2 -> pair -> next = ln1;

    ////////////////////////////////////////////////////////////////////////
    //                   Actualizo la FO greedy                           //
    ////////////////////////////////////////////////////////////////////////

    /////// Tarifa y costo dual ///////
    // Se mantienen igual

    /////// Costo de viaje ///////
    Macronode* last_M = last -> node -> father;
    Macronode* mns1 = ln1 -> node -> father;
    Macronode* mne1 = ln1 -> pair -> node -> father;
    Macronode* mns2 = ln2 -> node -> father;
    Macronode* mne2 = ln2 -> pair -> node -> father;
    Macronode* next_M = next -> node -> father;
    // Elimino el costo de los nodos viejos
    route -> fast_of += distance(last_M, mns1);
    route -> fast_of += distance(mne1, mns2);
    route -> fast_of += distance(mne2, next_M);
    // Agrego el costo nuevo
    route -> fast_of -= distance(last_M, mns2);
    route -> fast_of -= distance(mne2, mns1);
    route -> fast_of -= distance(mne1, next_M);

    /////// Costos de cancelacion ///////
    // Agrego los costos de los vuelos cancelados
    if (dictionary_cancel_flight(route -> cancelled_flight, last_M -> id, mns1 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mne1 -> id, mns2 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mne2 -> id, next_M -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    // Elimino el costo de cancelacion de los vuelos nuevos
    if (dictionary_create_flight(route -> cancelled_flight, last_M -> id, mns2 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mne2 -> id, mns1 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mne1 -> id, next_M -> id))
    {
      route -> fast_of += penalization_cost;
    }
  }
  // Caso en que estan separados
  else
  {
    // Obtengo los bordes de ambos
    LNode* last1 = ln1 -> last;
    LNode* next1 = ln1 -> pair -> next;
    LNode* last2 = ln2 -> last;
    LNode* next2 = ln2 -> pair -> next;

    // Cambio las referencias de los bordes a los nodos
    last1 -> next = ln2;
    next1 -> last = ln2 -> pair;
    last2 -> next = ln1;
    next2 -> last = ln1 -> pair;
    // Cambio las referencias de los nodos a los bordes
    ln1 -> pair -> next = next2;
    ln1 -> last = last2;
    ln2 -> pair -> next = next1;
    ln2 -> last = last1;

    ////////////////////////////////////////////////////////////////////////
    //                   Actualizo la FO greedy                           //
    ////////////////////////////////////////////////////////////////////////

    /////// Tarifa y costo dual ///////
    // Se mantienen igual

    /////// Costo de viaje ///////
    Macronode* last1_M = last1 -> node -> father;
    Macronode* next1_M = next1 -> node -> father;
    Macronode* last2_M = last2 -> node -> father;
    Macronode* next2_M = next2 -> node -> father;
    Macronode* mns1 = ln1 -> node -> father;
    Macronode* mne1 = ln1 -> pair -> node -> father;
    Macronode* mns2 = ln2 -> node -> father;
    Macronode* mne2 = ln2 -> pair -> node -> father;
    // Elimino el costo de los nodos viejos
    route -> fast_of += distance(last1_M, mns1);
    route -> fast_of += distance(mne1, next1_M);
    route -> fast_of += distance(last2_M, mns2);
    route -> fast_of += distance(mne2, next2_M);
    // Agrego el costo nuevo
    route -> fast_of -= distance(last1_M, mns2);
    route -> fast_of -= distance(mne2, next1_M);
    route -> fast_of -= distance(last2_M, mns1);
    route -> fast_of -= distance(mne1, next2_M);

    /////// Costos de cancelacion ///////
    // Agrego los costos de los vuelos cancelados
    if (dictionary_cancel_flight(route -> cancelled_flight, last1_M -> id, mns1 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mne1 -> id, next1_M -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, last2_M -> id, mns2 -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    if (dictionary_cancel_flight(route -> cancelled_flight, mne2 -> id, next2_M -> id))
    {
      route -> fast_of -= penalization_cost;
    }
    // Elimino el costo de cancelacion de los vuelos nuevos
    if (dictionary_create_flight(route -> cancelled_flight, last1_M -> id, mns2 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mne2 -> id, next1_M -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, last2_M -> id, mns1 -> id))
    {
      route -> fast_of += penalization_cost;
    }
    if (dictionary_create_flight(route -> cancelled_flight, mne1 -> id, next2_M -> id))
    {
      route -> fast_of += penalization_cost;
    }
  }
}



// FUNCIONES PARA CREAR LNODOS EN UNA RUTA

/** Crea dos LNodos enparejados vacios y retorna el primero */
LNode* l_node_init()
{
  // Los creo
  LNode* first = malloc(sizeof(LNode));
  LNode* last = malloc(sizeof(LNode));

  // Los emparejo
  first -> pair = last;
  last -> pair = first;

  // Completo las variables de next y last con NULL
  first -> last = NULL;
  first -> next = NULL;
  last -> last = NULL;
  last -> next = NULL;

  // Retorno el primero
  return first;
}

/** Asigna nodos en un par de LNodos a partir de un pedido */
void l_node_order_fill(LNode* pickup, Order* order)
{
  // Agrego las referencias a los nodos
  LNode* delivery = pickup -> pair;
  pickup -> node = order -> pickup;
  delivery -> node = order -> delivery;
  pickup -> pair = delivery;
  delivery -> pair = pickup;

  // Dejo otras variable sen 0
  pickup -> actual_weight = 0;
  pickup -> delta_weight = 0;
  pickup -> open_orders_count = 0;
  delivery -> actual_weight = 0;
  delivery -> delta_weight = 0;
  delivery -> open_orders_count = 0;
}

// FUNCIONES SOBRE LAS RUTAS USADAS EN EL CODIGO ANS

/** Calcula la funcion objetivo de la ruta iterando sobre ella */
double objective_function(Route* route, Map* map)
{
  // Partes de la funcion objetivo
  double fees = 0;
  double distances = 0;
  double duals = 0;
  double cancellation_cost = 0;

  // route_print(route);

  // Sumo las tarifas y resto los costos de viaje y los duales
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // sumo la parte de la carga que se cargo
      fees += ln -> node -> fee * ln -> delta_weight;
      duals += ln -> node -> dual_pi * ln -> delta_weight;
    }

    // Resto distancias
    distances += distance(ln -> node -> father, ln -> next -> node -> father);
  }
  // agrego el dual gamma
  duals += route -> airplane -> dual_gamma;


  // Costo de cancelacion
  cancellation_cost = get_cancellation_cost(route);

  // Calculo el total y lo retorno
  double total = fees - distances - duals - cancellation_cost;

  // // Imprimo cada uno de los elementos
  // if (duals > 0 || total == 0)
  // {
  //   printf("Fees: %lf\n", fees);
  //   printf("Distances: %lf\n", distances);
  //   printf("Duals: %lf\n", duals);
  //   printf("DualGamma: %lf\n", route -> airplane -> dual_gamma);
  //   printf("Costo de cancelacion: %lf\n", cancellation_cost);
  //   printf("Total: %lf\n\n", total);
  // }

  return total;
}

/** Calcula la funcion objetivo de la ruta iterando sobre ella */
void print_objective_function(Route* route, Map* map)
{
  // Partes de la funcion objetivo
  double fees = 0;
  double distances = 0;
  double duals = 0;
  double cancellation_cost = 0;

  // route_print(route);

  // Sumo las tarifas y resto los costos de viaje y los duales
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // sumo la parte de la carga que se cargo
      fees += ln -> node -> fee * ln -> delta_weight;
      duals += ln -> node -> dual_pi * ln -> delta_weight;
    }

    // Resto distancias
    distances += distance(ln -> node -> father, ln -> next -> node -> father);
  }
  // agrego el dual gamma
  duals += route -> airplane -> dual_gamma;


  // Costo de cancelacion
  cancellation_cost = get_cancellation_cost(route);

  // Calculo el total y lo retorno
  double total = fees - distances - duals - cancellation_cost;

  // Imprimo cada uno de los elementos
  printf("Fees: %lf\n", fees);
  printf("Distances: %lf\n", distances);
  printf("Duals: %lf\n", duals);
  printf("DualGamma: %lf\n", route -> airplane -> dual_gamma);
  printf("Costo de cancelacion: %lf\n", cancellation_cost);
  printf("Total: %lf\n\n", total);
}

/** Calcula la funcion objetivo de manera optimista (asuminedo carga completa) */
double fast_of(Route* route, Map* map)
{

  // Partes de la funcion objetivo
  double fees = 0;
  double distances = 0;
  double duals = 0;
  double cancellation_cost = 0;

  // Sumo las tarifas y resto los costos de viaje y los duales
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si el costo reducido es mayor, no lo sumo
    if (ln -> node -> dual_pi < ln -> node -> fee)
    {
      // sumo la carga completa
      fees += ln -> node -> fee * ln -> node -> total_weight;
      duals += ln -> node -> dual_pi * ln -> node -> total_weight;
    }

    // Resto distancias
    distances += distance(ln -> node -> father, ln -> next -> node -> father);
  }

  // agrego el dual gamma
  duals += route -> airplane -> dual_gamma;

  // Costo de cancelacion
  cancellation_cost = get_cancellation_cost(route);

  // Calculo el total y lo retorno
  double total = fees - distances - duals - cancellation_cost;

  // // Imprimo cada uno de los elementos
  // printf("Fees: %lf\n", fees);
  // printf("Distances: %lf\n", distances);
  // printf("Duals: %lf\n", duals);
  // printf("DualGamma: %lf\n", route -> airplane -> dual_gamma);
  // printf("Costo de cancelacion: %lf\n", cancellation_cost);
  // printf("Total: %lf\n\n", total);

  return total;
}

/** Calcula la utilidad de la ruta */
double utility(Route* route)
{
  // Partes de la utilidad
  double fees = 0;
  double distances = 0;
  double cancellation_cost = 0;

  // Sumo las tarifas y resto los costos de viaje
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // sumo la parte de la carga que se cargo
      fees += ln -> node -> fee * ln -> delta_weight;
    }

    // Resto distancias
    distances += distance(ln -> node -> father, ln -> next -> node -> father);
  }

  // Costo de cancelacion
  cancellation_cost = get_cancellation_cost(route);

  // Calculo el total y lo retorno
  double total = fees - distances - cancellation_cost;
  return total;
}

/** Calcula la utilidad de la ruta */
void print_utility(Route* route)
{
  // Partes de la utilidad
  double fees = 0;
  double distances = 0;
  double cancellation_cost = 0;

  // Sumo las tarifas y resto los costos de viaje
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // sumo la parte de la carga que se cargo
      fees += ln -> node -> fee * ln -> delta_weight;
    }

    // Resto distancias
    distances += distance(ln -> node -> father, ln -> next -> node -> father);
  }

  // Costo de cancelacion
  cancellation_cost = get_cancellation_cost(route);

  // Calculo el total y lo retorno
  double total = fees - distances - cancellation_cost;

  printf("Fees: %lf\n", fees);
  printf("Distances: %lf\n", distances);
  printf("Costo de cancelacion: %lf\n", cancellation_cost);
  printf("Total: %lf\n\n", total);
}

/** Ajusta los tiempos de una ruta y determina si es factible */
bool assign_time(Route* route)
{
  // route: ruta a ajustar sus tiempos

  // Cuento cuantos nodos hay
  int count = 0;
  for (LNode* ln = route -> nodes -> start; ln; ln = ln -> next) count ++;

  // Asigno los tiempos de llegada lo mas temprano posible
  int i = 0;
  for (LNode* ln = route -> nodes -> start; ln; ln = ln -> next)
  {
    ln -> arrive_time = ln -> node -> start_time;
    i++;
  }

  // Itero sobre la ruta
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    double dist = distance(ln -> node -> father, ln -> next -> node -> father);
    double delay = ln -> node -> delay_time;
    double end_i = ln -> node -> end_time;
    double start_j = ln -> next -> node -> start_time;
    double end_j = ln -> next -> node -> end_time;
    if (ln -> arrive_time > end_i || ln -> arrive_time + delay + dist > end_j)
    {
      route -> valid = false;
      return false;
    }
    else if (ln -> arrive_time + delay + dist >= start_j && ln -> arrive_time + delay + dist <= end_j)
    {
      ln -> next -> arrive_time = ln -> arrive_time + delay + dist;
    }
  }

  // Asigno los tiempos de salida
  for (LNode* ln = route -> nodes -> end; ln -> last; ln = ln -> last)
  {
    double dist = distance(ln -> last -> node -> father, ln -> node -> father);
    ln -> last -> leave_time = ln -> arrive_time - dist;
  }
  LNode* end = route -> nodes -> end;
  end -> leave_time = end -> arrive_time + end -> node -> delay_time;

  route -> valid = true;
  return true;
}

/** Asigna las cargas de manera greedy, y si no se puede optimiza */
bool assign_weights(Route* route)
{
  // Capacidad total
  double capacity = route -> airplane -> total_capacity;

  // Asigno peso 0 inicial
  route -> nodes -> start -> delta_weight = 0;
  route -> nodes -> start -> actual_weight = 0;

  // Para el resto de los nodos los actualizo de manera greedy
  for (LNode* ln = route -> nodes -> start -> next; ln -> next; ln = ln -> next)
  {
    // Si es pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // Si puedo cargar todo, lo cargo
      if (ln -> last -> actual_weight + ln -> node -> total_weight <= capacity)
      {
        ln -> delta_weight = ln -> node -> total_weight;
        ln -> actual_weight = ln -> last -> actual_weight + ln -> delta_weight;
      }
      // Si no puedo, llamo al optimizador de cargas
      else
      {
        optimize_weight(route);
        return route -> valid;
      }
    }
    // Si es delivery
    else if (ln -> node -> node_type == DELIVERY)
    {
      // Dejo lo que el pickup recogio
      ln -> delta_weight = -ln -> pair -> delta_weight;
      ln -> actual_weight = ln -> last -> actual_weight + ln -> delta_weight;
    }
  }
  // El nodo final debe tener delta weight 0 y actual weight 0
  route -> nodes -> end -> actual_weight = 0;
  route -> nodes -> end -> delta_weight = 0;

  route -> valid = true;
  return true;
}

/** Asigna las cargas de manera greedy maximizando utilidad, y si no se puede optimiza */
bool utility_assign_weights(Route* route)
{
  // Capacidad total
  double capacity = route -> airplane -> total_capacity;

  // Asigno peso 0 inicial
  route -> nodes -> start -> delta_weight = 0;
  route -> nodes -> start -> actual_weight = 0;

  // Para el resto de los nodos los actualizo de manera greedy
  for (LNode* ln = route -> nodes -> start -> next; ln -> next; ln = ln -> next)
  {
    // Si es pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // Si puedo cargar todo, lo cargo
      if (ln -> last -> actual_weight + ln -> node -> total_weight <= capacity)
      {
        ln -> delta_weight = ln -> node -> total_weight;
        ln -> actual_weight = ln -> last -> actual_weight + ln -> delta_weight;
      }
      // Si no puedo, llamo al optimizador de cargas
      else
      {
        utility_optimize_weight(route);
        return route -> valid;
      }
    }
    // Si es delivery
    else if (ln -> node -> node_type == DELIVERY)
    {
      // Dejo lo que el pickup recogio
      ln -> delta_weight = -ln -> pair -> delta_weight;
      ln -> actual_weight = ln -> last -> actual_weight + ln -> delta_weight;
    }
  }
  // El nodo final debe tener delta weight 0 y actual weight 0
  route -> nodes -> end -> actual_weight = 0;
  route -> nodes -> end -> delta_weight = 0;

  route -> valid = true;
  return true;
}
