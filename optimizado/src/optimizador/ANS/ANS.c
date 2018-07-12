#include "ANS.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "../random/pcg_basic.h"

//////////////////////////////////////////////////////////////////////////
//                        Funciones del ANS                             //
//////////////////////////////////////////////////////////////////////////

/** Retorna un double aleatorio entre 0 y la cota dada */
double random_bounded(double bound)
{
  // Obtengo un int entre 0 y el maximo de ints
  uint32_t rand_int = pcg32_random();
  // Transformo el numero a un double entre 0 y 1 uniforme
  double rand_double = (double)rand_int / (double)UINT32_MAX;
  // Multiplico este numero por la cota
  return rand_double * bound;
}

/** Crea la estructura ANS del programa */
ANS* ans_init(char* orders_filepath, char* airplanes_filepath, char* bp_filepath)
{
  // Creo al ans
  ANS* ans = malloc(sizeof(ANS));

  // Inicializo el mapa
  ans -> map = map_init(orders_filepath, airplanes_filepath);

  // Inicializo la planificacion base
  ans -> bp = bp_init(bp_filepath, ans -> map);

  // Inicializo el arreglo de operaciones
  ans -> op_count = 7;
  ans -> operations[0] = op_drop_and_add;
  ans -> operations[1] = op_swap;
  ans -> operations[2] = op_irrr;
  ans -> operations[3] = op_irre;
  ans -> operations[4] = op_irmrr;
  ans -> operations[5] = op_irmre;
  ans -> operations[6] = op_delete;

  // Inicializo el arreglo de probabilidades
  double initial_weight = 1.0;
  for (int i = 0; i < ans -> op_count; i++)
  {
    ans -> prob_weights[i] = initial_weight;
    ans -> operation_count[i] = 0;
  }
  ans -> total_weight = 7;

  // Creo los arreglos con las rutas para cada avion
  ans -> route_array_size = malloc(sizeof(int) * airplanes_count);
  ans -> route_count = malloc(sizeof(int) * airplanes_count);
  ans -> routes = malloc(sizeof(Route**) * airplanes_count);
  int initial_size = 16;
  for (int i = 0; i < airplanes_count; i++)
  {
    ans -> route_array_size[i] = initial_size;
    ans -> route_count[i] = 0;
    ans -> routes[i] = malloc(sizeof(Route*) * initial_size);
  }

  // Retorno el ans
  return ans;
}

/** Libera la memoria asociada */
void ans_destroy(ANS* ans)
{
  // Libera el mapa
  map_destroy(ans -> map);

  // Libera la planificacion base
  bp_destroy(ans -> bp);

  // Libero las rutas
  for (int k = 0; k < airplanes_count; k++)
  {
    for (int i = 0; i < ans -> route_count[k]; i++)
    {
      route_destroy(ans -> routes[k][i]);
    }
    free(ans -> routes[k]);
  }
  free(ans -> routes);
  free(ans -> route_array_size);
  free(ans -> route_count);

  // Libera el ans
  free(ans);
}

/** Agrega una ruta al avion k */
void insert_route(ANS* ans, Route* new_route, int k)
{
  // Si el arreglo k esta lleno, duplico su tamanio
  if (ans -> route_count[k] == ans -> route_array_size[k])
  {
    // Creo un arreglo nuevo
    Route** new_array = malloc(sizeof(Route*) * ans -> route_array_size[k] * 2);

    // Traspaso las rutas de un arreglo al otro
    for (int r = 0; r < ans -> route_array_size[k]; r++)
    {
      new_array[r] = ans -> routes[k][r];
    }

    // Actualizo el tamanio del arreglo
    ans -> route_array_size[k] *= 2;
    // Cambio el arreglo por el nuevo
    free(ans -> routes[k]);
    ans -> routes[k] = new_array;
  }

  // Inserto la ruta
  ans -> routes[k][ans -> route_count[k]] = new_route;
  // Actualizo la cantidad de rutas actuales
  ans -> route_count[k]++;
}

/** Imprime el nombre de la operacion */
void operation_print(int i)
{
  if (i == 0) printf("DROP AND ADD: ");
  else if (i == 1) printf("SWAP: ");
  else if (i == 2) printf("IRRE: ");
  else if (i == 3) printf("IRRR: ");
  else if (i == 4) printf("IRMRE: ");
  else if (i == 5) printf("IRMRR: ");
  else printf("DELETE: ");
}

//////////////////////////////////////////////////////////////////////////
//               Funciones usadas en las heuristicas                    //
//////////////////////////////////////////////////////////////////////////

