#from gurobipy import*
import math
#from leer_AA10_def import* #No es necesario llamarlo si ya estan creadas las distancias

#Parametros
#numero de pedidos
pedidos=35
num_nodos = 70 #Cambie esto del codigo de FD
costos_penalizacion = 10.0 #Lo dejare fijo para todos los vuelos por ahora (esto facilita la construccion de una PB) OJO: Esto hay que arreglarlo para cuando tengamos vuelos prioritarios

d_0 ={ #Nodo depot inicial
 1: 101,
 2: 102,
 3: 103,
 4: 104,
 5: 105,
 6: 106,
 7: 107
 }

d_f ={ #Nodo depot final
 1: 711,
 2: 712,
 3: 713,
 4: 714,
 5: 715,
 6: 716,
 7: 717
 }


#VEHICULOS
aviones =[1,2,3,4,5,6,7]
capacidad ={
    1: 15,
    2: 15,
    3: 15,
    4: 15,
    5: 15,
    6: 15,
    7: 15
}

nodos_pickup ={
    1: [i for i in range(1,pedidos+1)],
    2: [i for i in range(1,pedidos+1)],
    3: [i for i in range(1,pedidos+1)],
    4: [i for i in range(1,pedidos+1)],
    5: [i for i in range(1,pedidos+1)],
    6: [i for i in range(1,pedidos+1)],
    7: [i for i in range(1,pedidos+1)]
}


#NODOS
nodos=[]
for i in range(num_nodos+1+1):
    nodos.append(i)

#Parametros del nodo
x={}
y={}
tviaje={}
peso={}
earliest={}
lastest={}
duracion={}
tarifa={}

file = open("AA30_mod.txt", "r")

i=0

for line in file:

    if i>0:
        word=line.split('\n')
        linea=word[0][3:100] #Preguntar
        linea2=linea.split('  ')
        x[i]=float(linea2[0])
        y[i]=float(linea2[1])

        try:
            duracion[i]=float(linea2[2])
            peso[i]=float(linea2[3])
        except:
            auxiliar=linea2[2].split(' ')
            duracion[i]=float(auxiliar[1])
            peso[i]=int(auxiliar[2])

        try:
            earliest[i]=float(linea2[-3])
            lastest[i]=float(linea2[-2])
            tarifa[i]=float(linea2[-1])
        except:
            auxiliar=linea2[5].split(' ')
            earliest[i]=float(auxiliar[0])
            lastest[i]=float(auxiliar[1])
            tarifa[i]=float(linea2[-1])
    i+=1
#leer informacion de los aviones
file = open("AA30_aviones.txt", "r")
x_a={}
y_a={}
tviaje_a={}
peso_a={}
earliest_a={}
lastest_a={}
duracion_a={}

i=-1
for line in file:

    if i>=0:
        word=line.split('\n')
        linea=word[0][4:100] #Preguntar
        linea2=linea.split('  ')
        x_a[i]=float(linea2[0])
        y_a[i]=float(linea2[1])
        try:
            earliest_a[i]=float(linea2[2])
            lastest_a[i]=float(linea2[3])
        except:
            auxiliar=linea2[2].split(' ')
            earliest_a[i]=float(auxiliar[1])
            lastest_a[i]=int(auxiliar[2])

        #print i," ", x_a[i], " ", y_a[i], " ", earliest_a[i], " ", lastest_a[i]
    i+=1

#Crea archivo de distancias
file = open("I_distancias_AA30_Rec.txt","w")


for i in range(1,2*pedidos+1):
    for j in range(1,2*pedidos+1):
        #if i==0 and j<d_f:
        #    tviaje[i,j]=math.pow(10,4)
        #    tviaje[j,i]=math.sqrt(math.pow(x[i]-x[j],2) + math.pow(y[i]-y[j],2))
        #else:
        tviaje[i,j]=math.sqrt(math.pow(x[i]-x[j],2) + math.pow(y[i]-y[j],2))
        tviaje[j,i]=tviaje[i,j]
        #print 'distancia['+str(i+1)+','+str(j+1)+']:', tviaje[i+1,j+1]
        file.write(str('distancia['+str(i)+','+str(j)+']:'+str(tviaje[i,j]))+"\n")

