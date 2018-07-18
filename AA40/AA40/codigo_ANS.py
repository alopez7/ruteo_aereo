import time
import random
from gurobipy import*
from Datos import*
from codigo_Main import*
#import math 

class ANS():		
	def __init__(self, k):
		self.inicio = 1
		self.pendientes = []
		self.k = k
		self.vuelos = [] #Lo seteamos desde codigo_Main posterior a la creacion de la variable
		self.Iter = 0
		self.It = 0 
		self.costos = costos #Todos los aviones tienen el mismo costo
		self.tviaje = tviaje #Todos los aviones tienen la misma velocidad de viaje
		self.rutas = []
		self.CostoReducido = 0
		self.P = [] #Lista de probabilidades
		self.E = []
		for i in range(0,7):
			self.P.append(0.142857145)#Inicialmente son equiprobables*(i+1)
			self.E.append(0.142857145)#Inicialmente son equiprobables*(i+1)
		self.dual_pi = []
		for i in range(0,num_nodos/2):
			self.dual_pi.append(0)

	def Ejecutar(self, dual_pi, dual_gamma, ruta_inicial):#ruta_inicial: primera columna(ruta) de este avion que corresponde a la PB factibilizada
		self.dual_pi = dual_pi
		self.dual_gamma = dual_gamma
		P = self.P
		#tiempos = list()
	 	#tiempos.append("Heuristica	Tiempo(s)	FO Inicial	FO Final	Dif FO") #Encabezado
		count = 0

		#if self.k == 1:
		#	#print "Avion: ", str(self.k)
	 	##print "Inicializacion"
	 	count = time.time()
	 	#print "ruta inicio :", ruta_inicial.nodos
	 	r = self.Inicializacion(ruta_inicial)
	 	#r.NumRuta= 1000
	 	#print "Ruta Inicializacion: ", r.nodos
	 	#print "costo :", r.CostoRuta
	 	#tiempos.append("Inicializacion	"+str(time.time()-count)+"	-	"+str(self.FuncionObjetivo(r))+"	-")
		BestSolution = self.Copiar(r)
	 	BestUtility = r.FO
	 	#Temperatura=200000
	 	##print "Temperatura_inicial", Temperatura
 		Allow = 0
 		Local_Iter = 35 ################ DEBERIAMOS SENSIBILIZAR ESTO #####################
 		fo_nueva = 0
 		fo_inicial = BestUtility
 		tpo_rutina = 0.000000000001
 		bloq = []
 		for i in range(0,7):
 			bloq.append(0)

 		while Allow < Local_Iter:
 		#Inicio LocalSearch
 			aleatorio = random.random()
 			if (0 <= aleatorio <= P[0]) and bloq[0] == 0:
	 			##print "Paso 1: Drop and Add" 
	 			fo_inicial = self.FuncionObjetivo(r)
	 			count = time.time()
	 			r = self.Drop_and_Add(r)
	 			tpo_rutina = time.time()-count
	 			fo_nueva = self.FuncionObjetivo(r)
	 			#tiempos.append("Drop-And-Add	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
	 			self.Actualizar_Probabilidades(0, fo_inicial, fo_nueva, tpo_rutina)
	 			#if self.k == 1:
	 			#	#print "Ruta Inicializacion: ",r.nodos
	 			bloq[0] = 1
 			elif (P[0] < aleatorio <= P[0]+P[1]) and bloq[1] == 0:
	 			##print "Paso 2: Swap"
	 			fo_inicial = self.FuncionObjetivo(r)
	 			count = time.time()
	 			r = self.Swap(r)	
	 			tpo_rutina = time.time()-count
	 			fo_nueva = self.FuncionObjetivo(r)
	 			#tiempos.append("Swap	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
	 			self.Actualizar_Probabilidades(1, fo_inicial, fo_nueva, tpo_rutina)
	 			bloq[1] = 1
	 			#if self.k == 1:
	 			#	#print "Ruta Swap: ",r.nodos
 			elif (P[0]+P[1] < aleatorio <= P[0]+P[1]+P[2]) and bloq[2] == 0:
	 			##print "Paso 3: I-R-M-R-R"
	 			fo_inicial = self.FuncionObjetivo(r)
	 			count = time.time()
	 			for i in range(1,len(r.nodos)-1):
	 				if r.nodos[i] <= num_nodos/2: #Es Pickup
	 					r = self.Intra_route_multiple_request_relocate(r,r.nodos[i])
	 			tpo_rutina = time.time()-count
	 			fo_nueva = self.FuncionObjetivo(r)
	 			#tiempos.append("I-R-M-R-R	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
	 			self.Actualizar_Probabilidades(2, fo_inicial, fo_nueva, tpo_rutina)
	 			bloq[2] = 1
	 			#if self.k == 1:
	 			#	#print "Ruta I-R-M-R-R: ",r.nodos
 			elif (P[0]+P[1]+P[2] < aleatorio <= P[0]+P[1]+P[2]+P[3]) and bloq[3] == 0:
	 			##print "Paso 4: I-R-M-R-E"
	 			fo_inicial = self.FuncionObjetivo(r)
	 			count = time.time()
	 			r = self.Intra_route_multiple_request_exchange(r)
	 			tpo_rutina = time.time()-count
	 			fo_nueva = self.FuncionObjetivo(r)
	 			#tiempos.append("I-R-M-R-E	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
	 			self.Actualizar_Probabilidades(3, fo_inicial, fo_nueva, tpo_rutina)
	 			bloq[3] = 1
	 			#if self.k == 1:
	 			#	#print "Ruta I-R-M-R-E: ",r.nodos
 			elif (P[0]+P[1]+P[2]+P[3] < aleatorio <= P[0]+P[1]+P[2]+P[3]+P[4])  and bloq[4] == 0:
	 			##print "Paso 5: I-R-R-R"
	 			fo_inicial = self.FuncionObjetivo(r)
	 			count = time.time()
	 			for i in range(1,len(r.nodos)-1):
	 				if r.nodos[i] <= num_nodos/2: #Es Pickup
	 					r = self.Intra_route_request_relocate(r,r.nodos[i])
	 			tpo_rutina = time.time()-count
	 			fo_nueva= self.FuncionObjetivo(r)
	 			#tiempos.append("I-R-R-R	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
	 			self.Actualizar_Probabilidades(4, fo_inicial, fo_nueva, tpo_rutina)
	 			bloq[4] = 1
	 			#if self.k == 1:
	 			#	#print "Ruta I-R-R-R: ",r.nodos
	 		elif (P[0]+P[1]+P[2]+P[3]+P[4]< aleatorio <= P[0]+P[1]+P[2]+P[3]+P[4]+P[5])  and bloq[5] == 0:
	 			fo_inicial = self.FuncionObjetivo(r)
	 			count = time.time()
	 			r = self.Delete(r)
	 			tpo_rutina = time.time()-count
	 			fo_nueva = self.FuncionObjetivo(r)
	 			#tiempos.append("Delete	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
	 			self.Actualizar_Probabilidades(5, fo_inicial, fo_nueva, tpo_rutina)
	 			bloq[5] = 1	
 			else:
 				if bloq[6] == 0:
		 			##print "Paso 6: I-R-R-E"
		 			fo_inicial = self.FuncionObjetivo(r)
		 			count = time.time()
		 			r = self.Intra_route_request_exchange(r)
		 			#r, Temperatura = self.Intra_route_request_exchange(r, Temperatura)
		 			##print "Temperatura_1", Temperatura
		 			tpo_rutina = time.time() - count
		 			fo_nueva = self.FuncionObjetivo(r)
	 				#tiempos.append("I-R-R-E	"+str(tpo_rutina)+"	"+str(fo_inicial)+"	"+str(fo_nueva)+"	"+str(fo_nueva-fo_inicial))
		 			self.Actualizar_Probabilidades(5, fo_inicial, fo_nueva, tpo_rutina)
		 			bloq[6] = 1
		 			#if self.k == 1:
	 				#	#print "Ruta I-R-R-E: ",r.nodos
	 			
 			LocalUtility = self.FuncionObjetivo(r) #podria ser fo_nueva, pero en caso de que no entre a ninguno de los if este valor seria 0 y no es cierto.

	 		if LocalUtility > BestUtility:
 				BestSolution = self.Copiar(r) #Copia todos los atributos de la ruta r
 				BestUtility = LocalUtility
 				for i in range(0,7):
 					bloq[i] = 0
 			Allow = Allow + 1
 			if bloq[0]+bloq[1]+bloq[2]+bloq[3]+bloq[4]+bloq[5]+bloq[6] == 7:
 				Allow = Local_Iter
 		#Fin LocalSearch
		#tiempos.append("Probabilidades" + str(P)+ " Suma = "+str(sum(P)))
		BestSolution = self.GuardarCargas(BestSolution)
		BestSolution.NumRuta = len(self.rutas)+1
		BestSolution.CostoRuta = self.Utilidad(BestSolution) #Guarda la utilidad de la ruta y no su valor de funcion objetivo
		
		self.CostoReducido = BestUtility

		#Guardar #Tiempos
		#self.Iter = self.Iter + 1
		#file_route = "#Tiempos-Utilidades Iteracion "+str(self.Iter) +" Avion"+ str(self.k)+".txt"
		#self.write_table(file_route,#tiempos) #Borrado este metodo

		return BestSolution

	def Inicializacion(self, ruta_inicial): #Esta heuristica de Inicializacion prueba la eliminacion secuencial de los peores pedidos en la PB
		r = self.Copiar(ruta_inicial) #ruta_inicial #primera columna(ruta) de este avion que corresponde a la PB factibilizada
		rCopia = self.Copiar(r)
		maxUtilidad = self.FuncionObjetivo(r)

		list_orden = [] #Lista que almacena la utilidad que genera llevar un pedido y el numero del pedido

		ruta= r.nodos
		for i in range(1,len(ruta)):
			if (ruta[i] in nodos_pickup[self.k]) == True:
				pos_i = nodos_pickup[self.k].index(ruta[i])
				u = tarifa[ruta[i]]*r.q[i] - self.costos[ruta[i-1],ruta[i]] - self.dual_pi[pos_i]*r.q[i] #El costo de cancelacion no importa porque es igual a todos los pedidos de la ruta, eliminar cualquiera de estos genera el mismo costo de cancelacion
				list_orden.append([u,ruta[i]])
		list_orden.sort() #Ordenamos la lista de menor a mayor utilidad (definida como la diferencia entre ingresos, costos op y costos duales)

		for i in list_orden: #A continuacion se va probando la eliminacion SECUENCIAL de cada pedido
			p = i[1] #Elemento de la dupla que indica el nodo pedido (pickup)
			rCopia = self.EliminarPedido(p,rCopia)
			rCopia = self.AsignarTiempos(rCopia)
			#print "RUTA CANDIDATA:" , rCopia.nodos
			if rCopia.fact == 1:
				rCopia = self.AsignarCargas(rCopia)
				U = self.FuncionObjetivo(rCopia)
				if maxUtilidad < U:
					maxUtilidad = U
					r.CostoRuta = maxUtilidad
					r = self.Copiar(rCopia)
			rCopia = self.Copiar(r) #Copiamos la ultima mejor alternativa de ruta guardada
		r.FO = maxUtilidad
		return r
		
	#Heuristicas Local Search:
	def Intra_route_request_exchange(self, r): #, Temperatura
		#cooling=0.5 #factor en la cual descrece la temperatura 
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		maxUtilidad = r.FO
		ruta = rOriginal.nodos
		for p1 in ruta:
			if (p1 in nodos_pickup[self.k]) == True: #Primer Pickup
				for l in range(ruta.index(p1)+1,len(ruta)-1): #Segundo Pickup for p2 entre p1 y el fin #range(ruta.index(p1)+1,len(ruta)-1)
					p2 = ruta[l]
					if (p2 in nodos_pickup[self.k]) == True:
						#Guardar Posiciones
						pos_p1 = rCopia.nodos.index(p1)
						pos_d1 = rCopia.nodos.index(p1 +(num_nodos/2))
						pos_p2 = rCopia.nodos.index(p2)
						pos_d2 = rCopia.nodos.index(p2 +(num_nodos/2))
						#Intercambiar
						rCopia.nodos.remove(p1)
						rCopia.nodos.insert(pos_p2, p1)
						rCopia.nodos.remove(p1+(num_nodos/2))
						rCopia.nodos.insert(pos_d2, p1+(num_nodos/2))
						rCopia.nodos.remove(p2)
						rCopia.nodos.insert(pos_p1, p2)
						rCopia.nodos.remove(p2+(num_nodos/2))
						rCopia.nodos.insert(pos_d1, p2+(num_nodos/2))
						rCopia = self.AsignarTiempos(rCopia)
						if rCopia.fact == 1:
							rCopia = self.AsignarCargas(rCopia)
							U = self.FuncionObjetivo(rCopia)

							if maxUtilidad < U:
								maxUtilidad = U
								r = self.Copiar(rCopia)
								r.FO = maxUtilidad
							#Codigo nuevo para aceptar soluciones peores
							#else:
								#Temperatura=Temperatura*cooling
								##print "Temperatura", Temperatura, "U actual", U, "maxutilidad", maxUtilidad
								#probabilidad=math.exp((U-maxUtilidad)/Temperatura)
								#aleatorio_1=random.random()	
								#if probabilidad>=aleatorio_1: #se acepta una solucion peor
							#	r = self.Copiar(rCopia)
							#	r.FO = U
						rCopia = self.Copiar(rOriginal)
		return r #(r, Temperatura)

	def Intra_route_request_relocate(self,r,p1):
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		maxUtilidad = r.FO

		pos_d = 2
		while pos_d < len(rOriginal.nodos) - 1: #mientras la posicion del delivery sea menor a la del ultimo nodo
			rCopia = self.Copiar(rOriginal) #Volvemos a la ruta con sus posiciones iniciales
			pos_p = 1
			rCopia.nodos.remove(p1) #Remuevo el pickup de la ruta
			rCopia.nodos.insert(pos_p,p1) #Inserto el pickup en la primera posicion

			rCopia.nodos.remove(p1+(num_nodos/2)) #Remuevo el delivery de la ruta
			rCopia.nodos.insert(pos_d,p1+(num_nodos/2)) #Inserto el delivery en su nueva posicion

			while rCopia.nodos[pos_p+1] != p1 + num_nodos/2: #Si en la proxima posicion esta el delivery, para!
				pos_p += 1 #Nos movemos a la nueva posicion
				rCopia.nodos.remove(p1)
				rCopia.nodos.insert(pos_p,p1)

				rCopia = self.AsignarTiempos(rCopia)
				if rCopia.fact == 1:
					rCopia = self.AsignarCargas(rCopia)
					U = self.FuncionObjetivo(rCopia)
					if maxUtilidad <= U:
						maxUtilidad = U
						r = self.Copiar(rCopia)
			pos_d += 1

		r.FO = maxUtilidad
		return r

	def Intra_route_multiple_request_exchange(self, r): #p1 es la primera aparicion, p2 es la segunda
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		maxUtilidad = r.FO
		ruta = rOriginal.nodos
		for p1 in ruta:
			if p1 <= num_nodos/2 and p1 != 0: #Primer Pickup
				pos_p1 = ruta.index(p1)
				#Paso 0: Creando H[1]
				ruta_p1 = self.Crea_Ruta_Truncada(rOriginal,p1)
				for l in range(ruta.index(p1)+1,len(ruta)-1): #Segundo Pickup
					p2 = ruta[l]
					pos_p2 = ruta.index(p2)
					if p2 <= num_nodos/2:
						#Paso 0: H[2]
						ruta_p2 = self.Crea_Ruta_Truncada(rOriginal,p2)
						if ruta_p1 != {} and ruta_p2 != {}:
							#Paso 1: Copiar ruta_p1 en posicion de ruta_p2
							j = pos_p2
							for i in ruta_p1:
								rCopia.nodos.insert(j,i)
								j=j+1
							#Paso 2: Remover elementos de ruta_p1 desde pos_p1 hasta finalizar ruta_p1 (eliminar original y dejar copia)
							j = pos_p1
							i = 0
							while j < pos_p1+len(ruta_p1):
								rCopia.nodos.remove(ruta_p1[i]) #Remueve la primera aparcion en la ruta.
								i = i+1
								j = j+1 
							#Paso 3: Eliminar ruta_p2
							j=pos_p2 
							i=0
							while j < pos_p2 + len(ruta_p2):
								rCopia.nodos.remove(ruta_p2[i])
								i=i+1
								j=j+1
							#Paso 4: Insertamos ruta_p2 en pos_p1
							j = pos_p1
							for i in ruta_p2:
								rCopia.nodos.insert(j,i)
								j=j+1
						rCopia = self.AsignarTiempos(rCopia)
						if rCopia.fact == 1:
							rCopia = self.AsignarCargas(rCopia)
							U = self.FuncionObjetivo(rCopia)
							if maxUtilidad < U:
								maxUtilidad = U
								r = self.Copiar(rCopia)
						rCopia = self.Copiar(rOriginal)
		r.FO = maxUtilidad
		return r

	def Intra_route_multiple_request_relocate(self, r,p1):
		#Finds a path H(i), in a route and tries to find a better position for this path in the same route
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r) #NUEVO
		pos_p1 = r.nodos.index(p1)
		ruta_p1 = self.Crea_Ruta_Truncada(rCopia,p1)
		maxUtilidad = r.FO
		if ruta_p1 != {}:
			#Paso 1: Remover ruta_p1 de ruta
			j = pos_p1
			i = 0
			while j < pos_p1 + len(ruta_p1):
				rCopia.nodos.remove(ruta_p1[i]) #Remueve la primera aparicion en la ruta.
				i = i+1
				j = j+1 
			#Paso 2: Insertar en nueva posicion
			for posInicial in range(1,len(rCopia.nodos)-1): # Habia: range(1,len(rOriginal.nodos)-1)
				j = posInicial
				for i in ruta_p1:
					rCopia.nodos.insert(j,i)
					j=j+1
				rCopia = self.AsignarTiempos(rCopia)
				if rCopia.fact == 1:
					rCopia = self.AsignarCargas(rCopia)
					#if self.CheckFactibilidadCargas(rCopia) == "Factible":
					U = self.FuncionObjetivo(rCopia)
					if maxUtilidad < U:
						maxUtilidad = U
				#		r = self.Copiar(rCopia)
				rCopia = self.Copiar(rOriginal)
				for l in ruta_p1:
					rCopia.nodos.remove(l)
		r.FO = maxUtilidad
		return r

	def Drop_and_Add(self, r): #Insercion de pedidos
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		pendientes = []
		for p in nodos_pickup[self.k]: 
			if (p in r.nodos) == False:
				pendientes.append([tarifa[p]-self.dual_pi[p-1],p])
		pendientes.sort(reverse = True)
		maxUtilidad = self.FuncionObjetivo(r)
		for i in pendientes:
			p = i[1] #Elemento de la dupla que indica el nodo pickup
			# Insertamos en su mejor posicion:
			pos_d_new = 1 #posicion minima original
			while pos_d_new < len(rOriginal.nodos)-1: #Itero en las posiciones de delivery
				rCopia.nodos.insert(pos_d_new,p+num_nodos/2)
				pos_p_new = 1 # pos_p_new = pos_d_new -1 vamos moviendonos hacia la izquierda.
				while (pos_p_new <= pos_d_new):
					rCopia2 = self.Copiar(rCopia)
					rCopia2.nodos.insert(pos_p_new,p)
					rCopia2 = self.AsignarTiempos(rCopia2) #ARREGRARLO agregar una posicion
					if rCopia2.fact == 1:
						rCopia2 = self.AsignarCargas(rCopia2)
						U = self.FuncionObjetivo(rCopia2)
						if maxUtilidad < U:
							maxUtilidad = U
							r=self.Copiar(rCopia2)
					pos_p_new = pos_p_new + 1	
				rCopia= self.Copiar(rOriginal)
				pos_d_new = pos_d_new + 1
		r.FO = maxUtilidad
		return r

	def Swap(self, r):
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		pendientes = []
		#Guardamos todos los pedidos sin asignar
		for p in nodos_pickup[self.k]: 
			if (p in r.nodos) == False:
				pendientes.append([tarifa[p]-self.dual_pi[p-1],p])
		pendientes.sort(reverse = True)
		maxUtilidad = r.FO
		for i in pendientes:
			s = i[1]
			for p1 in rOriginal.nodos:
				if (p1 in nodos_pickup[self.k]) == True:
					pos_p = rOriginal.nodos.index(p1)
					pos_d = rOriginal.nodos.index(p1 + num_nodos/2)
					#Intercambiar
					rCopia.nodos.remove(p1)
					rCopia.nodos.insert(pos_p,s)
					rCopia.nodos.remove(p1+num_nodos/2)
					rCopia.nodos.insert(pos_d,s+num_nodos/2)
					rCopia = self.AsignarTiempos(rCopia)
					if rCopia.fact == 1:
						rCopia = self.AsignarCargas(rCopia)
						U = self.FuncionObjetivo(rCopia)
						if maxUtilidad < U:
							maxUtilidad = U
							r = self.Copiar(rCopia)
					rCopia = self.Copiar(rOriginal)
		r.FO = maxUtilidad
		return r
				
	def Delete(self,r):
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		maxUtilidad = r.CostoRuta
		mejoro = 1
		while mejoro == 1: #Probamos con la mejor ruta encontrada hasta el momento
			mejoro = 0
			for i in rOriginal.nodos: #Probamos eliminando diferentes pedidos de la misma ruta 
				if (i in nodos_pickup[self.k]) == True:
					rCopia = self.EliminarPedido(i,rCopia)
					rCopia = self.AsignarTiempos(rCopia)
					if rCopia.fact == 1:
						rCopia = self.AsignarCargas(rCopia)
						U = self.Utilidad(rCopia)
						if maxUtilidad < U:
							maxUtilidad = U
							r.CostoRuta = maxUtilidad
							r = self.Copiar(rCopia)
							mejoro = 1
					rCopia = self.Copiar(rOriginal)
			
			rOriginal = self.Copiar(r)

		return r

	#Metodos de apoyo
	def AsignarTiempos(self,r): #agregar parametro de la posicion del pedido que se modifico.
		ruta = r.nodos
		#print "nodos tiempos :", r.nodos
		#print "ruta :", ruta
		r.B = []
		for j in range(0,len(r.nodos)):
			r.B.append(earliest[r.nodos[j]])
		i = 0
		while (i < len(ruta) - 1):
			delta = 0 # Se consideran aumentos de delta=0.25 (15 minutos)
			fin = 0
			try:
				self.tviaje[ruta[i],ruta[i+1]] #self.tviaje[ruta[i],ruta[i+1],self.k]
			except KeyError:
				#print "infactible"
				r.fact = 0
				return r
				break
			#print "hola :", ruta[i]
			#print "r.B["+str(i) +"] :", r.B[i], "lastest["+str(ruta[i])+"] :", lastest[ruta[i]], "duracion["+str(ruta[i])+"] :", duracion[ruta[i]], "tviaje["+str([ruta[i],ruta[i+1]])+"] :", self.tviaje[ruta[i],ruta[i+1]], "lastest["+str(ruta[i+1])+"] :",  lastest[ruta[i+1]]
			if r.B[i] > lastest[ruta[i]] or r.B[i] + duracion[ruta[i]] + self.tviaje[ruta[i],ruta[i+1]] > lastest[ruta[i+1]] : #self.tviaje[ruta[i],ruta[i+1],self.k]
				r.fact = 0
				#print "i :", ruta[i], "r.B"+str([i]), r.B[i]
				return r
				break
			elif r.B[i] + duracion[ruta[i]] + self.tviaje[ruta[i],ruta[i+1]] >= earliest[ruta[i+1]] and r.B[i] + duracion[ruta[i]] + self.tviaje[ruta[i],ruta[i+1]]<= lastest[ruta[i+1]]:
				r.B[i+1] = r.B[i] + duracion[ruta[i]] + self.tviaje[ruta[i],ruta[i+1]]
				i = i+1
				r.fact = 1
			elif r.B[i] + duracion[ruta[i]]+self.tviaje[ruta[i],ruta[i+1]] < earliest[ruta[i+1]]:
				r.B[i+1] = earliest[ruta[i+1]]
				i=i+1
		return r
	
	def AsignarCargas(self, r):
		ruta = r.nodos
		r.q = range(len(ruta))
		r.Q = range(len(ruta))
		r.q[0] = 0
		r.Q[0] = 0
		r.q[len(ruta)-1] = 0
		r.Q[len(ruta)-1] = 0
		optimizar = 0
		for j in range(1,len(ruta)-1):
			if (ruta[j] in nodos_pickup[self.k]) == True:
				if peso[ruta[j]] <= capacidad[self.k] - r.Q[j-1]:
					r.q[j] = peso[ruta[j]] 
				else:
					optimizar = 1
					break
				pos_d = r.nodos.index(ruta[j]+num_nodos/2)
				r.q[pos_d] = - r.q[j]
			r.Q[j] = r.Q[j-1] + r.q[j]
		if optimizar == 1:
			r = self.AsignarCargasLP(r)
		return r

	def AsignarCargasLP(self, r):
		r.q = []
		r.Q = []
		m1= Model("OptimizaCargas")
		m1.params.OutputFlag = 0
		m1.params.Presolve = 0
		alpha = 0#0.001
		q={}
		Q={}
		P = []
		#print "nodos :", r.nodos, "self.k :", self.k
		for i in r.nodos:
			q[i]=m1.addVar(lb=-capacidad[self.k], vtype=GRB.CONTINUOUS, name="q"+str([i]))
			Q[i]=m1.addVar(vtype=GRB.CONTINUOUS, name="Q"+str([i]))
			if (i in nodos_pickup[self.k])==True:
				P.append(i)
		m1.update()
		m1.setObjective(quicksum((tarifa[p] -self.dual_pi[nodos_pickup[self.k].index(p)])*q[p] for p in P), GRB.MAXIMIZE)
	
		m1.addConstr(q[d_0[self.k]] == 0)
		m1.addConstr(q[d_f[self.k]] == 0)
		for p in P:
			m1.addConstr(q[p] <= peso[p])
			m1.addConstr(q[p]+q[p+num_nodos/2]==0)
			#m1.addConstr(q[p] >= 0)
			m1.addConstr(q[p] >= alpha*peso[p])
		m1.addConstr(Q[d_0[self.k]] == 0)
		m1.addConstr(Q[d_f[self.k]] == 0)
		for i in range(0,len(r.nodos)-1):
			m1.addConstr(Q[r.nodos[i]] + q[r.nodos[i+1]] <= Q[r.nodos[i+1]])
			m1.addConstr(Q[r.nodos[i]] <= capacidad[self.k])
		m1.optimize()
		for i in r.nodos:
		 	r.q.append(q[i].x)
		 	r.Q.append(Q[i].x)

		return r
	
	def GuardarCargas(self, r):
		r.Cargas = [] #Cargas guarda todos los pesos asignados a los nodospickup de la red completa
		#print "nodos :", r.nodos
		for i in range(1,num_nodos/2+1): #Inicializamos lista con todos sus valores 0 // num_nodos + 1 = d_f
			r.Cargas.append(0)
		for i in range(1,num_nodos/2+1): #Cargas es una lista ordenada de todos los posibles nodos de la red
			#print "nodos :",r.nodos
			if (i in r.nodos) == True: 
				pos_i = r.nodos.index(i) #Vemos la posicion que tiene el nodo pickup i dentro de la ruta y guardamos su carga
				#print "i :", i, " pos_i :", pos_i, "r.q[pos_i] :", r.q[pos_i]
				r.Cargas[i-1] = r.q[pos_i]
		return r
	
	def EliminarPedido(self,p,r):
		pos_p = r.nodos.index(p)
		pos_d = r.nodos.index(p+num_nodos/2)
		r.q[pos_p] = "Eliminar"
		r.q[pos_d] = "Eliminar"
		r.Q[pos_p] = "Eliminar"
		r.Q[pos_d] = "Eliminar"
		r.B[pos_p] = "Eliminar"
		r.B[pos_d] = "Eliminar"
		r.nodos.remove(p)
		r.nodos.remove(p+num_nodos/2)
		r.q.remove("Eliminar")
		r.q.remove("Eliminar")
		r.Q.remove("Eliminar")
		r.Q.remove("Eliminar")
		r.B.remove("Eliminar")
		r.B.remove("Eliminar")
		return r	

	def Crea_Ruta_Truncada(self,r,p):
		ruta = r.nodos
		salida = {}
		ruta_p = []
		j = ruta.index(p)
		d_j = ruta.index(p+num_nodos/2)
		if r.Q[j] == r.q[j] and r.Q[d_j]==0: #Esta ruta puede ser extraida de la ruta del vehiculo sin alterar el ruteo
			ruta_p.append(ruta[j])
			salida = ruta_p
		return salida

	def Actualizar_Probabilidades(self,h, fo_inicial, fo_nueva, tpo_rutina):
		epsilon = 0.00001
		peso_prob=0.7 #peso que se le da a la eficiencia nueva
		if tpo_rutina == 0:
			tpo_rutina = 0.00001
		if abs(fo_inicial) == 0 and abs(fo_nueva)>0:
			M_i = float(abs(fo_nueva-(fo_inicial+1)))/float(abs(fo_inicial+1))
		elif abs(fo_inicial) == 0 and abs(fo_nueva)==0:
			M_i = 0
		else:
			M_i = float(abs(fo_nueva-fo_inicial))/float(abs(fo_inicial))
		E_antigua=self.E[h] #Eficiencia antigua en la iteracion pasada	
		#self.E[h] = max(float(M_i)/float(tpo_rutina),epsilon) #sin tomar eficiencia hostorica
		E_nueva = max(float(M_i)/float(tpo_rutina),epsilon) #eficiencia de esta corrida
		self.E[h]=(1-peso_prob)*E_antigua+peso_prob*E_nueva #nueva eficiencia calculada como una ponderacion de nueva mas antigua.
		sum_E = sum(self.E)
		for i in range(0,7):
			if i!= h:
				self.P[i] = float(self.E[i])/float(sum_E)
			else:
				self.P[h] = float(self.E[h])/float(sum_E) # En casos donde no haya mejora en la funcion objetivo, entre mas chico sea el epsilon mas chica sera la probabilidad de elegir esta heuristica

	def FuncionObjetivo(self, r): #Funcion Objetivo del Problema Satelite/ r = Ruta()
		ruta = r.nodos
		k=self.k
		Ingresos = 0
		for i in range(1, len(ruta)-1):
			Ingresos = Ingresos + tarifa[ruta[i]]*r.q[i] #tarifa de nodos delivery es 0 
		CostosOps = 0
		for i in range(1, len(ruta)):
			CostosOps = CostosOps + self.costos[ruta[i-1],ruta[i]] #self.costos[ruta[i-1],ruta[i],self.k]
		CostosCanc = 0
		for nm in PB_vuelos[k]: #Vuelo: (nm[0], nm[1])
			Cancelado = 1
			for i in range(0,len(ruta)-1): #No estan el nodo d_0 y d_f dentro de listmacronodos
				#print "nodo", ruta[i], "vuelo", nm, "macronodo1", listmacronodos[nm[0]],"macronodo2", listmacronodos[nm[1]]
				#print ruta
				#print " "
				if ((ruta[i] in listmacronodos[nm[0]]) == True) and ((ruta[i+1] in listmacronodos[nm[1]]) == True): #encontramos un arco que es tramo de vuelo de planif base.
					#print "nodo", ruta[i], "macronodo", listmacronodos[nm[0]]
					#print ruta
					Cancelado = 0
					break
			if Cancelado == 1:
				CostosCanc = CostosCanc + costos_penalizacion #costos_penalizacion[nm]
				#print "Costos cancelacion", CostosCanc
		CostosDual = self.dual_gamma
		for i in range(0,len(ruta)-1):
			if (ruta[i] in nodos_pickup[self.k]) == True:
				pos_i = nodos_pickup[self.k].index(ruta[i])
				CostosDual = CostosDual + self.dual_pi[pos_i]*r.q[i]

		utilidad = Ingresos - CostosOps - CostosCanc - CostosDual

		return utilidad

	def Utilidad(self, r): #Funcion de utilidad (objetivo) del problema original (convencional)
		ruta = r.nodos
		#print "ruta utilidad", ruta
		k=self.k

		Ingresos = 0
		for i in range(1, len(ruta)-1):
			Ingresos = Ingresos + tarifa[ruta[i]]*r.q[i] #tarifa de nodos delivery es 0 
		r.Ingresos = Ingresos
		CostosOps = 0
		for i in range(1, len(ruta)):
			CostosOps = CostosOps + self.costos[ruta[i-1],ruta[i]] #self.costos[ruta[i-1],ruta[i],self.k]
		CostosCanc = 0
		#print "self.vuelos", self.vuelos
		for nm in PB_vuelos[k]: #Vuelo: (nm[0], nm[1])
			#print "nm :", nm[0], "nm1", nm[1] 
			Cancelado = 1
			for i in range(0,len(ruta)-1): #No estan el nodo d_0 y d_f dentro de listmacronodos
				if ((ruta[i] in listmacronodos[nm[0]]) == True) and ((ruta[i+1] in listmacronodos[nm[1]]) == True): #encontramos un arco que es tramo de vuelo de planif base.
					Cancelado = 0
					break
			if Cancelado == 1:
				CostosCanc = CostosCanc + costos_penalizacion#costos_penalizacion[nm]
				#print "penalizacion :",  CostosCanc
		r.Costos = CostosOps + CostosCanc
		utilidad = Ingresos - CostosOps - CostosCanc
		#print ruta
		#print "Ingresos :",Ingresos, " Costos Op :", CostosOps, " CostosCanc :", CostosCanc 
		return utilidad		

	def Copiar(self,r):
		rCopia = Ruta()
		rCopia.q = r.q[:]
		rCopia.Q = r.Q[:]
		rCopia.B = r.B[:]
		rCopia.nodos = r.nodos[:]
		rCopia.FO = r.FO

		rCopia.NumRuta = r.NumRuta
		rCopia.CostoRuta = r.CostoRuta
		rCopia.Cargas = r.Cargas
		rCopia.Ingresos =r.Ingresos
		rCopia.Costos = r.Costos
		#rCopia.pond = r.pond Sirve solo para el proceso de Mezcl
		return rCopia
	
	#Metodos para columnas iniciales
	def Drop_and_Add_v(self, r): #Insercion de pedidos
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		pendientes = []
		for p in nodos_pickup[self.k]: 
			if (p in r.nodos) == False:
				pendientes.append([tarifa[p]-self.dual_pi[p-1],p])
		pendientes.sort(reverse = True)
		maxUtilidad = self.Utilidad(r)
		for i in pendientes:
			p = i[1] #Elemento de la dupla que indica el nodo pickup
			# Insertamos en su mejor posicion:
			pos_d_new = 1 #posicion minima original
			while pos_d_new < len(rOriginal.nodos)-1: #Itero en las posiciones de delivery
				rCopia.nodos.insert(pos_d_new,p+num_nodos/2)
				pos_p_new = 1 # pos_p_new = pos_d_new -1 vamos moviendonos hacia la izquierda.
				while (pos_p_new <= pos_d_new):
					rCopia2 = self.Copiar(rCopia)
					rCopia2.nodos.insert(pos_p_new,p)
					rCopia2 = self.AsignarTiempos(rCopia2)
					if rCopia2.fact == 1:
						rCopia2 = self.AsignarCargas(rCopia2)
						U = self.Utilidad(rCopia2) #Esto es lo unico que cambia con respecto al Drop_and_Add de LS
						if maxUtilidad < U:
							maxUtilidad = U
							r=self.Copiar(rCopia2)
					pos_p_new = pos_p_new + 1	
				rCopia= self.Copiar(rOriginal)
				pos_d_new = pos_d_new + 1
		r.CostoRuta = maxUtilidad
		return r

	def Insercion_inicial (self, r, pendientes): #Insercion de pedidos
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		maxUtilidad = self.Utilidad(r)
		pendientes_aux=[]
		#print "pendientes", pendientes
		for i in pendientes:
			p = i #Elemento de la dupla que indica el nodo pickup
			# Insertamos en su mejor posicion:
			pos_d_new = 1 #posicion minima original
			while pos_d_new < len(rOriginal.nodos)-1: #Itero en las posiciones de delivery
				rCopia.nodos.insert(pos_d_new,p+num_nodos/2)
				pos_p_new = 1 # pos_p_new = pos_d_new -1 vamos moviendonos hacia la izquierda.
				while (pos_p_new <= pos_d_new):
					rCopia2 = self.Copiar(rCopia)
					rCopia2.nodos.insert(pos_p_new,p)
					rCopia2 = self.AsignarTiempos(rCopia2) #ARREGRARLO agregar una posicion
					if rCopia2.fact == 1:
						rCopia2 = self.AsignarCargas(rCopia2)
						U = self.Utilidad(rCopia2)
						if maxUtilidad < U:
							maxUtilidad = U
							pendientes_aux=i
							r=self.Copiar(rCopia2)
							#pendientes_aux.remove(i)
					pos_p_new = pos_p_new + 1	
				rCopia= self.Copiar(rOriginal)
				pos_d_new = pos_d_new + 1
		try:
			pendientes.remove(pendientes_aux)
		except:		
			pass		
		return (r, pendientes)	

	def Swap_v(self, r):
		rCopia = self.Copiar(r)
		rOriginal = self.Copiar(r)
		pendientes = []
		#Guardamos todos los pedidos sin asignar
		for p in nodos_pickup[self.k]: 
			if (p in r.nodos) == False:
				pendientes.append([tarifa[p]-self.dual_pi[p-1],p])
		pendientes.sort(reverse = True)
		maxUtilidad = self.Utilidad(r)
		for i in pendientes:
			s = i[1]
			for p1 in rOriginal.nodos:
				if (p1 in nodos_pickup[self.k]) == True:
					pos_p = rOriginal.nodos.index(p1)
					pos_d = rOriginal.nodos.index(p1 + num_nodos/2)
					#Intercambiar
					rCopia.nodos.remove(p1)
					rCopia.nodos.insert(pos_p,s)
					rCopia.nodos.remove(p1+num_nodos/2)
					rCopia.nodos.insert(pos_d,s+num_nodos/2)
					rCopia = self.AsignarTiempos(rCopia)
					if rCopia.fact == 1:
						rCopia = self.AsignarCargas(rCopia)
						U = self.Utilidad(rCopia)
						if maxUtilidad < U:
							maxUtilidad = U
							r = self.Copiar(rCopia)
					rCopia = self.Copiar(rOriginal)
		r.CostoRuta = maxUtilidad
		return r
	