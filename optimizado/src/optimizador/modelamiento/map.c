#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
//                        metodos privados                                   //
///////////////////////////////////////////////////////////////////////////////

/** crea un macro nodo a partir de un nodo */
static Macronode* macronode_init(int id)
{
  // Creo el macro nodo
  Macronode* macro = malloc(sizeof(Macronode));

  // Inicializo sus variables
  macro -> node_count = 0;
  macro -> id = id;

  // Retorno el macro nodo
  return macro;
}

static void map_nodes_init(Map* map, char* orders_path, char* airplanes_path)
{
  // Abro el archivo de nodos
  FILE* orders_file = fopen(orders_path, "r");

  // Abro el archivo de aviones
  FILE* airplanes_file = fopen(airplanes_path, "r");

  // Leo la cantida de nodos y creo el arreglo de nodos, de pedidos y limites
  // Aca inicializo las variables globales airplanes_count y penalization_cost
  fscanf(airplanes_file, "%d %lf", &airplanes_count, &penalization_cost);
  int airplane_nodes_count = airplanes_count * 2;

  int map_nodes_count;
  fscanf(orders_file, "%d %d", &map_nodes_count, &map -> macronode_count);
  // Aca inicializo la variable global order_count
  order_count = map_nodes_count / 2;
  map -> node_count = airplane_nodes_count + map_nodes_count;
  map -> nodes = malloc(sizeof(Node*) * map -> node_count);

  map -> orders = malloc(sizeof(Order) * order_count);

  map -> limit_count = airplanes_count;
  map -> limits = malloc(sizeof(Order) * map -> limit_count);

  // Creo el arreglo de macronodos y creo los macronodos incialmente vacios
  map -> macronodes = malloc(sizeof(Macronode*) * map -> macronode_count);
  for (int i = 0; i < map -> macronode_count; i++)
  {
    map -> macronodes[i] = macronode_init(i);
  }

  // Voy creando los nodos del mapa uno por uno
  for (int i = 0; i < map_nodes_count; i++)
  {
    map -> nodes[i] = malloc(sizeof(Node));
    Node* n = map -> nodes[i];

    // Leo e inicializo los atributos del nodo
    char type;
    int father_id;
    fscanf(orders_file, "%d %d %c %lf %lf %lf %lf %lf", &n -> id, &father_id,
    &type, &n -> total_weight, &n -> delay_time, &n -> start_time,
    &n -> end_time, &n -> fee);

    // Inicializo otros atributos del nodo
    n -> dual_pi = 0;
    n -> node_type = type == 'p' ? PICKUP : DELIVERY;
    n -> father = map -> macronodes[father_id];
    map -> macronodes[father_id] -> node_count++;

    // Creo los pedidos
    if (i < order_count)
    {
      map -> orders[i] = malloc(sizeof(Order));
      map -> orders[i] -> pickup = map -> nodes[i];
    }
    else if (i < order_count * 2)
    {
      map -> orders[i - order_count] -> delivery = map -> nodes[i];
    }
  }

  // Hago lo mismo para los nodos de las aviones
  for (int i = 0; i < airplane_nodes_count; i++)
  {
    int j = i + map_nodes_count;
    map -> nodes[j] = malloc(sizeof(Node));
    Node* n = map -> nodes[j];

    // Leo e inicializo los atributos del nodo
    char type;
    int father_id;
    int airplane_id;
    fscanf(airplanes_file, "%d %c %d %d %lf %lf\n", &airplane_id,
    &type, &n -> id, &father_id,
    &n -> start_time, &n -> end_time);

    // Inicializo otros atributos del nodo
    n -> total_weight = 0;
    n -> fee = 0;
    n -> delay_time = 0;
    n -> node_type = type == 's' ? START : END;
    n -> father = map -> macronodes[father_id];
    n -> father -> node_count++;
    n -> dual_pi = 0;

    // Creo los limites
    if (type == 's')
    {
      map -> limits[airplane_id] = malloc(sizeof(Limit));
      map -> limits[airplane_id] -> start = map -> nodes[j];
    }
    else
    {
      map -> limits[airplane_id] -> end = map -> nodes[j];
    }
  }

  // Creo las aviones
  map -> airplanes = malloc(sizeof(Airplane*) * airplanes_count);
  for (int i = 0; i < airplanes_count; i++)
  {
    // Creo la memoria necesaria
    map -> airplanes[i] = malloc(sizeof(Airplane));

    // Relleno sus variables
    fscanf(airplanes_file, "%lf", &map -> airplanes[i] -> total_capacity);
    map -> airplanes[i] -> id = i;
    map -> airplanes[i] -> dual_gamma = 0;
  }

  // cierro los archivos
  fclose(orders_file);
  fclose(airplanes_file);

  // Agrego las referencias de los macro nodos a los nodos

  // Creo los arreglos de nodos en los macro nodos
  for (int i = 0; i < map -> macronode_count; i++)
  {
    Macronode* macro = map -> macronodes[i];
    macro -> nodes = malloc(sizeof(Node*) * macro -> node_count);
    macro -> node_count = 0;
  }

  // Voy agregando las referencias en los macro nodos
  for (int i = 0; i < map -> node_count; i++)
  {
    Node* node = map -> nodes[i];
    node -> father -> nodes[node -> father -> node_count] = node;
    node -> father -> node_count++;
  }
}

