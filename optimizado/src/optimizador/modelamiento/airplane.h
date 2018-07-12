#pragma once

#include <stdlib.h>


/** Struct que maneja los datos de un avion */
struct airplane
{
  /** Capacidad total */
  double total_capacity;
  /** ID del avion */
  int id;
  /** Dual gamma */
  double dual_gamma;
};
/** Struct que maneja los datos de un avion */
typedef struct airplane Airplane;
