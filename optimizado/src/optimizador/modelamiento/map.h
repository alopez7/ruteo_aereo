#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "airplane.h"

/** Variable global que indica la cantidad de pedidos */
int order_count;
/** Variable global que indica la cantidad de aviones */
int airplanes_count;
/** Variable global que indica el costo de penalizacion por cancelacion */
double penalization_cost;

/** distintos tipos de nodos */
enum type
{
  PICKUP,
  DELIVERY,
  START,
  END
};
/** distintos tipos de nodos */
typedef enum type Type;


/** estructura de un nodo del mapa */
struct node;
/** estructura de un nodo del mapa */
typedef struct node Node;


/** estructura de macronodo */
struct macronode
{
  /** ID del macro nodo */
  int id;
  /** posicion x */
  double X;
  /** posicion y */
  double Y;
  /** cantidad de nodos que tiene */
  int node_count;
  /** arreglo de nodos */
  Node** nodes;
};
/** estructura de macronodo */
typedef struct macronode Macronode;

/** estructura de un nodo del mapa */
struct node
{
  /** ID del nodo */
  int id;
  /** posicion x */
  double X;
  /** posicion y */
  double Y;
  /** Determina si es nodo pickup, delivery, start o end */
  Type node_type;
  /** demora en pickup o delivery */
  double delay_time;
  /** peso del pickup o delivery completo */
  double total_weight;
  /** hora inicio */
  double start_time;
  /** hora fin */
  double end_time;
  /** tarifa ganada por hacer la ruta */
  double fee;
  /** nodo "padre" de este (macronodo)*/
  Macronode* father;
  /** Indica si el nodo ya fue asignado a una ruta (para hacer array de ordenes faltantes) */
  bool assigned;
  /** Costo dual del nodo (solo si el pickup) */
  double dual_pi;
};

/** Par de nodos (pickup-delivery) (un pedido) */
struct order
{
  /** Nodo pickup de la orden */
  Node* pickup;
  /** Nodo delovery de la orden */
  Node* delivery;
};
/** Par de nodos (pickup-delivery) (un pedido) */
typedef struct order Order;

/** Par de nodos (start-end) (un comienzo y termino de listas) */
struct limit
{
  /** Nodo start de la ruta */
  Node* start;
  /** Nodo end de la ruta */
  Node* end;
};
/** Par de nodos (start-end) (un comienzo y termino de listas) */
typedef struct limit Limit;

/** mapa con todos los nodos e macronodos */
struct map
{
  /** pedidos */
  Order** orders;
  /** cantidad de inicios y fines */
  int limit_count;
  /** limites de pedidos */
  Limit** limits;
  /** cantidad de nodos */
  int node_count;
  /** arreglo de nodos */
  Node** nodes;
  /** cantidad de macronodos */
  int macronode_count;
  /** arreglo de macronodos*/
  Macronode** macronodes;
  /** matriz de MNodo X MNodo que dice en que ruta esta originalmente el vuelo */
  uint16_t** original_flights;
  /** Arreglo con las aviones */
  Airplane** airplanes;
};
/** mapa con todos los nodos e macronodos */
typedef struct map Map;


///////////////////////////////////////////////////////////////////////////////
//                        metodos publicos                                   //
///////////////////////////////////////////////////////////////////////////////


/** Metodo que parsea el input y crea el mapa completo */
Map* map_init(char* file_path, char* airplanes_path);

/** Metodo que libera toda la memoria asociada el mapa */
void map_destroy(Map* map);

/** distancia euclideana de un macro nodo a otro */
double distance(Macronode* macro1, Macronode* macro2);

/** distancia euclideana de un macro nodo a otro */
double print_distance(Macronode* macro1, Macronode* macro2);
