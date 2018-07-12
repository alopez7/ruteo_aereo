from gurobipy import*
import time
import random
from Datos import *
from codigo_Maestro import *
from codigo_ANS import *

class Main_AA45():
	def __init__(self):
		#self.NumCorrida = 0
		self.TpoCorrida = 0
		self.U_relajada = 0
		self.U_final = 0
		self.PB = [] #Lista que almacena la PB como una secuencia de nodos por cada avion
		self.PB.append(0) #Esto lo hago solo para que los indices de PB representen el numero del avion
		self.R = [] #Columna base - Se utiliza como PB cuando esta no existe, de lo contrario, sirve como nuevo punto de partida diferente a la PB
		for k in aviones:
			self.PB.append(0)
		#global vuelos #Definirla como 'global' permite que la variable se pueda llamar desde cualquier otra clase
		self.vuelos = []
		self.vuelos.append(0) #Esto lo hago solo para que los indices de 'vuelos' representen el numero del avion
		for k in aviones:
			self.vuelos.append([])

	def Iniciar(self):
		tiempo = time.time()
		#INICIALIZACION - Construccion de columnas iniciales
		PM = Maestro()
		PM.It = 0

		self.R = self.Construir_CB() #Lo ejecutamos si o si para que sirva como nueva columna inicial del Problema Maestro
		GenerarPB = 1 #Variable que determina si hay que generar PB (=1) o no (=0)

		if GenerarPB == 0:
			self.PB = PB #Dato ingresado en archivo Datos.py
			for k in aviones: #Esto hay que corregirlo cuando hablemos de aeropuertos
				for pos in range(0,len(self.PB[k])-1):
					arc = (self.PB[k][pos],self.PB[k][pos+1])
					self.vuelos[k].append(arc)	
		else:
			for k in aviones:
				self.PB[k] = self.R[k-1].nodos #Se utiliza la Columna Base generada como PB del avion
				for pos in range(0,len(self.PB[k])-1):
					arc = (self.PB[k][pos],self.PB[k][pos+1])
					self.vuelos[k].append(arc)

		for k in aviones:
			sp = ANS(k)
			sp.vuelos = self.vuelos[k]
			#for i in range(0,1+num_nodos/2):
			#	sp.pesos.append(peso[i])
			
			sp = self.Generar_ColumnasIniciales(sp) #Generamos columnas iniciales del subproblema
			#for i in range(8):
			#	print sp.rutas[i].nodos
			
			PM.SPs.append(sp) #Agregamos subproblema al problema maestro
			##########################################################################
			##########################################################################
			'''
			for r in PM.SPs[k-1].rutas: #Imprimimos columnas iniciales del subproblema k
				self.ImprimirResultado(PM.It,k,r,'-') #ImprimirResultado(def, It, k, r, CostoReducido)
			'''
			##########################################################################
			##########################################################################
		#GENERACION DE COLUMNAS (GC):
		fin = 0
		while fin==0:
			PM.It = PM.It + 1
			duales = PM.OptimizarMaestro()
			dual_pi = duales[0]
			dual_gamma = duales[1]
			
			#Resolucion de subproblemas:
			for k in aviones:
				dual_pi_k = []
				for i in range(0,len(dual_pi)): #Discriminamos las variables duales de pickup que corresponden a los de cada avion
					if (i+1 in nodos_pickup[k]) == True: #El pedido al que corresponde el dual_pi esta dentro de la lista de nodos_pickup del avion k
						dual_pi_k.append(dual_pi[i])
				
				pos_r = random.randint(0,len(PM.SPs[k-1].rutas)-1) #Esto permite elegir aleatoriamente la ruta inicial que se ingresa al ANS
				#print "pos_r :",pos_r

				r = PM.SPs[k-1].Ejecutar(dual_pi_k, dual_gamma[k-1], PM.SPs[k-1].rutas[pos_r]) #Resolvemos subproblema
				#print "ruta ejecutar :", PM.SPs[k-1].rutas[pos_r].nodos
				#print "costo ejecutar :", PM.SPs[k-1].rutas[pos_r].CostoRuta
				#print "ruta generada :", r.nodos
				#print "costo generado :", r.CostoRuta

				
				if PM.SPs[k-1].CostoReducido > 0.000001:
					PM.SPs[k-1].rutas.append(r) #Agregamos la columna generada por el subproblema al problema maestro
					##########################################################################
					##########################################################################
					'''
					self.ImprimirResultado(PM.It, k, r, PM.SPs[k-1].CostoReducido) #ImprimirResultado(def, It, k, r, CostoReducido):
					'''
					##########################################################################
					##########################################################################
			#Test de parada GC: comprobamos que se haya generado alguna columna:
			fin = 1
			for k in aviones:
			 	if PM.SPs[k-1].CostoReducido > 0.000001: #Si algun costo reducido es mayor a 0 seguimos iterando.
			 		fin = 0

		#POST-GENERACION DE COLUMNAS
		PM.It = "Entero" #Tenemos el problema maestro final y lo corremos como un IP
		PM.OptimizarMaestro()
		self.U_relajada = PM.U_relajada
		self.U_final = PM.U_final

		tiempo = time.time()-tiempo
		self.TpoCorrida = tiempo

	def Generar_ColumnasIniciales(self,sp):
		#tiempo=time.time()
		#Columnas Iniciales
		#Ruta o Columna 1 : PB 'Factibilizada' - Modulo de factibilizacion de la PB
		r1 = Ruta()
		r1.NumRuta = 1
		r1.nodos = self.PB[sp.k] #self.PB es la secuencia de nodos de cada avion que representa su planificacion base
		#print "ruta col iniciales :", r1.nodos
		r1 = sp.AsignarTiempos(r1)
		#print "Factible? :", r1.fact
		if r1.fact == 1:
			#print "factible"
			r1 = sp.AsignarCargas(r1)
			for i in range(1,len(r1.nodos)-2): #[1,...,d_f - 1]
				if (r1.nodos[i] in nodos_pickup[sp.k])==True and r1.q[i] == 0: #Pedido con carga nula, conviene eliminarlo porque podria ser infactible
					r1 = sp.EliminarPedido(r1.nodos[i],r1) #Elimina el nodo pickup y delivery
			U1 = sp.Utilidad(r1)
			r1.CostoRuta = U1
		#print "nodos generar col :", r1.nodos
		#print "cargas generar col:", r1.q	
		r1 = sp.GuardarCargas(r1)
		sp.rutas.append(r1)

		#Ruta o Columna 2 : PB + DropAndAdd
		r2 = sp.Copiar(r1)
		r2 = sp.Drop_and_Add_v(r2)
		U2 = 0
		while U2 != r2.CostoRuta:
			U2 = r2.CostoRuta
			r2 = sp.Drop_and_Add_v(r2) #Variante de la original, que solo cambia la funcion que evalua la ruta (Utilidad en vez de FO)
		
		'''
		r2.NumRuta = len(sp.rutas)+1
		r2 = sp.GuardarCargas(r2)
		sp.rutas.append(r2)
		'''
		
		##################################
		if U2 != U1:
			#print "ruta 2"
			#print r2.nodos
			r2.NumRuta = len(sp.rutas)+1
			#print r2.NumRuta
			r2 = sp.GuardarCargas(r2)
			sp.rutas.append(r2)
		
		##################################
		#Ruta o Columna 3 : PB + Swap
		r3 = sp.Copiar(r2)
		r3 = sp.Swap_v(r3)
		U3 = 0
		while U3 != r3.CostoRuta:
			U3 = r3.CostoRuta
			r3 = sp.Swap_v(r3)
		'''	
		r3.NumRuta = len(sp.rutas)+1
		r3 = sp.GuardarCargas(r3)
		sp.rutas.append(r3)
		'''
		
		##################################	
		if U3 != U1 and U3 != U2:
			#print "ruta 3"
			#print r3.nodos
			r3.NumRuta = len(sp.rutas)+1
			#print r3.NumRuta
			r3 = sp.GuardarCargas(r3)
			sp.rutas.append(r3)
		
		##################################
		#Ruta o Columna 4 : Posibilidad de no utilizar el avion
		r4 = Ruta()
		aux = self.PB[sp.k]
		#print "columna inicial", aux
		r4.nodos = [aux[0],aux[-1]]
		#r4.nodos = [d_0,d_f]
		r4.q = [0,0]
		r4.Q = [0,0]
		r4.B = [0,0]
		U4 = sp.Utilidad(r4)
		'''	
		r4.NumRuta = len(sp.rutas)+1
		r4.CostoRuta = U4
		r4 = sp.GuardarCargas(r4)#REVISAR QUE NO SE ESTE CAYENDO POR LA FALTA DE "GUARDARCARGAS"
		sp.rutas.append(r4)
		'''
		##################################
		if U4 != U1 and U4 != U2 and U4 != U3:
			#print "ruta 4"
			#print r4.nodos
			r4.NumRuta = len(sp.rutas)+1
			#print r4.NumRuta
			r4.CostoRuta = U4
			r4 = sp.GuardarCargas(r4)#REVISAR QUE NO SE ESTE CAYENDO POR LA FALTA DE "GUARDARCARGAS"
			sp.rutas.append(r4)
		
		##################################
		#Nueva Columna Base
		#Ruta o Columna 5 : Columna Base (CB)
		r5 = self.R[sp.k-1]
		U5 = sp.Utilidad(r5) #Los costos de cancelacion van a ser respecto a la PB porque asi esta formulado el problema
		'''
		r5.NumRuta = len(sp.rutas)+1
		r5.CostoRuta = U5
		r5 = sp.GuardarCargas(r5)
		sp.rutas.append(r5)
		'''
		##################################
		if U5 != U1 and U5 != U2 and U5 != U3 and U5 != U4:
			#print "ruta 5"
			#print r5.nodos
			r5.NumRuta = len(sp.rutas)+1
			r5.CostoRuta = U5
			r5 = sp.GuardarCargas(r5)
			sp.rutas.append(r5)
		
		##################################
		#Ruta o Columna 6 : CB + DropAndAdd
		r6 = sp.Copiar(r5)
		r6 = sp.Drop_and_Add_v(r5)
		U6 = 0
		while U6 != r6.CostoRuta:
			U6 = r6.CostoRuta
			r6 = sp.Drop_and_Add_v(r6)
		'''
		r6.NumRuta = len(sp.rutas)+1
		r6 = sp.GuardarCargas(r6)
		sp.rutas.append(r6)
		'''
		##################################
		if U6 != U1 and U6 != U2 and U6 != U3 and U6 != U4 and U6 != U5:
			#print "ruta 6"
			#print r6.nodos
			r6.NumRuta = len(sp.rutas)+1
			r6 = sp.GuardarCargas(r6)
			sp.rutas.append(r6)
		
		##################################
		#Ruta o Columna 7 : CB + Swap
		r7 = sp.Copiar(r6)
		r7 = sp.Swap_v(r6)
		U7 = 0
		while U7 != r7.CostoRuta:
			U7 = r7.CostoRuta
			r7 = sp.Swap_v(r7)
		'''
		r7.NumRuta = len(sp.rutas)+1
		r7 = sp.GuardarCargas(r7)
		sp.rutas.append(r7)
		'''
		##################################
		if U7 != U1 and U7 != U2 and U7 != U3 and U7 != U4 and U7 != U5 and U7 != U6:
			#print "ruta 7"
			#print r7.nodos
			r7.NumRuta = len(sp.rutas)+1
			r7 = sp.GuardarCargas(r7)
			sp.rutas.append(r7)
		
		##################################
		#t_final=time.time()-tiempo
		#print "tiempo columnas iniciales :", t_final
		return sp

	def Construir_CB(self):
		#Insercion paralela
		N = [] #list() #Lista de nodos pendientes de insertar
		R = [] #list() #Almacena las rutas finales de cada avion
		K = aviones[:] #Lista con numero de aviones aleatorios
		N_global = range(1,num_nodos/2+1) #Nodos pickup de la red

		for k in aviones:
			N.append(k) #Hay un conjunto por cada avion
			#print N
			N[k-1] = nodos_pickup[k][:] #cada elemento del conjunto N tiene asociada una ruta (secuencia de nodos)
			r = Ruta()
			r.nodos = [d_0[k],d_f[k]]
			R.append(r)
			#print "R :", R[k-1].nodos

		aux = 0

		while (len(N_global) != 0) and (aux < len(K)):
			#print "N_global", N_global
			aux = 0 #Identifica cuantos vehiculos no pudieron insertar un pedido, si no se pudieron asignar pedidos al total de vehiculos (aux==len(K)) el bucle finaliza	
			#Elegir aleatoriamente un avion de la lista
			random.shuffle(K)
		
			for k in K:
				D_k = R[k-1].nodos[:] #D_k = [d_0,d_f]
				#print "D_k: ", D_k
				R_k = [] #Lista que almacena el costo de insercion del pedido y la ruta que genera
				sp = ANS(k) #Variable auxiliar que nos permitira llamar a los metodos Asignar
				
				N_aux = N[k-1][:]
				#print "N_k: ", N_aux
				for i in N_aux:
					if (i in N_global) == False:
						N[k-1].remove(i)

				for i in N[k-1]:
					r = Ruta()
					r.nodos = D_k[:]
					if len(D_k) == 2: #D_k = [d_0,d_f]
						c_i = costos[d_0[k],i]+ costos[i,i+num_nodos/2]+costos[i+num_nodos/2,d_f[k]]
						r.nodos.insert(1,i+num_nodos/2)
						r.nodos.insert(1,i)
					else:
						j = D_k[len(D_k)-2] #ultimo nodo delivery en la ruta
						pos_j = D_k.index(j)
						c_i = costos[j,i]+ costos[i,i+num_nodos/2]+costos[i+num_nodos/2,d_f[k]]
						r.nodos.insert(pos_j+1,i+num_nodos/2) #insertamos los nodos despues del ultimo delivery
						r.nodos.insert(pos_j+1,i)
					
					r = sp.AsignarTiempos(r) #Chequeamos factibilidad de tiempos
						
					if r.fact == 1:
						r = sp.AsignarCargas(r) 
						#Si no es posible llevar la carga de un pedido en la ruta definida('infactible' en capacidad), la carga asignada a ese pedido sera 0 
						#Entonces esta heuristica al no considerar la funcion objetivo o utilidad, puede generar rutas iniciales no rentables (esto tiene la ventaja de ampliar la diversidad, pero si se considera como PB nos podria dejar encerrados en un optimo local)
						#Por este motivo, ahora asumiremos que si la carga del nodo es 0 no conviene guardar la ruta
						if r.q[r.nodos.index(i)] != 0:
							#D_k = r.nodos[:]
							R_k.append([c_i,r,i])
							
				if len(R_k) != 0: #Tenemos elementos que se pueden insertar:
					R_k.sort() #Ordenamos de menor a mayor costo
					R[k-1] = R_k[0][1] #Entrega el objeto Ruta() de la insercion de menor costo
					N_global.remove(R_k[0][2])#Eliminamos el pedido respectivo de la lista de nodos pendientes en la red
				else:
					aux += 1
		'''
		#print "SALIR"
		#print K
		for k in K:
			#print "k :",k
			sp.k=k
			aux_exit=0
			while (len(N_global)) != 0 and aux_exit==0:
				N_aux=[i for i in N_global]
				#print "N_global_aux", N_aux
				print "N_global", N_global
				R[k-1],N_global=sp.Insercion_inicial(R[k-1],N_global)
				#print "N_global1", N_global
				#print "N_global_aux", N_aux
				#print "len(N_global_aux", len(N_aux), "len(N_global)",len(N_global)
				if len(N_aux)==len(N_global):
					aux_exit=1
					print "aux_exit :", aux_exit
				#print "R"+str([k]), R[k-1].nodos	
				#print "N_global2", N_global
		'''		
		aux_exit=0
		while (len(N_global)) != 0 and aux_exit==0:
			for k in K:
				sp.k=k
				N_aux=[i for i in N_global]
				#print "N_global_aux", N_aux
				#print "N_global", N_global
				R[k-1],N_global=sp.Insercion_inicial(R[k-1],N_global)
				#print "N_global1", N_global
				#print "N_global_aux", N_aux
				#print "len(N_global_aux", len(N_aux), "len(N_global)",len(N_global)
				if len(N_aux)==len(N_global):
					aux_exit=1
					#print "aux_exit :", aux_exit
				#print "R"+str([k]), R[k-1].nodos	
				#print "N_global2", N_global
		return R

	def ImprimirResultado(self, It, k, r, CostoReducido):
		nombre_archivo = "Iteracion " + str(It) + " Ruta " + str(r.NumRuta) + " Subproblema_" + str(k)+".txt"
		
		f = open(str(nombre_archivo),"w")

		f.write("Avion: "+str(k)+"      Iteracion: "+str(It)+"\n"+"\n")
		f.write("Num Ruta: "+str(r.NumRuta)+"\n")
		f.write("Ingresos: "+str(r.Ingresos)+"\n")
		f.write("Costos: "+str(r.Costos)+"\n")
		f.write("Utilidad Ruta: "+str(r.CostoRuta)+"\n")	
		f.write("Costo Reducido: "+str(CostoReducido)+"\n")
		f.write("Nodos: "+str(r.nodos)+"\n")
		f.write("q: "+str(r.q)+"\n")
		f.write("Q: "+str(r.Q)+"\n")
		f.write("B: "+str(r.B)+"\n")

		f.close()

	def write_table(self, file_route, table):
		f = open(str(file_route),"w")
		for item in table:
			f.write(str(item)+"\n")
		f.close()


#Ejecutar algoritmo:
#m  = Main_AA10()
#m.Iniciar()