/** Funcion comparadora para ordenar segun el fee * weight */
static int compare(const void* a, const void* b)
{
  int fee1 = (*((Order**) a)) -> pickup -> fee * (*((Order**) a)) -> pickup -> total_weight;
  int fee2 = (*((Order**) b)) -> pickup -> fee * (*((Order**) b)) -> pickup -> total_weight;
  return fee1 - fee2;
}

/** Funcion comparadora para ordernan pickups segun utilidad */
static int utility_comp(const void* a, const void* b)
{
  double ut1 = (*(LNode**) a) -> utility;
  double ut2 = (*(LNode**) b) -> utility;
  if (ut1 < ut2) return -1;
  else if (ut1 == ut2) return 0;
  return 1;
}

/** Revisa si se genera una ruta mejor y valida, retorna true o false y guarda en of, el valor actual */
static bool improved(Route* route, ANS* ans, double best_of)
{
  // Comparo con la fo greedy
  if (route -> fast_of > best_of)
  {
    // Veo si la ruta es valida
    if (assign_weights(route) && assign_time(route))
    {
      // Si mejoro, retorno true
      route -> objective_function = objective_function(route, ans -> map);
      if (route -> objective_function > best_of)
      {
        return true;
      }
    }
  }
  return false;
}

/** Revisa que la asignacion del nodo puede hacerse en el tiempo dado */
static bool is_time_consistent(LNode* ln1, LNode* ln2)
{
  // Compruebo que al insertar ln2 a la derecha de ln1 puedan calzar los tiempos
  double start = ln1 -> node -> start_time;
  double dist = distance(ln1 -> node -> father, ln2 -> node -> father);
  double end = ln2 -> node -> end_time;

  // Optimistamente puedo partir justo cuando comienza el start del primer nodo
  if (start + dist > end)
  {
    // Si se cumple este desigualdad quiere decir que no se puede
    return false;
  }
  return true;
}

/** Busca la mejor insercion y retorna la funcion objetivo de un pedido */
static double single_insert(Route* route, LNode* pickup_l_node, double best_of, ANS* ans)
{

  // Relleno los l_nodos a insertar en la lista
  LNode* delivery_l_node = pickup_l_node -> pair;

  // Almaceno la mejor solucion y las posiciones donde de obtuvo la mejor
  LNode* best_start = NULL;
  LNode* best_end = NULL;

  // Itero sobre los nodos de la ruta
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Compruebo si voy a tener una problema de tiempo antes de insertar
    if (is_time_consistent(ln, pickup_l_node) && is_time_consistent(pickup_l_node, ln -> next))
    {
      // Inserto el pickup
      l_node_insert_next(route, ln, pickup_l_node);

      // Itero sobre los nodos restantes desde el recien insertado
      for (LNode* ln2 = pickup_l_node; ln2 -> next; ln2 = ln2 -> next)
      {
        // Compruebo consistencia de tiempos
        if (is_time_consistent(ln2, delivery_l_node) && is_time_consistent(delivery_l_node, ln2 -> next))
        {
          // Inserto el delivery
          l_node_insert_next(route, ln2, delivery_l_node);

          // Comparo
          if (improved(route, ans, best_of))
          {
            best_of = route -> objective_function;
            best_start = pickup_l_node -> last;
            best_end = delivery_l_node -> last;
          }

          // Deshago la asignacion del delivery
          l_node_pop(route, delivery_l_node);
        }
      }

      // Deshago la asignacion del pickup
      l_node_pop(route, pickup_l_node);
    }
  }

  // Dejo en los lnodos de pickup y delivery las mejores asignaciones
  pickup_l_node -> last = best_start;
  delivery_l_node -> last = best_end;

  return best_of;
}

