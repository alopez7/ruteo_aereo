# orders.txt

Primera linea
```
[n_nodes (pickup or delivery)] [n_macronodes]
```

n_nodes lineas siguientes
```
[id_node] [id_macronode] [tipo] [peso] [duracion] [hora_inicio] [hora_fin] [tarifa]
```

## airplanes.txt

Primera linea
```
[n_aviones] [costo_penalizacion]
```

n_aviones * 2 lineas siguientes
```
[id_avion] [tipo (s o e)] [id_node] [id_macronode] [hora_inicio] [hora_fin]
```

n_aviones lineas finales
```
[capacidad]
```

## costs.txt

```
[n_costos]
```

n_costos líneas siguientes

```
[macro_id1] [macro_id2] [costo] [tiempo]
```

## PB.txt

numero_aviones líneas parten con

```
[n_nodos_ruta]
```

y los siguientes números de cada línea son los nodos de la ruta
