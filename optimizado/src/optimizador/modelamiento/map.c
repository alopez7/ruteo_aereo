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

static void map_nodes_init(Map* map, char* map_path, char* airplanes_path)
{
  // Abro el archivo de nodos
  FILE* map_file = fopen(map_path, "r");

  // Abro el archivo de aviones
  FILE* airplanes_file = fopen(airplanes_path, "r");

  // Leo la cantida de nodos y creo el arreglo de nodos, de pedidos y limites
  // Aca inicializo las variables globales airplanes_count y penalization_cost
  fscanf(airplanes_file, "%d %lf", &airplanes_count, &penalization_cost);
  int airplane_nodes_count = airplanes_count * 2;

  int map_nodes_count;
  fscanf(map_file, "%d %d", &map_nodes_count, &map -> macronode_count);
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
    fscanf(map_file, "%d %d %c %lf %lf %lf %lf %lf", &n -> id, &father_id,
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
  fclose(map_file);
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

/** Lee la planificacion base y almacena los vuelos que estaba originalmente */
static void map_add_original_flights(Map* map, char* PB_file)
{
  // Creo la matriz
  map -> original_flights = malloc(sizeof(int*) * map -> macronode_count);
  for (int i = 0; i < map -> macronode_count; i++)
  {
    map -> original_flights[i] = malloc(sizeof(int) * map -> macronode_count);
    for (int j = 0; j < map -> macronode_count; j++)
    {
      // Inicializo en -1 por lo que no esta en ningun vuelo
      map -> original_flights[i][j] = -1;
    }
  }

  // Leo el archivo
  FILE* PB_f = fopen(PB_file, "r");

  // Leo la planificacion base
  for (int a = 0; a < airplanes_count; a++)
  {
    // Leo el numero de nodos de la ruta
    int size;
    fscanf(PB_f, "%d", &size);

    // Leo el primer nodo
    int n_ant;
    fscanf(PB_f, "%d", &n_ant);

    // Itero leyendo los nodos
    int n_act;
    for (int n = 0; n < size; n++)
    {
      // Leo el indice del nodo
      fscanf(PB_f, "%d", &n_act);

      // Obtengo los nodos de los indices que lei
      Node* N_ant = map -> nodes[n_ant];
      Node* N_act = map -> nodes[n_act];

      // Agrego el vuelo a la matriz
      map -> original_flights[N_ant -> father -> id][N_act -> father -> id] = a;

      // Cambio el nodo anterior
      n_ant = n_act;
    }
  }

  // Cierro el archivo
  fclose(PB_f);
}

///////////////////////////////////////////////////////////////////////////////
//                        metodos publicos                                   //
///////////////////////////////////////////////////////////////////////////////


/** Metodo que parsea el input y crea el mapa completo */
Map* map_init(char* map_path, char* airplanes_path, char* macronodes_file, char* costs_file, char* PB_file)
{
  // Creo el mapa
  Map* map = malloc(sizeof(Map));

  // Creo los nodos y los macronodos
  map_nodes_init(map, map_path, airplanes_path);

  // Leo el archivo de costos
  map_costs_init(map, costs_file);

  // Leo la planificacion base
  map_add_original_flights(map, PB_file);

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

  // Libero la matriz de vuelos originales
  for (int i = 0; i < map -> macronode_count; i++)
  {
    free(map -> original_flights[i]);
  }
  free(map -> original_flights);

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

/** distancia euclideana de un macro nodo a otro */
double distance(Macronode* macro1, Macronode* macro2)
{
  return sqrt(pow(macro2 -> X - macro1 -> X, 2) + pow(macro2 -> Y - macro1 -> Y, 2));
}

double print_distance(Macronode* macro1, Macronode* macro2)
{
  double d = sqrt(pow(macro2 -> X - macro1 -> X, 2) + pow(macro2 -> Y - macro1 -> Y, 2));
  printf("Distancia de (%lf, %lf) a (%lf, %lf) = %lf\n", macro1 -> X, macro1 -> Y, macro2 -> X, macro2 -> Y, d);
  return d;
}