/** Busca el mejor swap para un pedido dado */
static double single_swap(Route* route, LNode* pickup_l_node, double best_of, ANS* ans)
{
  // Obtengo el lnode de delivery
  LNode* delivery_l_node = pickup_l_node -> pair;

  // Datos del mejor escenario hasta ahora
  LNode* best_start = NULL;
  LNode* best_end = NULL;

  // Itero sobre la ruta viendo donde puedo hacer swap
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si me topo con un nodo pickup, hago swap
    if (ln -> node -> node_type == PICKUP)
    {
      // Inserto el pickup
      l_node_insert_next(route, ln, pickup_l_node);

      // Saco el pickup anterior
      l_node_pop(route, ln);

      // Acto seguido inserto el delivery
      l_node_insert_next(route, ln -> pair, delivery_l_node);

      // Saco el delivery anterior
      l_node_pop(route, ln -> pair);

      if (improved(route, ans, best_of))
      {
        best_of = route -> objective_function;
        best_start = ln;
        best_end = ln -> pair;
      }

      // Deshago los swaps:
      // Inserto el pickup viejo
      l_node_insert_next(route, pickup_l_node, ln);

      // Saco el pickup insertado
      l_node_pop(route, pickup_l_node);

      // Acto seguido inserto el delivery viejo
      l_node_insert_next(route, delivery_l_node, ln -> pair);

      // Saco el delivery insertado
      l_node_pop(route, delivery_l_node);
    }
  }

  // Dejo asignada la mejor opcion
  pickup_l_node -> last = best_start;
  delivery_l_node -> last = best_end;

  // retorno la funcion objetivo
  return best_of;
}

/** Rellena un arreglo con las ordenes disponibles ordenadamente y retorna cuantos son */
static int unassigned_orders(Route* route, Order** all_orders, Order** unassigned)
{
  // Contador de pedidos no asignadas
  int unassigned_count = order_count;

  // Itero sobre las ordenes y marco los nodos de los pedidos no asignados
  for (int i = 0; i < order_count; i++)
  {
    all_orders[i] -> pickup -> assigned = false;
  }

  // Itero sobre la ruta y marco los pedidos asignados
  for (LNode* ln = route -> nodes -> start -> next; ln -> next; ln = ln -> next)
  {
    if (ln -> node -> node_type == PICKUP)
    {
      ln -> node -> assigned = true;
      unassigned_count--;
    }
  }

  // Itero sobre las ordenes y traspaso al arreglo las ordenes no asignadas
  int position = 0;
  for (int i = 0; i < order_count; i++)
  {
    if (!all_orders[i] -> pickup -> assigned)
    {
      unassigned[position] = all_orders[i];
      position++;
    }
  }

  // Ordeno las ordenes segun su ganancia
  qsort(unassigned, unassigned_count, sizeof(Order*), compare);

  // Retorno el contador
  return unassigned_count;
}

