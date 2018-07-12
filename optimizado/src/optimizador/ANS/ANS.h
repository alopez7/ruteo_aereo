#pragma once

#include "../modelamiento/route.h"

struct ans;
typedef struct ans ANS;

// Tipo de operaciones (para hacer un arreglo con las operaciones)
typedef void (*op_func_t)(Route*, ANS*);

/** estuctura que maneja todas las operaciones y estados de una ruta */
struct ans
{
  /** Contiene el mapa con toda la informacion de las rutas y nodos */
  Map* map;
  /** Contiene la planificacion base */
  BP* bp;
  /** Cantidad de operaciones disponibles */
  int op_count;
  /** Arreglo de operaciones */
  op_func_t operations[7];
  /** Pesos probabilisticos de las distintas operaciones */
  double prob_weights[7];
  /** Peso probabilistico total */
  double total_weight;
  /** Arreglo con las rutas actuales separado por avion*/
  Route*** routes;
  /** Cantidad de rutas que tengo actualmente para cada avion*/
  int* route_count;
  /** Tamanio del arreglo de rutas actualemnte para cada avion*/
  int* route_array_size;
  /** Cantidad de veces que se ha ejecutado cada operacion */
  int operation_count[7];
};

//////////////////////////////////////////////////////////////////////////
//                        Funciones del ans                             //
//////////////////////////////////////////////////////////////////////////

/** Crea la estructura ANS del programa */
ANS* ans_init(char* orders_filepath, char* airplanes_filepath, char* bp_filepath);

/** Libera la memoria asociada */
void ans_destroy(ANS* ans);

/** Agrega una ruta al avion k */
void insert_route(ANS* ans, Route* new_route, int k);

/** Imprime el nombre de la operacion */
void operation_print(int i);

//////////////////////////////////////////////////////////////////////////
//                  Operaciones de la heurística                        //
//////////////////////////////////////////////////////////////////////////

/** Agregar un pedido pendiente */
void op_drop_and_add(Route* route, ANS* ans);

/** Cambiar un pedido de la ruta por uno pendiente */
void op_swap(Route* route, ANS* ans);

/** Intra route request exchange: cambia dos pedidos dentro de la ruta */
void op_irre(Route* route, ANS* ans);

/** Intra route request relocate: cambia de posicion un pedido dentro de la ruta */
void op_irrr(Route* route, ANS* ans);

/** Intra route multiple request exchange: cambia dos subrutas truncadas*/
void op_irmre(Route* route, ANS* ans);

/** Intra route multiple request relocate: mueve una subruta trucada */
void op_irmrr(Route* route, ANS* ans);

/** Elimina pedidos de la ruta hasta que no mejore */
void op_delete(Route* route, ANS* ans);

/** Version especial de drop and add para rutas iniciales */
Route* initial_drop_and_add(Route* base, ANS* ans);

/** Version especial de swap para rutas iniciales */
Route* initial_swap(Route* base, ANS* ans);

/** Inserta los pedidos que pueda y retorna la cantidad que no pudo insertar */
int initial_insert(Route* route, int unnasigned, Order** unassigned_orders, ANS* ans);

/////////////////////////////////////////////////////////////////////////
//                          Optimizaciones                             //
/////////////////////////////////////////////////////////////////////////


/** Actualiza las probabilidades dada una operacion */
void refresh_probabilities(ANS* ans, int operation_id, double old_of, double new_of, double time);

/** Inicializa una rita de pla planificacion base eliminando pedidos */
Route* initialize(ANS* ans, Route* original_route);

/** Ejecuta la heuristica */
Route* run(ANS* ans, Route* original_route);
