#include "dictionary.h"
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////
//                       Funciones privadas                            //
/////////////////////////////////////////////////////////////////////////

// Funcion de hash simple y rapida (no conmutativa)
static int hash(int id1, int id2)
{
  return (id1 * id2) ^ id1;
}

// Crea un arreglo mas grande y rehashea todos los elementos
static void rehash(Dictionary* dict)
{
  // Creo un arreglo del doble de grande
  Cell** new_cells = calloc(dict -> array_lenght * 2, sizeof(Cell*));

  // Itero sobre las celdas del arreglo viejo y las agrego al nuevo
  for (int i = 0; i < dict -> array_lenght; i++)
  {
    // Si es una celda
    if (dict -> cells[i])
    {
      // La agrego al arreglo nuevo
      int id1 = dict -> cells[i] -> id_start;
      int id2 = dict -> cells[i] -> id_end;
      int position = hash(id1, id2) % (dict -> array_lenght * 2);
      while (new_cells[position])
      {
        position = (position + 1) % (dict -> array_lenght * 2);
      }

      // Inserto la celda
      new_cells[position] = dict -> cells[i];
    }
  }

  // Elimino el arreglo anterior
  free(dict -> cells);

  // Agrego el nuevo al diccionario y actualizo su tamaño
  dict -> cells = new_cells;
  dict -> array_lenght = dict -> array_lenght * 2;
}

/** Copia una celda y la retorna */
static Cell* cell_copy(Cell* cell)
{
  // Creo la copia
  Cell* copy = malloc(sizeof(Cell));

  // Traspaso todos los valores
  copy -> fly_count = cell -> fly_count;
  copy -> id_end = cell -> id_end;
  copy -> id_start = cell -> id_start;

  // Retorno la copia
  return copy;
}

/////////////////////////////////////////////////////////////////////////
//                       Funciones públicas                            //
/////////////////////////////////////////////////////////////////////////

/** Inicializa un diccionario vacio de tamaño 8 */
Dictionary* dictionary_init()
{
  // Inicializo el diccionario
  Dictionary* dict = malloc(sizeof(Dictionary));

  // Inicializo sus variables
  dict -> array_lenght = 8;
  dict -> cell_count = 0;
  dict -> cells = calloc(dict -> array_lenght, sizeof(Cell*));

  // Retorno el diccionario
  return dict;
}

/** Agrega al diccionario un vuelo (id_macronodo1, id_macronodo2) */
void dictionary_add(Dictionary* dict, int id1, int id2)
{
  // Creo la celda
  Cell* cell = malloc(sizeof(Cell));
  cell -> fly_count = 0;
  cell -> id_start = id1;
  cell -> id_end = id2;

  // Busco sus posicion
  int position = hash(id1, id2) % dict -> array_lenght;
  while (dict -> cells[position])
  {
    position = (position + 1) % dict -> array_lenght;
  }

  // Inserto la celda
  dict -> cells[position] = cell;

  // Aumento la cuenta de celdas
  dict -> cell_count++;

  // Veo si tengo que rehashear
  if (dict -> cell_count >= dict -> array_lenght * 0.7)
  {
    rehash(dict);
  }
}

/** Reinicia el diccionario con ceros en los vuelos */
void dictionary_reset(Dictionary* dict)
{
  // Itero sobre las celdas del diccionario
  for (int i = 0; i < dict -> array_lenght; i++)
  {
    // Si encuentro una celda
    if (dict -> cells[i])
    {
      // Dejo su cuenta en 0
      dict -> cells[i] -> fly_count = 0;
    }
  }
}

/** Revisa si existe el vuelo y suma 1 si es el caso y retorna true si hay que eliminar el costo de penalizacion */
bool dictionary_create_flight(Dictionary* dict, int id1, int id2)
{
  // Obtengo el hash de la celda
  int position = hash(id1, id2) % dict -> array_lenght;

  // Busco el vuelo en la celda
  while (dict -> cells[position])
  {
    // Si es el vuelo que busco
    Cell* cell = dict -> cells[position];
    if (cell -> id_start == id1 && cell -> id_end == id2)
    {
      // Le sumo 1 y termino
      cell -> fly_count++;
      // retorno true, si ahora hay 1 solo vuelo (antes no estaba)
      if (cell -> fly_count == 1) return true;
      return false;
    }

    position = (position + 1) % dict -> array_lenght;
  }
  return false;
}

/** Revisa si existe el vuelo y resta 1 si es el caso y retorna true si hay que agregar el costos de penalizacion */
bool dictionary_cancel_flight(Dictionary* dict, int id1, int id2)
{
  // Obtengo el hash de la celda
  int position = hash(id1, id2) % dict -> array_lenght;

  // Busco el vuelo en la celda
  while (dict -> cells[position])
  {
    // Si es el vuelo que busco
    Cell* cell = dict -> cells[position];
    if (cell -> id_start == id1 && cell -> id_end == id2)
    {
      // Le resto 1 y termino
      cell -> fly_count--;
      // Retorno true si ahora no hay vuelos (se cancelo)
      if (cell -> fly_count == 0) return true;
      return false;
    }

    position = (position + 1) % dict -> array_lenght;
  }
  return false;
}

/** Destruye el diccionario y todos sus recursos */
void dictionary_destroy(Dictionary* dict)
{
  // Itero sobre las celdas
  for (int i = 0; i < dict -> array_lenght; i++)
  {
    // Si encuentro una celda
    if (dict -> cells[i])
    {
      // La elimino
      free(dict -> cells[i]);
    }
  }
  free(dict -> cells);

  // Elimino el diccionario
  free(dict);
}

/** Imprime los valores del diccionario */
void dictionary_print(Dictionary* dict)
{
  printf("Diccionario con %d elementos:\n", dict -> cell_count);

  // Itero sobre el diccionario
  for (int i = 0; i < dict -> array_lenght; i++)
  {
    // Si es una celda, la imprimo
    if (dict -> cells[i])
    {
      Cell* cell = dict -> cells[i];
      printf("ID1: %d, ID2: %d, Vuelos: %d\n", cell -> id_start, cell -> id_end, cell -> fly_count);
    }
  }
  printf("\n");
}

/** Copia un diccionario */
Dictionary* dictionary_copy(Dictionary* dict)
{
  // Creo el diccionario
  Dictionary* copy = malloc(sizeof(Dictionary));

  // Copio los valores de las variables
  copy -> array_lenght = dict -> array_lenght;
  copy -> cell_count = dict -> cell_count;

  // Creo el arreglo de celdas
  copy -> cells = calloc(dict -> array_lenght, sizeof(Cell*));

  // Itero sobre el diccionario copiando las celdas
  for (int i = 0; i < dict -> array_lenght; i++)
  {
    // Si hay una celda
    if (dict -> cells[i])
    {
      // La copio
      copy -> cells[i] = cell_copy(dict -> cells[i]);
    }
  }

  // Retorno la copia
  return copy;
}
