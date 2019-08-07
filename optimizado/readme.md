# Modo de uso

## Compilar

Para compilar hay que estar en la carpeta "optimizado" con la consola y escribir

```sh
make
```

Esto generará un archivo compilado llamado "optimizador" y una carpeta obj con archivos compilados

Para limpiar todos los archivos binarios y compilados hay que escribir en consola

```sh
make clean
```

Esto borrará la carpeta obj y el archivo "optimizador"

## Ejecutar programa

Ejecutar en consola:

```sh
./optimizador [orders.txt] [airplanes.txt] [PB.txt] [costs.txt] (-ow)
```

Donde:

* [orders.txt] es el archivo con los pedidos
* [airplanes.txt] es el archivo con las aviones
* [PB.txt] es el archivo con la planificación base
* [costs.txt] es el archivo con los costos de los vuelos
* (-ow) es opcional y determina si se optimizan los pesos en una ruta o si se toman solo los pesos completos

Ejemplo ejecutando la instacia ubicada en la carpeta Ejemplo5:
```sh
./optimizador files/Ejemplo5/orders.txt files/Ejemplo5/airplanes.txt files/Ejemplo5/PB.txt files/Ejemplo5/costs.txt -ow
```

Además se recomienda agregar a este comando lo siguiente para que no se imprima un mensaje de gurobi a cada llamada de una de sus funciones:
```sh
| grep -v "Academic license"
```

# Formato de los archivos

## Pedidos:

La primera linea indica cuantos nodos hay (el doble de los pedidos) y el número de macronodos totales.
Las siguientes lineas corresponden a cada nodo de tipo pickup o delivery, y se indican los siguientes elementos:

```
[id_node] [id_macronode] [tipo] [peso] [duracion] [hora_inicio] [hora_fin] [tarifa]
```

* El tipo puede ser "p" o "d", lo que indica que es pickup o delivery.
* La duración es el tiempo que se demora en hacer el pickup o el delivery.
* El peso es el total de la carga, y es negativo en caso de ser un nodo de delivery.
* hora_inicio y hora_fin corresponden al rango de tiempo en los cuales esta disponible el nodo para el pickup o delivery.
* La tarifa es la ganancia por hacer el pickup (es o en caso de ser delivery)

**OJO:**
Si hay "n" pedidos, el pickup de ID "i" corresponde al delivery de ID = i + n

**OJO**
El número de macronodos incluye a los macronodos de los nodos de inicio y de fin. Estos deben estar cada uno en un macronodo distinto.

## airplanes.txt

Primera linea
```
[n_aviones] [costo_penalizacion]
```

n_aviones * 2 lineas siguientes
```
[id_avion] [tipo (s o e)] [id_node] [id_macronode] [hora_inicio] [hora_fin]
```

Las últimas n_aviones lineas dan la capacidad de cada avion (peso).

Los IDs deben partir en 2 * pedidos y terminar en 2 * pedidos + 2 * aviones - 1

## costs.txt


La primera línea parte indicando la cantidad de líneas de costos del archivo

Las líneas siguientes tienen

```
[macro_id1] [macro_id2] [costo] [tiempo]
```

* macro_id1 y macro_id2 son los ids de los macronodos de donde parte el viaje y donde termina respectivamente
* costo es el costo del viaje
* tiempo es el tiempo que demora el avion en llega de macro_id1 a macro_id2

## PB.txt

Primera linea
```
[numero_aviones]
```

siguientes numero_aviones líneas parten con

```
[n_nodos_ruta]
```

y los siguientes números de cada línea son los nodos de la ruta

# Output del programa

Al ejecutar el programa se generarán dos archivos con resultados: results.txt y details.txt

results.txt contiene los resultados de cada corrida dentro de la iteración: Utilidad, rutas en la solución, probabilidades de las operaciones y cantidad de veces que se ejecutó cada iteración.

details.txt contiene los detalles de la solución final: Utilidad, tiempo total, rutas finales con asignaciones de tiempos y utilidad de cada una.

El orden de las operaciones es:

```
Drop and Add
Swap
IRRR
IRRE
IRMRR
IRMRE
Delete
```

## Cálculo de probabilidades

Para calcular las probabilidades de usó la siguiente técnica:

Cada operación parte con 1 punto. Si al utilizar la operación esta mejora, se multiplica sus puntos por 2. Si no mejora, se resta 1 a sus puntos. Cada operación puede tener mínimo 1 punto y máximo 50.

La probabilidad de ejecutar una operación es su cantidad de puntos dividido en los puntos totales
