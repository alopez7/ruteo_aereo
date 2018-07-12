#include "../ANS/ANS.h"
#include "../optimizacion/optimizacion.h"
#include <stdlib.h>
#include <stdio.h>
#include "../random/pcg_basic.h"
#include <time.h>
#include <math.h>
#include <string.h>

/** Retorna un randint entre 0 y bound */
int random_int(int bound);

// Desordena la rutas (asume que es un arreglo con airplanes_count rutas)
void shuffle(Route** routes);

// Crea rutas vacias para cada avion
Route** create_empty_routes(ANS* ans);

// Crea las rutas iniciales para cada avion
Route** create_initial_routes(ANS* ans);

// Crea muchas columnas iniciales
void generate_initial_routes(ANS* ans, int airplane_id, Route* initial_route);

// Elige una ruta aleatoria del arreglo y la retorna
Route* choose_random(Route** routes, int count);

// Para cada ruta recalcula su funcion objetivo (ya que cambia con los costos duales)
void recalculate_of(ANS* ans);

// Loop principal del programa
double* solve(ANS* ans);