aux_1=0
aux_2=0
for i in range(0,2*len(aviones)):
    if (i+1)%2==0: #si es par
        aux_1+=1
    else:
        aux_2+=1
    for j in range(1,2*pedidos+1):
        tviaje_a[i+1,j]=math.sqrt(math.pow(x_a[i]-x[j],2) + math.pow(y_a[i]-y[j],2))
        tviaje_a[j,i+1]=tviaje_a[i+1,j]
        #print "tviaje_a["+str([j+1,i+1])+" :", tviaje_a[j+1,i+1]
        if (i+1)%2==0: #si es par
            #print 'distancia_a['+str(2*pedidos+1)+str(aux_1)+','+str(j+1)+']:', tviaje_a[i+1,j+1]
            aux_entero=int(str(2*pedidos+1)+str(aux_1))

            file.write(str('distancia['+str(2*pedidos+1)+str(aux_1)+','+str(j)+']:'+str(tviaje_a[i+1,j]))+"\n")
            file.write(str('distancia['+str(j)+','+str(2*pedidos+1)+str(aux_1)+']:'+str(tviaje_a[j,i+1]))+"\n")

            earliest[aux_entero]=earliest_a[i]
            lastest[aux_entero]=lastest_a[i]
            duracion[aux_entero]=0

            #print "earliest["+str(aux_entero)+str("] :"), earliest[aux_entero]
        else:
            aux_3=tviaje_a[j,i+1]
            tviaje_a[i+1,j]=math.pow(10,4)

            #print 'distancia_a[0'+str(aux_2)+','+str(j+1)+']:', tviaje_a[i+1,j+1]
            file.write(str('distancia[10'+str(aux_2)+','+str(j)+']:'+str(tviaje_a[i+1,j]))+"\n")
            #print aux_3
            file.write(str('distancia['+str(j)+','+'10'+str(aux_2)+']:'+str(aux_3))+"\n")
            aux_entero1=int(str(10)+str(aux_2))
            earliest[aux_entero1]=earliest_a[i]
            lastest[aux_entero1]=lastest_a[i]
            duracion[aux_entero1]=0

            #print "earliest["+str(aux_entero1)+str("] :"), earliest[aux_entero1]


        #print 'distancia_a['+str(i+1)+','+str(j+1)+']:', tviaje_a[i+1,j+1]
        #file.write(str('distancia_a['+str(i+1)+','+str(j+1)+']:'+str(tviaje_a[i+1,j+1]))+"\n")
aux_4=1
i=1
while i <2*len(aviones) :
    tviaje_a[i,i+1]=math.sqrt(math.pow(x_a[i-1]-x_a[i],2) + math.pow(y_a[i-1]-y_a[i],2))
    file.write(str('distancia[10'+str(aux_4)+','+str(2*pedidos+1)+str(aux_4)+']:'+str(tviaje_a[i,i+1]))+"\n")
    file.write(str('distancia['+str(2*pedidos+1)+str(aux_4)+','+'10'+str(aux_4)+']:'+str(tviaje_a[i,i+1]))+"\n")
    aux_4+=1
    i+=2

file.close()


#ARCOS
arcos={}
#arcos_pick_up={}
predecesor={}
for j in nodos:
    predecesor[j]=[]