/** libera los recursos asociados a un nodo */
static void node_destroy(Node* node)
{
  free(node);
}

/** libera los recursos asociados a un macro nodo (libera sus nodos) */
static void macronode_destroy(Macronode* macronode)
{
  for (int i = 0; i < macronode -> node_count; i++)
  {
    node_destroy(macronode -> nodes[i]);
  }
  free(macronode -> nodes);
  free(macronode);
}

/** Crea las matrices de costos y distancias a partir del archivo */
static void map_costs_init(Map* map, char* costs_file)
{
  // Inicializo las matrices de Mnodos X Mnodos X airplanes
  map -> costs = malloc(sizeof(double*) * map -> macronode_count);
  map -> distances = malloc(sizeof(double*) * map -> macronode_count);

  // Les pongo valor inicial infinito
  for (int i = 0; i < map -> macronode_count; i++)
  {
    map -> costs[i] = malloc(sizeof(double) * map -> macronode_count);
    map -> distances[i] = malloc(sizeof(double) * map -> macronode_count);
    for (int j = 0; j < map -> macronode_count; j++)
    {
      map -> costs[i][j] = INFINITY;
      map -> distances[i][j] = INFINITY;
    }
  }

  // Leo el archivo y completo las matrices
  FILE* costs_f = fopen(costs_file, "r");
  int costs_count;
  fscanf(costs_f, "%d", &costs_count);

  for (int i = 0; i < costs_count; i++)
  {
    int m1;
    int m2;
    double cost;
    double dist;
    fscanf(costs_f, "%d %d %lf %lf", &m1, &m2, &cost, &dist);

    map -> costs[m1][m2] = cost;
    map -> distances[m1][m2] = dist;
  }

  // Cierro el archivo
  fclose(costs_f);
}

///////////////////////////////////////////////////////////////////////////////
//                        metodos publicos                                   //
///////////////////////////////////////////////////////////////////////////////


/** Metodo que parsea el input y crea el mapa completo */
Map* map_init(char* orders_path, char* airplanes_path, char* costs_file)
{
  // Creo el mapa
  Map* map = malloc(sizeof(Map));

  // Creo los nodos y los macronodos
  map_nodes_init(map, orders_path, airplanes_path);

  // Leo el archivo de costos
  map_costs_init(map, costs_file);

  return map;
}

/** Metodo que libera toda la memoria asociada el mapa */
void map_destroy(Map* map)
{
  // libero el arreglo de nodos
  free(map -> nodes);

  // Libero los macronodos y los nodos
  for (int i = 0; i < map -> macronode_count; i++)
  {
    macronode_destroy(map -> macronodes[i]);
  }
  free(map -> macronodes);

  // Libero las ordenes
  for (int i = 0; i < order_count; i++)
  {
    free(map -> orders[i]);
  }
  free(map -> orders);

  // Libero los limites
  for (int i = 0; i < map -> limit_count; i++)
  {
    free(map -> limits[i]);
  }
  free(map -> limits);

  // Libero los aviones
  for (int i = 0; i < airplanes_count; i++)
  {
    free(map -> airplanes[i]);
  }
  free(map -> airplanes);

  // Libero matrices de costos y distancias
  for (int i = 0; i < map -> macronode_count; i++)
  {
    free(map -> costs[i]);
    free(map -> distances[i]);
  }
  free(map -> costs);
  free(map -> distances);

  // libero el mapa
  free(map);
}

/** Distancia entre macronodos */
double distance(Macronode* macro1, Macronode* macro2, Map* map)
{
  return map -> distances[macro1 -> id][macro2 -> id];
}

/** Distancia entre macronodos */
double print_distance(Macronode* macro1, Macronode* macro2, Map* map)
{
  double d = map -> distances[macro1 -> id][macro2 -> id];
  printf("Distancia de M(%d a M(%d = %lf\n", macro1 -> id, macro2 -> id, d);
  return d;
}

/** Costo entre macronodos */
double cost(Macronode* macro1, Macronode* macro2, Map* map)
{
  return map -> costs[macro1 -> id][macro2 -> id];
}

/** Costo entre macronodos */
double print_cost(Macronode* macro1, Macronode* macro2, Map* map)
{
  double d = map -> costs[macro1 -> id][macro2 -> id];
  printf("Costo de M(%d a M(%d = %lf\n", macro1 -> id, macro2 -> id, d);
  return d;
}
