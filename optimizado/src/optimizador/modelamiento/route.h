#pragma once

#include "map.h"
#include "dictionary.h"
#include <stdio.h>

/** Nodo de la lista */
struct l_node
{
  /** l_nodo siguiente */
  struct l_node* next;
  /** l_nodo anterior */
  struct l_node* last;
  /** l_nodo par (pickup-delivery o start-end) */
  struct l_node* pair;
  /** nodo del mapa que contiene */
  Node* node;
  /** Indica la cantidad de pedidos que hay en el avion en ese momento */
  int open_orders_count;
  /** Indica el peso actual que lleva el avion en ese momento */
  double actual_weight;
  /** Indica el peso agregado o dejado en este nodo */
  double delta_weight;
  /** Tarifa - costo dual - costo del ultimo viaje */
  double utility;
  /** Indice del nodo dentro de la ruta (se usa al optimizar con gurobi) */
  int route_index;
  /** Momento en que llega el avion */
  double arrive_time;
  /** Momento en que se va el avion */
  double leave_time;
};
/** Nodo de la lista */
typedef struct l_node LNode;

/** Lista diseñada para manejar las operaciones de las rutas de manera eficiente */
struct list
{
  /** LNodo inicial de la lista */
  LNode* start;
  /** LNodo final de la lista */
  LNode* end;
};
/** Lista diseñada para manejar las operaciones de las rutas de manera eficiente */
typedef struct list List;

/** Estructura que maneja los nodos de una ruta en particular */
struct route
{
  /** Lista con los nodos de la ruta */
  List* nodes;
  /** Valor de la funcion objetivo actual de la ruta */
  double objective_function;
  /** Valor optimista de la funcion objetivo (rapido de calcular) */
  double fast_of;
  /** Indica si la ruta es valida */
  bool valid;
  /** Avion que realiza esta ruta */
  Airplane* airplane;
  /** Variable que determina si la ruta es de la planificacion base o no */
  bool is_bp;
  /** Diccionario que contiene los vuelos de la PB de este avion (para calcular costos de cancelacion eficientemente) */
  Dictionary* cancelled_flight;
};
/** Estructura que maneja los nodos de una ruta en particular */
typedef struct route Route;

/** Estructura que maneja la planificacion base */
struct bp
{
  /** Cantidad de rutas en la planificacion */
  int route_count;
  /** Arreglo con las rutas de la planificacion base */
  Route** routes;
};

/** Estructura que maneja la planificacion base */
typedef struct bp BP;

/////////////////////////////////////////////////////////////////////////
//                             Funciones                               //
/////////////////////////////////////////////////////////////////////////



// FUNCIONES BASICAS DE RUTAS

/** Crea una ruta nueva vacia */
Route* route_init(Airplane* airplane);

/** Imprime la ruta */
void route_print(Route* route, FILE* file);

/** Copia una ruta */
Route* route_copy(Route* route, Map* map);

/** Libera la memoria de una ruta */
void route_destroy(Route* route);



// FUNCIONES DE LA PLANIFICACION BASE

/** Crea la planificacion base a partir del archivo */
BP* bp_init(char* bp_file_path, Map* map);

/** Libera la memoria asociada a la planificacion base */
void bp_destroy(BP* bp_destroy);



// FUNCIONES QUE ALTERAN LAS RUTAS Y CAMBIAN LA FUNCION OBJETIVO RAPIDA

/** Inserta un nodo en la posicion siguiente al lnodo dado */
void l_node_insert_next(Route* route, LNode* original_l_node, LNode* new_l_node);

/** Desconecta el l_nodo de sus hermanos y deja a sus hermanos conectados */
void l_node_pop(Route* route, LNode* l_node);

/** Intercambia dos nodos dentro de una ruta */
void l_nodes_swap(Route* route, LNode* ln1, LNode* ln2);

/** Inserta una subruta en la ruta en la posicion dada */
void sub_route_insert_next(Route* route, LNode* position, LNode* pickup);

/** Elimina una subruta de la ruta */
void sub_route_pop(Route* route, LNode* pickup);

/** Intercambia dos subrutas truncadas */
void sub_route_swap(Route* route, LNode* ln1, LNode* ln2);



// FUNCIONES PARA CREAR LNODOS EN UNA RUTA

/** Crea dos LNodos enparejados vacios y retorna el primero */
LNode* l_node_init();

/** Asigna nodos en un par de LNodos a partir de un pedido */
void l_node_order_fill(LNode* pickup, Order* order);



// FUNCIONES SOBRE LAS RUTAS USADAS EN EL CODIGO ANS

/** Calcula la funcion objetivo de una ruta */
double objective_function(Route* route, Map* map);

void print_objective_function(Route* route, Map* map);

/** Calcula la funcion objetivo de manera optimista (sin costos duales pi y asumiendo carga completa) */
double fast_of(Route* route, Map* map);

/** Calcula la utilidad de la ruta */
double utility(Route* route);

/** Calcula la utilidad de la ruta */
void print_utility(Route* route);

/** Ajusta los tiempos de una ruta y determina si es factible */
bool assign_time(Route* route);

/** Asigna las cargas de manera greedy, y si no se puede optimiza */
bool assign_weights(Route* route);

/** Asigna las cargas de manera greedy maximizando utilidad, y si no se puede optimiza */
bool utility_assign_weights(Route* route);