'''
for i in nodos:
    arcos[i]=[]
    #arcos_pick_up[i]=[] #De que sirve??
    for j in nodos:
        if (earliest[i]+duracion[i]+tviaje[i,j]>lastest[j]) or (earliest[i]>lastest[j]):
            pass
            #print "infactible en tiempo arco :", str([i,j])
        elif i==j:
            pass
            #print 'infactible i=j :', str([i,j])
        #elif q[i]+q[j]>capacidad: #Donde estan definidos los q?? no es un valor fijo deberia ser eliminado   #### REVISAR
        #    print 'infactible carga qi+qj>CAP :', str([i,j])
        elif i==0 and j>pedidos and j!=num_nodos+1:
            pass
            #print "infaltible (0,n+i) :", str([i,j])
        elif i<=pedidos and j==num_nodos+1 and i!=0:
            pass
            #print "infactible (i,2n+1) :", str([i,j])
        elif i>pedidos and j==i-pedidos:
            pass
            #print "infactible (n+i,i) :", str([i,j])
        elif i==num_nodos+1:
            pass
            #print "infactible (2n+1,j) :", str([i,j])
        elif j==0:
            pass
            #print 'infactible (i,0) :', str([i,j])
        else:
            arcos[i].append(j)
            predecesor[j].append(i)
            #if j in pickup:
            #    arcos_pick_up[i].append(j) #De que sirve???
'''
##Abrir archivo distancia
file = open("I_distancias_AA30_Rec.txt", "r")
tviaje={}
costos={}
i=0
j=0
for line in file:
    word=line.split(':')
    #tviaje[i,j]=float(word[1])
    aux_p=word[0][10:]
    aux_p1=aux_p.split(',')
    #print aux_p1[0], aux_p1[1][:-1]
    #print word[0][10:]
    i=int(aux_p1[0])
    j=int(aux_p1[1][:-1])
    tviaje[i,j]=float(word[1])
    tviaje[j,i]=tviaje[i,j]
    costos[i,j]=tviaje[i,j]
    costos[j,i]=costos[i,j]
    #print "costos["+str(i),str(j)+"]", costos[i,j]
   # if num_nodos+1==j:
    #    j=0
    #    i=i+1
    #else:
    #    j=j+1
    #print "tviaje"+str([i,j])+" :", tviaje[i,j], "costos"+str([i,j])+" :", costos[i,j]

file.close()


#print "tviaje"+str([102,212]), tviaje[102,212]
#PLANIFICACION BASE
PB = {
    1: [101,30,65,24,59,1,36,8,19,54,43,711],
    2: [102,16,51,12,47,10,45,26,61,13,48,20,4,39,55,2,37,712],
    3: [103,11,46,21,56,27,62,713],
    4: [104,17,52,9,44,25,60,29,64,714],
    5: [105,18,53,28,63,15,50,7,42,715],
    6: [106,6,41,5,40,716],
    7: [107,14,49,3,38,23,58,22,57,717]
}

PB_vuelos={}
for k in aviones:
    PB_vuelos[k]=[]

for k in aviones:
    for pos in range(0,len(PB[k])-1):
        arc = (PB[k][pos],PB[k][pos+1])
       # print "arc :", arc
        PB_vuelos[k].append(arc)
    print PB_vuelos[k]

listmacronodos = {
    101: [101],
    102: [102],
    103: [103],
    104: [104],
    105: [105],
    106: [106],
    107: [107],
    1: [1],
    2: [2],
    3: [3],
    4: [4],
    5: [5],
    6: [6],
    7: [7],
    8: [8],
    9: [9],
    10: [10],
    11: [11],
    12: [12],
    13: [13],
    14: [14],
    15: [15],
    16: [16],
    17: [17],
    18: [18],
    19: [19],
    20: [20],
    21: [21],
    22: [22],
    23: [23],
    24: [24],
    25: [25],
    26: [26],
    27: [27],
    28: [28],
    29: [29],
    30: [30],
    31: [31],
    32: [32],
    33: [33],
    34: [34],
    35: [35],
    36: [36],
    37: [37],
    38: [38],
    39: [39],
    40: [40],
    41: [41],
    42: [42],
    43: [43],
    44: [44],
    45: [45],
    46: [46],
    47: [47],
    48: [48],
    49: [49],
    50: [50],
    51: [51],
    52: [52],
    53: [53],
    54: [54],
    55: [55],
    56: [56],
    57: [57],
    58: [58],
    59: [59],
    60: [60],
    61: [61],
    62: [62],
    63: [63],
    64: [64],
    65: [65],
    66: [66],
    67: [67],
    68: [68],
    69: [69],
    70: [70],
    711: [711],
    712: [712],
    713: [713],
    714: [714],
    715: [715],
    716: [716],
    717: [717]
}


class Ruta:
    def __init__(self):
        self.NumRuta = 0
        self.CostoRuta = 0
        self.Cargas = []
        self.q = []
        self.B = []
        self.Q = []
        self.nodos = []
        self.FO = 0
        self.Ingresos = 0
        self.Costos = 0
        #self.pond = 0 Sirve solo para el proceso de Mezcla
        self.fact = 1

    def __repr__(self):
        string = "nodes = " + str(self.nodos) + "\n"
        string += "fact = " + str(self.fact) + "\n"
        return string