/** Cuenta los pedidos abiertos y cerrados en toda la ruta */
static void mark_sub_routes(Route* route)
{
  // variable que cuenta los pedidos en el avion
  int open_orders_count = 0;

  // Itero sobre la ruta contando los pedidos en el avion en cada momento
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {

    // Le agrego cuantos pedidos hay
    ln -> open_orders_count = open_orders_count;
    // Si el pickup le sumo al numero de pedidos en el avion
    if (ln -> node -> node_type == PICKUP)
    {
      open_orders_count++;
    }
    // Si el delivery, le resto al numero de pedidos en el avion
    else if (ln -> node -> node_type == DELIVERY)
    {
      open_orders_count--;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
//                  Operaciones de la heurística                        //
//////////////////////////////////////////////////////////////////////////

/** Agregar un pedido pendiente */
void op_drop_and_add(Route* route, ANS* ans)
{
  // route: es la ruta a modificar
  // ans: contiene la informacion del mapa y de la planificacion base

  // Tomo la funcion objetivo original
  double old_of = route -> objective_function;

  // Creo los lnodos a usar en las inserciones
  LNode* pickup_l_node = l_node_init();

  // Aqui almaceno las variables de las mejores asignaciones
  double best_of = old_of;
  Order* best_order = NULL;
  LNode* best_start = NULL;
  LNode* best_end = NULL;

  // Obtengo los pedidos sin asignar ordenados
  Order** unassigned = malloc(sizeof(Order*) * order_count);
  int unassigned_count = unassigned_orders(route, ans -> map -> orders, unassigned);

  // Itero sobre los pedidos insertando uno por uno
  for (int i = 0; i < unassigned_count; i++)
  {
    // Podo de inmediato si no puedo ganar mas que el best_of
    int fee = unassigned[i] -> pickup -> fee;
    int total_weight = unassigned[i] -> pickup -> total_weight;
    if (fee * total_weight + route -> fast_of < best_of)
    {
      break;
    }
    // Relleno el lnode de la orden
    l_node_order_fill(pickup_l_node, unassigned[i]);

    // Pruebo con el pedido en todas las posiciones posibles
    double new_of = single_insert(route, pickup_l_node, best_of, ans);

    // Veo si es la mejor hasta le momento
    if (new_of > best_of)
    {
      // Actualizo las variables con los mejores resultados
      best_of = new_of;
      best_order = unassigned[i];
      best_start = pickup_l_node -> last;
      best_end = pickup_l_node -> pair -> last;
    }
  }

  // Si mejore, dejo la mejor asignacion en la ruta
  if (best_of > old_of)
  {
    pickup_l_node -> node = best_order -> pickup;
    pickup_l_node -> pair -> node = best_order -> delivery;
    l_node_insert_next(route, best_start, pickup_l_node);
    l_node_insert_next(route, best_end, pickup_l_node -> pair);
    route -> objective_function = best_of;
  }
  else
  {
    // libero los l_nodes (ya que no nos necesito)
    free(pickup_l_node -> pair);
    free(pickup_l_node);
  }

  // Libero la memoria usada
  free(unassigned);

  // Dejo la funcion objetivo consistente
  route -> objective_function = objective_function(route, ans -> map);
}

/** Cambiar un pedido de la ruta por uno pendiente */
void op_swap(Route* route, ANS* ans)
{
  // route: ruta a modificar
  // orders: todos los pedidos
  // order_count: cantidad de pedidos
  // ans: contiene la informacion del mapa y de la planificacion base

  // Tomo la funcion objetivo original
  double old_of = route -> objective_function;

  // Creo los lnodos a usar en las inserciones
  LNode* pickup_l_node = l_node_init();

  // Aqui almaceno las variables de las mejores asignaciones
  double best_of = old_of;
  Order* best_order = NULL;
  LNode* best_start = NULL;
  LNode* best_end = NULL;

  // Obtengo los pedidos sin asignar ordenados
  Order** unassigned = malloc(sizeof(Order*) * order_count);
  int unassigned_count = unassigned_orders(route, ans -> map -> orders, unassigned);

  // Itero sobre los pedidos pendientes
  for (int i = 0; i < unassigned_count; i++)
  {
    // Relleno el lnodo con el pedido a insertar
    l_node_order_fill(pickup_l_node, unassigned[i]);

    // Hago swap de este nodo en todas las posiciones posibles
    double new_of = single_swap(route, pickup_l_node, best_of, ans);

    // Actualizo los mejores resultados
    if (new_of > best_of)
    {
      best_of = new_of;
      best_order = unassigned[i];
      best_start = pickup_l_node -> last;
      best_end = pickup_l_node -> pair -> last;
      route -> objective_function = best_of;
    }
  }

  // Si mejore, dejo swapeada la mejor opcion
  if (best_of > old_of)
  {
    l_node_order_fill(pickup_l_node, best_order);
    l_node_insert_next(route, best_start, pickup_l_node);
    l_node_pop(route, best_start);
    l_node_insert_next(route, best_end, pickup_l_node -> pair);
    l_node_pop(route, best_end);
    route -> objective_function = best_of;
  }
  else
  {
    // Libero el par de l_nodes
    free(pickup_l_node -> pair);
    free(pickup_l_node);
  }

  // Libero la memoria pedida
  free(unassigned);

  // Dejo la funcion objetivo consistente
  route -> objective_function = objective_function(route, ans -> map);
}

/** Intra route request exchange: cambia dos pedidos dentro de la ruta */
void op_irre(Route* route, ANS* ans)
{
  // route: es la ruta a alterar
  // ans: contiene la informacion del mapa y de la planificacion base

  // Funcion objetivo original
  double old_of = route -> objective_function;

  // Mejores valores
  double best_of = old_of;
  LNode* best_pickup1 = NULL;
  LNode* best_pickup2 = NULL;

  // Primero itero sobre la ruta hasta encontrar un pickup
  for (LNode* ln = route -> nodes -> start -> next; ln -> next; ln = ln -> next)
  {
    // Veo si es de pickup
    if (ln -> node -> node_type == PICKUP)
    {
      // Itero sobre los siguientes nodos para hacer el swap
      for (LNode* ln2 = ln -> next; ln2 -> next; ln2 = ln2 -> next)
      {
        // Si es de pickup, hago el cambio
        if (ln2 -> node -> node_type == PICKUP)
        {
          // Intercambio los pickups
          l_nodes_swap(route, ln, ln2);
          // Intercambio los deliverys
          l_nodes_swap(route, ln -> pair, ln2 -> pair);


          // Si mejore, actualizo los datos
          if (improved(route, ans, best_of))
          {
            // Actualizo la mejor of
            best_of = route -> objective_function;
            // Actualizo los mejores pedidos a intercambiar
            best_pickup1 = ln;
            best_pickup2 = ln2;
          }

          // Deshago el intercambio
          l_nodes_swap(route, ln2, ln);
          l_nodes_swap(route, ln2 -> pair, ln -> pair);
        }
      }
    }
  }

  // Si mejore, dejo el cambio hecho
  if (best_of > old_of)
  {
    // Intercambio los pickups
    l_nodes_swap(route, best_pickup1, best_pickup2);

    // Intercambio los deliverys
    l_nodes_swap(route, best_pickup1 -> pair, best_pickup2 -> pair);

    // Dejo la funcion objetivo correcta
    route -> objective_function = best_of;
  }

  // Dejo la funcion objetivo consistente
  route -> objective_function = objective_function(route, ans -> map);
}

/** Intra route request relocate: cambia de posicion un pedido dentro de la ruta */
void op_irrr(Route* route, ANS* ans)
{
  // route: la ruta a modificar
  // ans: contiene la informacion del mapa y de la planificacion base

  // Guardo la funcion objetivo vieja
  double old_of = route -> objective_function;

  // Guardo los mejores resultados
  double best_of = route -> objective_function;
  LNode* best_start = NULL;
  LNode* best_end = NULL;
  LNode* best_pickup = NULL;

  // Itero sobre los pedidos de la ruta para cambiar su posicion
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup saco el pedido de la ruta
    if (ln -> node -> node_type == PICKUP)
    {
      // Creo las variables de pickup y delivery a mover
      LNode* pickup_ln = ln;
      LNode* delivery_ln = ln -> pair;

      // Saco el pickup y el delivery guardando sus posiciones originales
      // Saco el pickup
      LNode* original_start = pickup_ln -> last;
      l_node_pop(route, pickup_ln);
      // Saco el delivery
      LNode* original_end = delivery_ln -> last;
      l_node_pop(route, delivery_ln);

      // Inserto el pickup y el delivery en sus mejores posiciones en la ruta
      double actual_of = single_insert(route, pickup_ln, best_of, ans);

      // Si mejoro, actualizo los valores de mejor resultado
      if (actual_of > best_of)
      {
        best_of = actual_of;
        best_start = pickup_ln -> last;
        best_end = pickup_ln -> pair -> last;
        best_pickup = pickup_ln;
      }

      // Vuelvo a poner el pedido en su posicion original (en el orden contrario)
      l_node_insert_next(route, original_end, delivery_ln);
      l_node_insert_next(route, original_start, pickup_ln);
    }
  }

  // Si mejoro, dejo asignado el mejor cambio
  if (best_of > old_of)
  {
    // Saco los nodos de sus posiciones
    l_node_pop(route, best_pickup);
    l_node_pop(route, best_pickup -> pair);

    // Los inserto donde deben estar
    l_node_insert_next(route, best_start, best_pickup);
    l_node_insert_next(route, best_end, best_pickup -> pair);

    // Hago que la funcion objetivo sea consistente
    route -> objective_function = best_of;
  }


  // Dejo la funcion objetivo consistente
  route -> objective_function = objective_function(route, ans -> map);
}

/** Intra route multiple request exchange: cambia dos subrutas truncadas*/
void op_irmre(Route* route, ANS* ans)
{
  // route: ruta a la cual hago la operacion
  // ans: contiene la informacion del mapa y de la planificacion base

  // Detecto las subrutas truncadas
  mark_sub_routes(route);

  // Guardo la funcion objetivo original
  double old_of = route -> objective_function;

  // Guardo los mejores resultados
  double best_of = old_of;
  LNode* best_pickup1 = NULL;
  LNode* best_pickup2 = NULL;

  // Itero sobre la ruta
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup y forma una subruta truncada
    if (ln -> node -> node_type == PICKUP &&
       ln -> open_orders_count == ln -> pair -> open_orders_count - 1)
    {
      // Itero sobre los nodos que quedan a la derecha de la subruta
      for (LNode* ln2 = ln -> pair -> next; ln2 -> next; ln2 = ln2 -> next)
      {
        // Si encuentro otra subruta:
        if (ln2 -> node -> node_type == PICKUP &&
           ln2 -> open_orders_count == ln2 -> pair -> open_orders_count - 1)
        {
          // Intercambio las subrutas
          sub_route_swap(route, ln, ln2);

          // Si mejore, actualizo los mejores resultados
          if (improved(route, ans, best_of))
          {
            best_of = route -> objective_function;
            best_pickup1 = ln;
            best_pickup2 = ln;
          }

          // Restauro las posiciones
          sub_route_swap(route, ln2, ln);
        }
      }
    }
  }

  // Si mejoro, dejo los mejores resultados en la ruta
  if (best_of > old_of)
  {
    sub_route_swap(route, best_pickup1, best_pickup2);
    // Dejo consistente la funcion objetivo
    route -> objective_function = best_of;
  }

  // Dejo la funcion objetivo consistente
  route -> objective_function = objective_function(route, ans -> map);
}

/** Intra route multiple request relocate: mueve una subruta trucada */
void op_irmrr(Route* route, ANS* ans)
{
  // route: ruta a la cual hago la operacion
  // ans: contiene la informacion del mapa y de la planificacion base

  // Detecto las subrutas truncadas
  mark_sub_routes(route);

  // Guardo la funcion objetivo vieja
  double old_of = route -> objective_function;

  // Guardo los mejores valores
  double best_of = old_of;
  LNode* best_pickup = NULL;
  LNode* best_start = NULL;

  // itero sobre la ruta buscando subrutas truncadas
  for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
  {
    // Si es pickup y forma una subruta truncada
    if (ln -> node -> node_type == PICKUP &&
       ln -> open_orders_count == ln -> pair -> open_orders_count - 1)
    {
      // Obtengo la posicion original de la subruta
      LNode* original_position = ln -> last;

      // Saco la subruta
      sub_route_pop(route, ln);

      // Itero sobre la ruta para insertar la subruta
      for (LNode* ln2 = route -> nodes -> start; ln2 -> next; ln2 = ln2 -> next)
      {
        // Inserto la subruta truncada
        sub_route_insert_next(route, ln2, ln);

        // Si mejore, actualizo los datos
        if (improved(route, ans, best_of))
        {
          best_of = route -> objective_function;
          best_pickup = ln;
          best_start = ln2;
        }

        // Deshago la insercion
        sub_route_pop(route, ln);
      }

      // Vuelvo a poner la subruta en su lugar inicial
      sub_route_insert_next(route, original_position, ln);
    }
  }

  // Si mejore, muevo la ruta a su mejor posicion
  if (best_of > old_of)
  {
    sub_route_pop(route, best_pickup);
    sub_route_insert_next(route, best_start, best_pickup);
    // Dejo consistente la funcion objetivo
    route -> objective_function = best_of;
  }

  // Dejo la funcion objetivo consistente
  route -> objective_function = objective_function(route, ans -> map);
}

/** Elimina pedidos de la ruta hasta que no mejore */
void op_delete(Route* route, ANS* ans)
{
  // route: ruta a la cual le voy a sacar pedidos
  // ans: contiene la informacion del mapa y de la planificacion base

  // Guardo los mejores valores hasta el momento
  double best_of = route -> objective_function;
  LNode* best_pickup = NULL;
  LNode* best_delivery = NULL;

  // Indica si sigo iterando
  bool iter = true;
  // Cada iteracion saca un solo pedido (termino si no mejora)
  while (iter)
  {
    // Por ahora no he mejorado
    iter = false;
    // Itero sobre la ruta eliminando pedidos
    for (LNode* ln = route -> nodes -> start; ln; ln = ln -> next)
    {
      // Si es un pickup
      if (ln -> node -> node_type == PICKUP)
      {
        LNode* pick = ln;
        LNode* deli = pick -> pair;

        // Elimino el pedido
        l_node_pop(route, pick);
        l_node_pop(route, deli);

        // Si mejore, actualizo los valores del mejor resultado
        if (improved(route, ans, best_of))
        {
          best_of = route -> objective_function;
          best_pickup = pick;
          best_delivery = deli;
          iter = true;
        }

        // Devuelvo los nodos (En el orden contrario)
        l_node_insert_next(route, deli -> last, deli);
        l_node_insert_next(route, pick -> last, pick);
      }
    }

    // Si mejore, dejo el nodo eliminado
    if (iter)
    {
      // Elimino el pedido
      l_node_pop(route, best_pickup);
      l_node_pop(route, best_delivery);
      // Libero la memoria
      free(best_pickup -> pair);
      free(best_pickup);
    }
  }

  // Dejo la funcion objetivo correcta
  route -> objective_function = objective_function(route, ans -> map);
}

/** Hace drop and add hasta que no puede mejorar */
Route* initial_drop_and_add(Route* base, ANS* ans)
{
  // base: es la ruta a copiar y modificar
  // ans: contiene la informacion del mapa y de la planificacion base

  // Copio la ruta
  Route* route = route_copy(base, ans -> map);

  // Hago drop_and_add hasta que no mejore
  double OF = route -> objective_function;
  while (true)
  {
    // printf("\t\t\tDrop And Add\n");
    op_drop_and_add(route, ans);
    route -> objective_function = objective_function(route, ans -> map);
    if (route -> objective_function <= OF) break;
    OF = route -> objective_function;
  }

  // Retorno la ruta
  return route;
}

/** Hace swap hasta que no puede mejorar */
Route* initial_swap(Route* base, ANS* ans)
{
  // base: es la ruta a copiar y modificar
  // ans: contiene la informacion del mapa y de la planificacion base

  // Copio la ruta
  Route* route = route_copy(base, ans -> map);

  // Hago swap hasta que no mejore
  double OF = route -> objective_function;
  while (true)
  {
    op_swap(route, ans);
    route -> objective_function = objective_function(route, ans -> map);
    if (route -> objective_function <= OF) break;
    OF = route -> objective_function;
  }

  // Retorno la ruta
  return route;
}

/** Inserta los pedidos que pueda y retorna la cantidad que no pudo insertar */
int initial_insert(Route* route, int unnasigned, Order** unassigned_orders, ANS* ans)
{
  // Primero calculo la utilidad de la ruta
  double best_utility = utility(route);

  // Itero sobre los pedidos
  int i = 0;
  while (i < unnasigned)
  {
    // Tomo el pedido a insertar
    Order* order = unassigned_orders[i];

    // Creo los lnodos para el pickup y el delivery y los relleno con la orden
    LNode* pickup = l_node_init();
    LNode* delivery = pickup -> pair;
    l_node_order_fill(pickup, order);

    // Guardo la mejor posicion hasta ahora del pickup y delivery
    LNode* best_p = NULL;
    LNode* best_d = NULL;

    // Itero sobre la ruta para insertar el pickup
    for (LNode* ln = route -> nodes -> start; ln -> next; ln = ln -> next)
    {
      // Si es consistente con los tiempos
      if (is_time_consistent(ln, pickup) && is_time_consistent(pickup, ln -> next))
      {
        // Inserto el pickup
        l_node_insert_next(route, ln, pickup);

        // Ahora itero sobre el resto de la ruta para insertar el delivery
        for (LNode* ln2 = pickup; ln2 -> next; ln2 = ln2 -> next)
        {
          // Si es consistente en tiempos
          if (is_time_consistent(ln2, delivery) && is_time_consistent(delivery, ln2 -> next))
          {
            // Inserto el delivery
            l_node_insert_next(route, ln2, delivery);

            // Si la ruta obtenida es valida en tiempos y en pesos
            if (assign_time(route) && assign_weights(route))
            {
              // Calculo su utilidad
              int util = utility(route);

              // Si su utilidad es mayor a la anterior
              if (util > best_utility)
              {
                // Actualizo la mejor utilidad
                best_utility = util;

                // Guardo las posiciones donde los inserte
                best_p = ln;
                best_d = ln2;
              }
            }

            // Saco el delivery
            l_node_pop(route, delivery);
          }
        }

        // Saco el pickup
        l_node_pop(route, pickup);
      }
    }

    // Si encontre una posicion para insertalos
    if (best_p)
    {
      // Los pongo en sus posiciones
      l_node_insert_next(route, best_p, pickup);
      l_node_insert_next(route, best_d, delivery);

      // Elimino la orden del arreglo
      for (int j = i + 1; j < unnasigned; j++)
      {
        unassigned_orders[j - 1] = unassigned_orders[j];
      }

      // No hago avanzar el i ya que movi los pedidos
      unnasigned--;
    }
    // Si no encontre una posicion para insertarlo
    else
    {
      // Libero los l_nodos
      free(pickup);
      free(delivery);

      // Prosigo a la siguiente order
      i++;
    }
  }

  // Dejo como valida a la ruta
  route -> valid = true;

  // Retorno la cantidad de pedidos no asignados
  return unnasigned;
}

/////////////////////////////////////////////////////////////////////////
//                    Funciones propias del ans                        //
/////////////////////////////////////////////////////////////////////////


/** Actualiza las probabilidades dada una operacion */
void refresh_probabilities(ANS* ans, int operation_id, double old_of, double new_of, double op_time)
{
  int old_e = ans -> prob_weights[operation_id];
  if (old_of < new_of) ans -> prob_weights[operation_id] *= 2;
  else ans -> prob_weights[operation_id] -= 1;
  if (ans -> prob_weights[operation_id] > 50) ans -> prob_weights[operation_id] = 50;
  else if (ans -> prob_weights[operation_id] < 1) ans -> prob_weights[operation_id] = 1;
  ans -> total_weight = ans -> total_weight - old_e + ans -> prob_weights[operation_id];
  return;
}

/** Inicializa una ruta de la planificacion base eliminando pedidos */
Route* initialize(ANS* ans, Route* original_route)
{
  // Creo una copia de la ruta
  Route* route = route_copy(original_route, ans -> map);

  // Calculo la funcion objetivo y declaro que es la mejor hasta ahora
  double old_of = route -> objective_function;
  double best_of = old_of;

  // Cuento cuantos pedidos hay en la ruta
  // Ademas calculo la utilidad de los pedidos
  int order_n = 0;
  for (LNode* ln = route -> nodes -> start -> next; ln -> next; ln = ln -> next)
  {
    if (ln -> node -> node_type == PICKUP)
    {
      order_n++;
      Node* node = ln -> node;
      double dist = distance(ln -> last -> node -> father, node -> father);
      ln -> utility = (node -> fee - node -> dual_pi) * node -> total_weight - dist;
    }
  }

  // Creo un arreglo de pickups
  LNode** pickup_array = malloc(sizeof(LNode*) * order_n);

  // Relleno el arreglo de pickups iterando sobre la ruta
  int position = 0;
  for (LNode* ln = route -> nodes -> start -> next; ln -> next; ln = ln -> next)
  {
    if (ln -> node -> node_type == PICKUP)
    {
      pickup_array[position] = ln;
      position++;
    }
  }

  // Ordeno el arreglo segun su utilidad
  qsort(pickup_array, order_n, sizeof(LNode*), utility_comp);

  // Itero sobre el arreglo ordenado eliminando el pedido
  for (int i = 0; i < order_n; i++)
  {
    LNode* pickup = pickup_array[i];
    LNode* delivery = pickup -> pair;

    // Guardo las posiciones
    LNode* pickup_position = pickup -> last;
    LNode* delivery_position = delivery -> last;

    // Elimino el pedido
    l_node_pop(route, pickup);
    l_node_pop(route, delivery);

    // Si mejore, actualizo la mejor fo y libero los l_nodos
    if (improved(route, ans, best_of))
    {
      best_of = route -> objective_function;
      free(pickup);
      free(delivery);
    }
    // Si no mejore, deshago la eliminacion
    else
    {
      l_node_insert_next(route, pickup_position, pickup);
      l_node_insert_next(route, delivery_position, delivery);
    }
  }

  // libero la memoria usada
  free(pickup_array);

  // Retorno la ruta
  return route;
}

/** Ejecuta la heuristica */
Route* run(ANS* ans, Route* original_route)
{

  // Hago las operaciones iniciales sobre la ruta
  Route* route = initialize(ans, original_route);

  // Guardo la mejor funcion objetivo
  double best_of = route -> objective_function;

  // Numero de iteraciones maximas
  int max_iter = 35;

  // Arreglo que indica que operaciones ya se han hecho
  int bloq[7] = { 0, 0, 0, 0, 0, 0, 0 };

  //Itero varias veces (max_iter veces)
  for (int iter = 0; iter < max_iter; iter++)
  {
    // Elijo una operacion aleatoriamente
    int operation_id = 6;
    double random_value = random_bounded(ans -> total_weight);
    double sum = 0;
    for (int op_id = 0; op_id < 7; op_id++)
    {
      sum += ans -> prob_weights[op_id];
      if (sum >= random_value)
      {
        operation_id = op_id;
        break;
      }
    }

    // Si la operacion no esta bloqueada
    if (bloq[operation_id] == 0)
    {
      // Hago la operacion calculando su tiempo
      double start_clock = clock();
      ans -> operations[operation_id](route, ans);
      double end_clock = clock();

      // Calculo el tiempo
      double op_time = (end_clock - start_clock) / CLOCKS_PER_SEC;

      // Actualizo las probablidades
      refresh_probabilities(ans, operation_id, best_of, route -> objective_function, op_time);

      // Cuento uno para la operacion
      ans -> operation_count[operation_id]++;

      // Si no mejore, bloqueo la operacion
      if (route -> objective_function <= best_of)
      {
        bloq[operation_id] = 1;
      }
      else
      {
        best_of = route -> objective_function;
        route -> valid = true;
      }
    }
  }

  assign_time(route);
  assign_weights(route);
  route -> objective_function = objective_function(route, ans -> map);
  // Retorno la ruta final
  return route;
}
