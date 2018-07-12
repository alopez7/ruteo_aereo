#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
//                        metodos privados                                   //
///////////////////////////////////////////////////////////////////////////////

/** crea un macro nodo a partir de un nodo */
static Macronode* macronode_init(Node* node, int id)
{
  // Creo el macro nodo
  Macronode* macro = malloc(sizeof(Macronode));

  // Inicializo sus variables
  macro -> node_count = 1;
  macro -> X = node -> X;
  macro -> Y = node -> Y;
  macro -> id = id;

  // Creo la referencia al macro desde el nodo
  node -> father = macro;

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
  fscanf(map_file, "%d", &map_nodes_count);
  // Aca inicializo la variable global order_count
  order_count = map_nodes_count / 2;
  map -> node_count = airplane_nodes_count + map_nodes_count;
  map -> nodes = malloc(sizeof(Node*) * map -> node_count);

  map -> orders = malloc(sizeof(Order) * order_count);

  map -> limit_count = airplanes_count;
  map -> limits = malloc(sizeof(Order) * map -> limit_count);

  // Voy creando los nodos del mapa uno por uno
  for (int i = 0; i < map_nodes_count; i++)
  {
    map -> nodes[i] = malloc(sizeof(Node));

    // Leo e inicializo los atributos del nodo
    char type;
    fscanf(map_file, "%d %c %lf %lf %lf %lf %lf %lf %lf", &map -> nodes[i] -> id,
    &type, &map -> nodes[i] -> X, &map -> nodes[i] -> Y,
    &map -> nodes[i] -> delay_time, &map -> nodes[i] -> total_weight,
    &map -> nodes[i] -> start_time, &map -> nodes[i] -> end_time,
    &map -> nodes[i] -> fee);

    // Inicializo otros atributos del nodo
    map -> nodes[i] -> dual_pi = 0;
    map -> nodes[i] -> node_type = type == 'p' ? PICKUP : DELIVERY;
    map -> nodes[i] -> father = NULL;

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

    // Leo e inicializo los atributos del nodo
    char type;
    int airplane_id;
    fscanf(airplanes_file, "%d %c %d %lf %lf %lf %lf\n", &airplane_id,
    &type, &map -> nodes[j] -> id, &map -> nodes[j] -> X, &map -> nodes[j] -> Y,
    &map -> nodes[j] -> start_time, &map -> nodes[j] -> end_time);

    // Inicializo otros atributos del nodo
    map -> nodes[j] -> total_weight = 0;
    map -> nodes[j] -> fee = 0;
    map -> nodes[j] -> delay_time = 0;
    map -> nodes[j] -> node_type = type == 's' ? START : END;
    map -> nodes[j] -> father = NULL;
    map -> nodes[j] -> dual_pi = 0;

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
}

/** crea el arrelgo de macro nodos */
static void map_macronodes_init(Map* map)
{
  // Creo los macro nodos
  // Creo el arrelgo de macro nodos de tamaño |nodes| pero luego lo comprimo
  map -> macronodes = malloc(sizeof(Macronode*) * map -> node_count);
  map -> macronode_count = 0;

  // (no es particularmente eficiente pero este tiempo es
  // negligible con respecto al programa)

  // Para todos los nodos
  for (int i = 0; i < map -> node_count; i++)
  {
    Node* actualnode = map -> nodes[i];
    // Veo los nodos anteriores
    for (int j = 0; j < i; j++)
    {
      // Si hay un macro nodo con las mismas cordenadas
      Macronode* actualmacro = map -> nodes[j] -> father;
      if (actualmacro -> X == actualnode -> X && actualmacro -> Y == actualnode -> Y)
      {
        // Agrego el macro nodo
        actualnode -> father = actualmacro;
        actualmacro -> node_count++;
        break;
      }
    }
    // Si no se ha agregado ningun macro nodo, creo uno nuevo
    if (!actualnode -> father)
    {
      map -> macronodes[map -> macronode_count] = macronode_init(actualnode, map -> macronode_count);
      map -> macronode_count++;
    }
  }

  // Comprimo el arreglo de macro nodos
  Macronode** macros = malloc(sizeof(Macronode*) * map -> macronode_count);
  for (int i = 0; i < map -> macronode_count; i++)
  {
    macros[map -> macronodes[i] -> id] = map -> macronodes[i];
  }
  free(map -> macronodes);
  map -> macronodes = macros;

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

// crea la matriz que dice si un vuelo pertenecia a la planificacion base
static void map_pb_matrix_init(Map* map)
{
  // La creo vacia por ahora
  map -> original_flights = malloc(sizeof(uint16_t*) * map -> macronode_count);

  // Creo sus subarreglos
  for (int i = 0; i < map -> macronode_count; i++)
  {
    map -> original_flights[i] = calloc(map -> macronode_count, sizeof(uint16_t));
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


///////////////////////////////////////////////////////////////////////////////
//                        metodos publicos                                   //
///////////////////////////////////////////////////////////////////////////////


/** Metodo que parsea el input y crea el mapa completo */
Map* map_init(char* map_path, char* airplanes_path)
{
  // Creo el mapa
  Map* map = malloc(sizeof(Map));

  // Creo los nodos
  map_nodes_init(map, map_path, airplanes_path);

  // creo los macro nodos
  map_macronodes_init(map);

  // creo la matriz de PB
  map_pb_matrix_init(map);

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
