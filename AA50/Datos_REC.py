#from gurobipy import*
import math 
#from leer_AA10_def import* #No es necesario llamarlo si ya estan creadas las distancias

#Parametros
#numero de pedidos
pedidos=52
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
 1: 1051,
 2: 1052,
 3: 1053,
 4: 1054,
 5: 1055,
 6: 1056,
 7: 1057
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

file = open("AA50_mod.txt", "r")

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
file = open("AA50_aviones.txt", "r")
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
file = open("I_distancias_AA50_Rec.txt","w")


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
file = open("I_distancias_AA50_Rec.txt", "r")
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

x_0[1, 53, 3]=1.0
x_0[2, 54, 5]=1.0
x_0[3, 55, 3]=1.0
x_0[4, 56, 2]=1.0
x_0[5, 57, 6]=1.0
x_0[6, 58, 7]=1.0
x_0[7, 59, 4]=1.0
x_0[8, 60, 7]=1.0
x_0[9, 61, 5]=1.0
x_0[10, 62, 3]=1.0
x_0[11, 83, 6]=1.0
x_0[12, 64, 5]=1.0
x_0[13, 65, 5]=1.0
x_0[14, 66, 4]=1.0
x_0[15, 67, 4]=1.0
x_0[16, 68, 3]=1.0
x_0[17, 69, 7]=1.0
x_0[18, 70, 1]=1.0
x_0[19, 71, 5]=1.0
x_0[20, 72, 6]=1.0
x_0[21, 73, 2]=1.0
x_0[22, 74, 7]=1.0
x_0[23, 42, 2]=1.0
x_0[24, 76, 4]=1.0
x_0[25, 77, 5]=1.0
x_0[26, 78, 7]=1.0
x_0[27, 79, 2]=1.0
x_0[28, 80, 5]=1.0
x_0[29, 81, 7]=1.0
x_0[30, 82, 4]=1.0
x_0[31, 11, 6]=1.0
x_0[32, 84, 4]=1.0
x_0[33, 85, 7]=1.0
x_0[34, 86, 6]=1.0
x_0[35, 87, 5]=1.0
x_0[36, 88, 1]=1.0
x_0[37, 89, 4]=1.0
x_0[38, 90, 1]=1.0
x_0[39, 91, 1]=1.0
x_0[40, 92, 6]=1.0
x_0[41, 93, 6]=1.0
x_0[42, 94, 2]=1.0
x_0[43, 95, 1]=1.0
x_0[44, 96, 1]=1.0
x_0[45, 97, 4]=1.0
x_0[46, 98, 5]=1.0
x_0[47, 99, 2]=1.0
x_0[48, 100, 3]=1.0
x_0[49, 101, 1]=1.0
x_0[50, 102, 3]=1.0
x_0[53, 3, 3]=1.0
x_0[54, 46, 5]=1.0
x_0[55, 50, 3]=1.0
x_0[56, 27, 2]=1.0
x_0[57, 41, 6]=1.0
x_0[58, 33, 7]=1.0
x_0[59, 37, 4]=1.0
x_0[60, 6, 7]=1.0
x_0[61, 12, 5]=1.0
x_0[62, 1, 3]=1.0
x_0[63, 34, 6]=1.0
x_0[64, 35, 5]=1.0
x_0[65, 2, 5]=1.0
x_0[66, 45, 4]=1.0
x_0[67, 7, 4]=1.0
x_0[68, 10, 3]=1.0
x_0[69, 22, 7]=1.0
x_0[70, 39, 1]=1.0
x_0[71, 13, 5]=1.0
x_0[72, 1056, 6]=1.0
x_0[73, 4, 2]=1.0
x_0[74, 26, 7]=1.0
x_0[75, 21, 2]=1.0
x_0[76, 30, 4]=1.0
x_0[77, 19, 5]=1.0
x_0[78, 8, 7]=1.0
x_0[79, 1052, 2]=1.0
x_0[80, 25, 5]=1.0
x_0[81, 1057, 7]=1.0
x_0[82, 14, 4]=1.0
x_0[83, 63, 6]=1.0
x_0[84, 15, 4]=1.0
x_0[85, 29, 7]=1.0
x_0[86, 5, 6]=1.0
x_0[87, 28, 5]=1.0
x_0[88, 18, 1]=1.0
x_0[89, 1054, 4]=1.0
x_0[90, 1051, 1]=1.0
x_0[91, 43, 1]=1.0
x_0[92, 31, 6]=1.0
x_0[93, 20, 6]=1.0
x_0[94, 75, 2]=1.0
x_0[95, 38, 1]=1.0
x_0[96, 36, 1]=1.0
x_0[97, 32, 4]=1.0
x_0[98, 1055, 5]=1.0
x_0[99, 23, 2]=1.0
x_0[100, 1053, 3]=1.0
x_0[101, 44, 1]=1.0
x_0[102, 48, 3]=1.0
x_0[1001, 49, 1]=1.0
x_0[1002, 47, 2]=1.0
x_0[1003, 16, 3]=1.0
x_0[1004, 24, 4]=1.0
x_0[1005, 9, 5]=1.0
x_0[1006, 40, 6]=1.0
x_0[1007, 17, 7]=1.0
'''
PB = {
    1: [101,16,63,14,61,42,89,41,88,21,68,951],
    2: [102,24,71,1,11,58,48,18,65,15,62,4,7,54,51,37,84,952],
    3: [103,31,78,44,91,23,70,6,53,33,80,29,76,953],
    4: [104,17,64,3,50,36,83,34,81,28,75,25,72,13,60,20,67,954],
    5: [105,10,57,8,32,79,55,5,52,955],
    6: [106,9,56,12,59,22,69,45,92,19,43,66,90,38,85,2,49,956],
    7: [107,40,87,30,77,35,82,26,73,39,86,27,74,957]   
}
'''

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
seq = [i for i in range(1,num_nodos+1)]

listmacronodos={}
listmacronodos = listmacronodos.fromkeys(seq)
#print "New Dictionary : %s" %  str(dict)

for i in range(1,num_nodos+1):
    listmacronodos[i]=[i]

#depot final
for i in range(1051,1058):
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