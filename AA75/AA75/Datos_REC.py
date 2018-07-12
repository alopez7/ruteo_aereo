#from gurobipy import*
import math
#from leer_AA10_def import* #No es necesario llamarlo si ya estan creadas las distancias

#Parametros
#numero de pedidos
pedidos=78
num_nodos = 2*pedidos #Cambie esto del codigo de FD
costos_penalizacion = 10.0 #Lo dejare fijo para todos los vuelos por ahora (esto facilita la construccion de una PB) OJO: Esto hay que arreglarlo para cuando tengamos vuelos prioritarios

d_0 ={ #Nodo depot inicial
 1: 1001,
 2: 1002,
 3: 1003,
 4: 1004,
 5: 1005,
 6: 1006,
 7: 1007
 }

d_f ={ #Nodo depot final
 1: 1571,
 2: 1572,
 3: 1573,
 4: 1574,
 5: 1575,
 6: 1576,
 7: 1577

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

nodos_aux=[]
for i in range(1,num_nodos+1+1):
	nodos_aux.append(i)

for i in d_0:
	nodos_aux.append(d_0[i])

for i in d_f:
	nodos_aux.append(d_f[i])

#print "nodos_aux :", nodos_aux
#Parametros del nodo
x={}
y={}
tviaje={}
peso={}
earliest={}
lastest={}
duracion={}
tarifa={}

file = open("AA75_mod.txt", "r")

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
file = open("AA75_aviones.txt", "r")
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
file = open("I_distancias_AA75_Rec.txt","w")


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
            file.write(str('distancia[100'+str(aux_2)+','+str(j)+']:'+str(tviaje_a[i+1,j]))+"\n")
            #print aux_3
            file.write(str('distancia['+str(j)+','+'100'+str(aux_2)+']:'+str(aux_3))+"\n")
            aux_entero1=int(str(100)+str(aux_2))
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
    file.write(str('distancia[100'+str(aux_4)+','+str(2*pedidos+1)+str(aux_4)+']:'+str(tviaje_a[i,i+1]))+"\n")
    file.write(str('distancia['+str(2*pedidos+1)+str(aux_4)+','+'100'+str(aux_4)+']:'+str(tviaje_a[i,i+1]))+"\n")
    aux_4+=1
    i+=2

file.close()

#ARCOS
arcos={}
#arcos_pick_up={}
predecesor={}
for j in nodos:
    predecesor[j]=[]

##Abrir archivo distancia
file = open("I_distancias_AA75_Rec.txt", "r")
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
x_0={}

for i in nodos_aux:
	for j in nodos_aux:
		for k in aviones:
			x_0[i,j,k]=0

x_0[1, 42, 1]=1.0
x_0[2, 80, 2]=1.0
x_0[3, 81, 3]=1.0
x_0[4, 82, 2]=1.0
x_0[5, 83, 1]=1.0
x_0[6, 153, 6]=1.0
x_0[7, 85, 4]=1.0
x_0[8, 86, 3]=1.0
x_0[9, 87, 7]=1.0
x_0[10, 88, 4]=1.0
x_0[11, 89, 7]=1.0
x_0[12, 90, 5]=1.0
x_0[13, 91, 4]=1.0
x_0[14, 92, 2]=1.0
x_0[15, 93, 4]=1.0
x_0[16, 94, 1]=1.0
x_0[17, 143, 6]=1.0
x_0[18, 96, 5]=1.0
x_0[19, 97, 2]=1.0
x_0[20, 4, 2]=1.0
x_0[21, 99, 3]=1.0
x_0[22, 100, 4]=1.0
x_0[23, 101, 3]=1.0
x_0[24, 102, 3]=1.0
x_0[25, 103, 6]=1.0
x_0[26, 104, 1]=1.0
x_0[27, 105, 5]=1.0
x_0[28, 106, 6]=1.0
x_0[29, 107, 3]=1.0
x_0[30, 108, 3]=1.0
x_0[31, 109, 4]=1.0
x_0[32, 110, 5]=1.0
x_0[33, 111, 6]=1.0
x_0[34, 112, 6]=1.0
x_0[35, 120, 1]=1.0
x_0[36, 114, 7]=1.0
x_0[37, 115, 5]=1.0
x_0[38, 145, 7]=1.0
x_0[39, 32, 5]=1.0
x_0[40, 118, 1]=1.0
x_0[41, 119, 3]=1.0
x_0[42, 79, 1]=1.0
x_0[43, 121, 3]=1.0
x_0[44, 122, 7]=1.0
x_0[45, 123, 2]=1.0
x_0[46, 124, 7]=1.0
x_0[47, 68, 2]=1.0
x_0[48, 126, 1]=1.0
x_0[49, 95, 6]=1.0
x_0[50, 128, 4]=1.0
x_0[51, 129, 5]=1.0
x_0[52, 130, 2]=1.0
x_0[53, 131, 5]=1.0
x_0[54, 132, 6]=1.0
x_0[55, 133, 4]=1.0
x_0[56, 134, 5]=1.0
x_0[57, 66, 3]=1.0
x_0[58, 31, 4]=1.0
x_0[59, 137, 6]=1.0
x_0[60, 138, 2]=1.0
x_0[61, 139, 4]=1.0
x_0[62, 140, 5]=1.0
x_0[63, 141, 7]=1.0
x_0[64, 142, 1]=1.0
x_0[65, 17, 6]=1.0
x_0[66, 135, 3]=1.0
x_0[67, 38, 7]=1.0
x_0[68, 125, 2]=1.0
x_0[69, 147, 7]=1.0
x_0[70, 148, 1]=1.0
x_0[71, 149, 2]=1.0
x_0[72, 150, 7]=1.0
x_0[73, 151, 4]=1.0
x_0[74, 152, 1]=1.0
x_0[75, 6, 6]=1.0
x_0[79, 35, 1]=1.0
x_0[80, 1572, 2]=1.0
x_0[81, 23, 3]=1.0
x_0[82, 98, 2]=1.0
x_0[83, 48, 1]=1.0
x_0[84, 33, 6]=1.0
x_0[85, 1574, 4]=1.0
x_0[86, 41, 3]=1.0
x_0[87, 11, 7]=1.0
x_0[88, 58, 4]=1.0
x_0[89, 44, 7]=1.0
x_0[90, 62, 5]=1.0
x_0[91, 15, 4]=1.0
x_0[92, 146, 2]=1.0
x_0[93, 7, 4]=1.0
x_0[94, 40, 1]=1.0
x_0[95, 127, 6]=1.0
x_0[96, 39, 5]=1.0
x_0[97, 52, 2]=1.0
x_0[98, 2, 2]=1.0
x_0[99, 29, 3]=1.0
x_0[100, 50, 4]=1.0
x_0[101, 8, 3]=1.0
x_0[102, 30, 3]=1.0
x_0[103, 75, 6]=1.0
x_0[104, 70, 1]=1.0
x_0[105, 37, 5]=1.0
x_0[106, 25, 6]=1.0
x_0[107, 1573, 3]=1.0
x_0[108, 57, 3]=1.0
x_0[109, 136, 4]=1.0
x_0[110, 117, 5]=1.0
x_0[111, 1576, 6]=1.0
x_0[112, 28, 6]=1.0
x_0[113, 26, 1]=1.0
x_0[114, 63, 7]=1.0
x_0[115, 1575, 5]=1.0
x_0[116, 46, 7]=1.0
x_0[117, 51, 5]=1.0
x_0[118, 74, 1]=1.0
x_0[119, 43, 3]=1.0
x_0[120, 113, 1]=1.0
x_0[121, 21, 3]=1.0
x_0[122, 36, 7]=1.0
x_0[123, 71, 2]=1.0
x_0[124, 1577, 7]=1.0
x_0[125, 14, 2]=1.0
x_0[126, 64, 1]=1.0
x_0[127, 59, 6]=1.0
x_0[128, 73, 4]=1.0
x_0[129, 27, 5]=1.0
x_0[130, 20, 2]=1.0
x_0[131, 12, 5]=1.0
x_0[132, 34, 6]=1.0
x_0[133, 13, 4]=1.0
x_0[134, 18, 5]=1.0
x_0[135, 144, 3]=1.0
x_0[136, 61, 4]=1.0
x_0[137, 54, 6]=1.0
x_0[138, 45, 2]=1.0
x_0[139, 22, 4]=1.0
x_0[140, 56, 5]=1.0
x_0[141, 72, 7]=1.0
x_0[142, 1571, 1]=1.0
x_0[143, 49, 6]=1.0
x_0[144, 3, 3]=1.0
x_0[145, 69, 7]=1.0
x_0[146, 60, 2]=1.0
x_0[147, 116, 7]=1.0
x_0[148, 5, 1]=1.0
x_0[149, 19, 2]=1.0
x_0[150, 67, 7]=1.0
x_0[151, 55, 4]=1.0
x_0[152, 1, 1]=1.0
x_0[153, 84, 6]=1.0
x_0[1001, 16, 1]=1.0
x_0[1002, 47, 2]=1.0
x_0[1003, 24, 3]=1.0
x_0[1004, 10, 4]=1.0
x_0[1005, 53, 5]=1.0
x_0[1006, 65, 6]=1.0
x_0[1007, 9, 7]=1.0

PB_vuelos={}
for k in aviones:
    PB_vuelos[k]=[]

for k in aviones:
	for i in nodos_aux:
		for j in nodos_aux:
			if x_0[i,j,k]==1:
				arc=(i,j)
				PB_vuelos[k].append(arc)
	#print PB_vuelos[k]

'''
for k in aviones:
    for pos in range(0,len(PB[k])-1):
        arc = (PB[k][pos],PB[k][pos+1])
       # print "arc :", arc
        PB_vuelos[k].append(arc)
    print PB_vuelos[k]
'''


'''Nodos que son iguales'''
seq = [i for i in range(1,num_nodos+1)]

listmacronodos={}
listmacronodos = listmacronodos.fromkeys(seq)
#print "New Dictionary : %s" %  str(dict)

for i in range(1,num_nodos+1):
    listmacronodos[i]=[i]

#depot final
for i in range(1571,1578):
    listmacronodos[i]=[i]

#depot inicial
for i in range(1001,1008):
    listmacronodos[i]=[i]

#print "New Dictionary 3: %s" %  str(listmacronodos)

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
