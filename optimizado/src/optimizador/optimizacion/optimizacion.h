#pragma once

#include "../modelamiento/route.h"
#include "gurobi_c.h"

double final_utility;
int ones;

bool optimizing_weights;

/** Optimiza las cargas de un avion */
void optimize_weight(Route* route);

/** Optimiza las cargas de un avion considerando la utilidad */
void utility_optimize_weight(Route* route);

/** Codigo que optimiza las rutas de manera binaria */
double* optimize_routes(Route*** routes, int* count, Map* map);

/** Codigo que optimiza las rutas de manera relajada y da los duales */
double optimize_routes_relaxed(Route*** routes, int* count, Map* map);
