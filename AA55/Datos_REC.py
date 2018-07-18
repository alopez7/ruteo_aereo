#from gurobipy import*
import math 
#from leer_AA10_def import* #No es necesario llamarlo si ya estan creadas las distancias

#Parametros
#numero de pedidos
pedidos=58
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
 1: 1171,
 2: 1172,
 3: 1173,
 4: 1174,
 5: 1175,
 6: 1176,
 7: 1177

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

file = open("AA55_mod.txt", "r")

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
file = open("AA55_aviones.txt", "r")
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
file = open("I_distancias_AA55_Rec.txt","w")


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
file = open("I_distancias_AA55_Rec.txt", "r")
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

x_0[1, 11, 7]=1.0
x_0[2, 60, 4]=1.0
x_0[3, 61, 1]=1.0
x_0[4, 62, 4]=1.0
x_0[5, 63, 6]=1.0
x_0[6, 64, 5]=1.0
x_0[7, 65, 7]=1.0
x_0[8, 66, 3]=1.0
x_0[9, 67, 6]=1.0
x_0[10, 68, 3]=1.0
x_0[11, 69, 7]=1.0
x_0[12, 70, 6]=1.0
x_0[13, 71, 1]=1.0
x_0[14, 72, 4]=1.0
x_0[15, 73, 7]=1.0
x_0[16, 74, 1]=1.0
x_0[17, 47, 1]=1.0
x_0[18, 76, 2]=1.0
x_0[19, 32, 1]=1.0
x_0[20, 78, 1]=1.0
x_0[21, 79, 5]=1.0
x_0[22, 80, 1]=1.0
x_0[23, 81, 5]=1.0
x_0[24, 82, 3]=1.0
x_0[25, 83, 7]=1.0
x_0[26, 84, 2]=1.0
x_0[27, 85, 5]=1.0
x_0[28, 86, 2]=1.0
x_0[29, 87, 5]=1.0
x_0[30, 88, 1]=1.0
x_0[31, 89, 5]=1.0
x_0[32, 90, 1]=1.0
x_0[33, 91, 4]=1.0
x_0[34, 92, 6]=1.0
x_0[35, 93, 3]=1.0
x_0[36, 94, 7]=1.0
x_0[37, 95, 6]=1.0
x_0[38, 96, 6]=1.0
x_0[39, 97, 6]=1.0
x_0[40, 98, 7]=1.0
x_0[41, 99, 3]=1.0
x_0[42, 100, 2]=1.0
x_0[43, 101, 3]=1.0
x_0[44, 102, 4]=1.0
x_0[45, 103, 3]=1.0
x_0[46, 104, 2]=1.0
x_0[47, 105, 1]=1.0
x_0[48, 106, 3]=1.0
x_0[49, 107, 4]=1.0
x_0[50, 108, 1]=1.0
x_0[51, 109, 5]=1.0
x_0[52, 110, 5]=1.0
x_0[53, 111, 4]=1.0
x_0[54, 112, 1]=1.0
x_0[55, 113, 2]=1.0
x_0[59, 36, 7]=1.0
x_0[60, 4, 4]=1.0
x_0[61, 50, 1]=1.0
x_0[62, 33, 4]=1.0
x_0[63, 38, 6]=1.0
x_0[64, 21, 5]=1.0
x_0[65, 1177, 7]=1.0
x_0[66, 41, 3]=1.0
x_0[67, 12, 6]=1.0
x_0[68, 35, 3]=1.0
x_0[69, 59, 7]=1.0
x_0[70, 34, 6]=1.0
x_0[71, 20, 1]=1.0
x_0[72, 44, 4]=1.0
x_0[73, 7, 7]=1.0
x_0[74, 30, 1]=1.0
x_0[75, 16, 1]=1.0
x_0[76, 28, 2]=1.0
x_0[77, 13, 1]=1.0
x_0[78, 1171, 1]=1.0
x_0[79, 51, 5]=1.0
x_0[80, 19, 1]=1.0
x_0[81, 52, 5]=1.0
x_0[82, 10, 3]=1.0
x_0[83, 15, 7]=1.0
x_0[84, 18, 2]=1.0
x_0[85, 29, 5]=1.0
x_0[86, 55, 2]=1.0
x_0[87, 1175, 5]=1.0
x_0[88, 54, 1]=1.0
x_0[89, 23, 5]=1.0
x_0[90, 77, 1]=1.0
x_0[91, 1174, 4]=1.0
x_0[92, 39, 6]=1.0
x_0[93, 45, 3]=1.0
x_0[94, 25, 7]=1.0
x_0[95, 1176, 6]=1.0
x_0[96, 37, 6]=1.0
x_0[97, 5, 6]=1.0
x_0[98, 1, 7]=1.0
x_0[99, 43, 3]=1.0
x_0[100, 26, 2]=1.0
x_0[101, 48, 3]=1.0
x_0[102, 2, 4]=1.0
x_0[103, 8, 3]=1.0
x_0[104, 1172, 2]=1.0
x_0[105, 75, 1]=1.0
x_0[106, 1173, 3]=1.0
x_0[107, 14, 4]=1.0
x_0[108, 22, 1]=1.0
x_0[109, 27, 5]=1.0
x_0[110, 6, 5]=1.0
x_0[111, 49, 4]=1.0
x_0[112, 3, 1]=1.0
x_0[113, 46, 2]=1.0
x_0[1001, 17, 1]=1.0
x_0[1002, 42, 2]=1.0
x_0[1003, 24, 3]=1.0
x_0[1004, 53, 4]=1.0
x_0[1005, 31, 5]=1.0
x_0[1006, 9, 6]=1.0
x_0[1007, 40, 7]=1.0
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
for i in range(1171,1178):
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
