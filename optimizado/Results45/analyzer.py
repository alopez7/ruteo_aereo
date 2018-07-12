from os import listdir
import matplotlib.pyplot as plt

folders = ["SinOW", "ConOW"]
# folders = ["SinOW0.3", "SinOW0.5", "SinOW0.7", "ResultadosConOW", "SinOWLimitadas","ConOWLimitadas", "PesosArreglado"]

def clean(line):
    remove = ["Probabilidades", "[", "]", " ", "="]
    for i in remove:
        line = line.replace(i, "")
    return line


folders_data = {}
for folder in folders:
    data = []
    with open(folder + "/results.txt") as results_file:
        i = 0

        for line in results_file.readlines():
            if i % 7 == 0:
                iteracion = int(line.strip().split(" ")[2])
            elif i % 7 == 1:
                tiempo = float(line.strip().split(" ")[2])
            elif i % 7 == 2:
                utilidad = float(line.strip().split(" ")[2])
            elif i % 7 == 3:
                rutas_solucion = int(line.strip().split(" ")[2])
            elif i % 7 == 4:
                rutas_generadas = int(line.strip().split(" ")[2])
            elif i % 7 == 5:
                probabilidades = clean(line.strip()).split(",")
                probabilidades = list(map(lambda x: float(x), probabilidades))
                # y = probabilidades
                # N = len(y)
                # x = range(N)
                # width = 1/1.5
                # plt.bar(x, y, width, color="blue")
                # plt.show()
            else:
                data.append({"iteracion":iteracion, "tiempo":tiempo,
                "utilidad":utilidad, "rutas_solucion":rutas_solucion,
                "rutas_generadas":rutas_generadas, "probabilidades":probabilidades})
            i += 1

    folders_data[folder] = data





for f in folders:
    utilities = []
    times = []
    routes = []
    generated = []
    for i in folders_data[f]:
        utilities.append(i["utilidad"])
        times.append(i["tiempo"])
        routes.append(i["rutas_solucion"])
        generated.append(i["rutas_generadas"])

    plt.plot(utilities, times, "o")
    # plt.plot(utilities, routes, "o")
    # plt.plot(utilities, generated, "o")
plt.show()

for f in folders:
    utilities = []
    times = []
    routes = []
    generated = []
    for i in folders_data[f]:
        utilities.append(i["utilidad"])
        times.append(i["tiempo"])
        routes.append(i["rutas_solucion"])
        generated.append(i["rutas_generadas"])

    # plt.plot(utilities, times, "o")
    plt.plot(utilities, routes, "o")
    # plt.plot(utilities, generated, "o")
plt.show()

for f in folders:
    utilities = []
    times = []
    routes = []
    generated = []
    for i in folders_data[f]:
        utilities.append(i["utilidad"])
        times.append(i["tiempo"])
        routes.append(i["rutas_solucion"])
        generated.append(i["rutas_generadas"])

    # plt.plot(utilities, times, "o")
    # plt.plot(utilities, routes, "o")
    plt.plot(utilities, generated, "o")
plt.show()
