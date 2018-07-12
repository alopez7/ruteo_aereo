#include "maestro/maestro.h"

// Le da una semilla random al generador de números aleatorios
void random_seed()
{
  double t1 = clock();
  double t2 = clock();
  pcg32_srandom((uint64_t)t1, (uint64_t)t2);
}

// Funcion inicial del programa que contiene el algoritmo iterativo
int main(int argc, char *argv[])
{
  // Obtengo el input del programa
  if (argc == 4)
  {
    optimizing_weights = false;
  }
  else if (argc == 5)
  {
    optimizing_weights = true;
  }
  else
  {
    printf("Modo de uso:\n");
    printf("[./optimizador] [orders_file] [airplanes_file] [PB_file] (-ow)\n");
    return 0;
  }
  // Pongo una seed aleatoria al random del programa
  random_seed();

  int max_iterations = 10;
  int improve_runs = 3;
  double best_result = 0;

  int no_improve = 0;
  int iteration = 0;
  double total_time = 0;
  clock_t initial_time = clock();
  while (no_improve < improve_runs && iteration < max_iterations && total_time < 3600)
  {
    // Inicializo el ans
    ANS* ans = ans_init(argv[1], argv[2], argv[3]);

    // Ejecuto el solver
    double* solution = solve(ans);

    // Si mejore con respecto a la mejor corrida escribo los detalles
    if (final_utility > best_result)
    {
      // Creo un archivo con los detalles
      FILE* detail_output = fopen("details.txt", "w");

      // Escribo los detalles de la corrida
      int pos = 0;
      for (int i = 0; i < airplanes_count; i++)
      {
        fprintf(detail_output, "Airplane %d\n", i);
        for (int j = 0; j < ans -> route_count[i]; j++)
        {
          if (solution[pos] > 0)
          {
            route_print(ans -> routes[i][j], detail_output);
            fprintf(detail_output, "Utility: %lf\n", utility(ans -> routes[i][j]));
          }
          pos += 1;
        }
        fprintf(detail_output, "\n");
      }
      fprintf(detail_output, "Total utility: %lf\n", final_utility);

      // Cierro el archivo
      fclose(detail_output);
    }

    // Cuento las rutas hasta ahora
    int pos = 0;
    for (int i = 0; i < airplanes_count; i++)
    {
      pos += ans -> route_count[i];
    }

    // Escribo el archivo con la informacion de esta corrida
    FILE* scores = fopen("results.txt", "a");
    fprintf(scores, "Utilidad = %lf\n", final_utility);
    fprintf(scores, "RutasSolucion = %d\n", ones);
    fprintf(scores, "RutasGeneradas = %d\n", pos);
    fprintf(scores, "Probabilidades = [");
    for (int p = 0; p < 7; p++)
    {
      if (p != 6) fprintf(scores, "%lf, ", ans -> prob_weights[p] / ans -> total_weight);
      else fprintf(scores, " %lf]\n", ans -> prob_weights[p] / ans -> total_weight);
    }
    fprintf(scores, "Veces ejecutado = [");
    for (int p = 0; p < 7; p++)
    {
      if (p != 6) fprintf(scores, "%d, ", ans -> operation_count[p]);
      else fprintf(scores, " %d]\n\n", ans -> operation_count[p]);
    }
    fclose(scores);

    // Libero la solucion
    free(solution);

    // Libero el ans
    ans_destroy(ans);

    // Veo si mejore en esta iteracion
    if (final_utility > best_result)
    {
      no_improve = 0;
      best_result = final_utility;
    }
    else no_improve++;

    // Calculo el tiempo total que llevo
    clock_t actual_time = clock();
    total_time = (actual_time - initial_time) / (double)CLOCKS_PER_SEC;

    printf("Terminada iteracion %d\n", iteration);
    printf("Veces sin mejorar: %d\n", no_improve);
    printf("Tiempo: %lf\n", total_time);

    // Sumo 1 a la cantidad de iteraciones que llevo
    iteration++;

  }

  // Escribo tiempo final
  FILE* detail_output = fopen("details.txt", "a");
  fprintf(detail_output, "Tiempo total: %lfs\n", total_time);
  fprintf(detail_output, "Corridas internas: %d\n", iteration);
  fclose(detail_output);

  return 0;
}
