#pragma once

#include "map.h"

// Este diccionario registra qué vuelos estan inicialmente en la planificacion base.
// Se usa para calcular los costos de cancelacion de manera eficiente.
// Para esto se itera sobre la ruta y se revisa si el vuelo correspondiente esta en el diccionario.
// En este caso se suma 1 al contador que registra ese vuelo.
// Si al final un contador está en 0, entonces ese vuelo se canceló.
// Al agregar y sacar vuelos en las operaciones se puede saber rapidamente si se cancela o no un vuelo.

/** celda del diccionario que contiene los ids y la cuenta de vuelos */
struct cell
{
  /** id del macronodo donde parte el vuelo */
  int id_start;
  /** id del macronodo donde termina el vuelo */
  int id_end;
  /** numero que indica la cantidad de veces que se hace este vuelo */
  int fly_count;
};

/** celda del diccionario que contiene los ids y la cuenta de vuelos */
typedef struct cell Cell;

/** Diccionario que registra los vuelos de la planificacion base de un avion espacifico*/
struct dictionary
{
  /** Arreglo de celdas */
  Cell** cells;
  /** Tamaño del arreglo de celdas */
  int array_lenght;
  /** Cantidad de celdas registradas */
  int cell_count;
};

/** Diccionario que registra los vuelos de la planificacion base de un avion espacifico*/
typedef struct dictionary Dictionary;

/////////////////////////////////////////////////////////////////////////
//                             Funciones                               //
/////////////////////////////////////////////////////////////////////////

/** Inicializa un diccionario vacio de tamaño 8 */
Dictionary* dictionary_init();

/** Agrega al diccionario un vuelo (id_macronodo1, id_macronodo2) */
void dictionary_add(Dictionary* dict, int id1, int id2);

/** Reinicia el diccionario con ceros en los vuelos */
void dictionary_reset(Dictionary* dict);

/** Revisa si existe el vuelo y suma 1 si es el caso y retorna true si hay que eliminar el costo de penalizacion */
bool dictionary_create_flight(Dictionary* dict, int id1, int id2);

/** Revisa si existe el vuelo y resta 1 si es el caso y retorna true si hay que agregar el costos de penalizacion */
bool dictionary_cancel_flight(Dictionary* dict, int id1, int id2);

/** Destruye el diccionario y todos sus recursos */
void dictionary_destroy(Dictionary* dict);

/** Imprime los valores del diccionario */
void dictionary_print(Dictionary* dict);

/** Copia un diccionario */
Dictionary* dictionary_copy(Dictionary* dict);
