#from Graficar import*
from gurobipy import*
from Datos_REC import *
from codigo_ANS import*


class Maestro():
	def __init__(self):
		self.SPs = []
		self.It = 0
		self.U_final = 0
		self.U_relajada = 0
		self.rutas_finales = [] #Almacena las rutas finales de cada avion
		for k in aviones:
			self.rutas_finales.append(0)	

	def OptimizarMaestro(self):
		m=Model('M')
		m.params.FeasibilityTol = 1e-3
		m.params.MIPFocus = 2
		m.params.Presolve = 0
		x = {}
		c = {}
		q = {}
		for k in aviones:
			for r in range(0,len(self.SPs[k-1].rutas)):
				x[r+1,k]=m.addVar(vtype=GRB.CONTINUOUS, name="x"+str([r+1,k])) 
				c[r+1,k]=self.SPs[k-1].rutas[r].CostoRuta
				#print "c"+str([r+1,k]), c[r+1,k]
				#print "costo reducido :", self.SPs[k-1].CostoReducido
				#print "ruta", self.SPs[k-1].rutas[r].nodos

				for i in range(0,len(self.SPs[k-1].rutas[r].Cargas)): #Cargas son nodos_pickup
					q[i+1,r+1,k] = self.SPs[k-1].rutas[r].Cargas[i]
					#print "q"+str([i+1,r+1,k]), q[i+1,r+1,k]
		m.update()
		#Funcion Objetivo
		m.setObjective(quicksum(quicksum(c[r+1,k]*x[r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) for k in aviones), GRB.MAXIMIZE) 

		#Restriccion 1 Vinculante Pickup
		for i in range(1,(num_nodos/2)+1): 
			m.addConstr(quicksum(quicksum(x[r+1,k]*q[i,r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) for k in aviones) <= peso[i], "Peso"+str([i]))
		

		#Restriccion 2 Convexidad
		for k in aviones:
			m.addConstr(quicksum(x[r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) == 1, "Convexidad"+str([k]))

		m.update()

		m.optimize()

		m.write("Modelo_MIP2.rlp")
		
		salida = []
		salida2=[]
		dual_pi = []
		dual_gamma = []
		cons = m.getConstrs()

		#Dual Restriccion 1
		for i in range(0, num_nodos/2): #1 menos que en la restriccion original, porque parte a enumerar desde 0
			dual_pi.append(cons[i].getAttr("Pi"))
		#Dual Gamma corresponde al resto de las restricciones de convexidad
		for k in range(num_nodos/2,len(cons)):
			#Dual Restriccion 2
			dual_gamma.append(cons[k].getAttr("Pi"))

		duales = []
		duales.append(dual_pi)
		duales.append(dual_gamma)
		##########################################################################
		'''
		for k in aviones:
			uTotal = 0
			for j in range(0,len(self.SPs[k-1].rutas)):
				uTotal = uTotal + c[j+1,k]*x[j+1,k].x
				salida.append([x[j+1,k].VarName, '=', x[j+1,k].x, "/    Utilidad Ruta : ", c[j+1,k]])
			self.SPs[k-1].UTotal = uTotal
		#salida.append(["Dual_Pi: ", dual_pi]) #salida.append(["Dual_Gamma: ", dual_gamma])

		archivo = "Resultados Maestro Relajado.txt"
		self.write_table(archivo,salida)
		'''
		##########################################################################
		print "#########################################################"

		if self.It == "Entero": #Al final de la generacion de columnas ejecutamos el Maestro como un IP
			for k in aviones:
				for r in range(0,len(self.SPs[k-1].rutas)):
					x[r+1,k].vtype = GRB.INTEGER

		#			#Restriccion 1 Vinculante Pickup
			#c0={}
			#for j in range(1,(num_nodos/2)+1): 
			#	c0[j]=m.getConstrByName('Peso '+str([j]))
			#	c0[j].setAttr("Sense",'=')

			#for i in range(1,(num_nodos/2)+1): 
			#	m.addConstr(quicksum(quicksum(x[r+1,k]*q[i,r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) for k in aviones) >= peso[i])		

			m.update()
			m.optimize()
			m.write("Modelo_IP.rlp")
			uTotal = 0
			for k in aviones:
				for r in range(0,len(self.SPs[k-1].rutas)):
					if x[r+1,k].x>0.8: #'=', x[r+1,k].x Esto da lo mismo porque todos tienen valor 1.0
						salida2.append([x[r+1,k].VarName, "/ Utilidad", round(c[r+1,k],2),"/ Nodos",self.SPs[k-1].rutas[r].nodos, "/ q",self.SPs[k-1].rutas[r].q])
						uTotal+= round(c[r+1,k],2)
			uTotal = "Total Utilidad: "+str(uTotal)
			self.U_final = uTotal
			salida2.append(uTotal)
			archivo = "Resultados Maestro IP.txt"
			self.write_table(archivo,salida2)
			self.U_relajada = m.objVal
		else:	
			return duales

	def Guardar_rutas_finales(self,r,k):
		self.rutas_finales[k-1] = r #Guarda el objeto r de la clase Ruta en su posicion dentro del arreglo de rutas finales

	#Funcion para escribir archivos
	def write_table(self, file_route, table):
		f = open(str(file_route),"w")
		for item in table:
			f.write(str(item)+"\n")
		f.close()
	#Fin funcion para escribir archivos