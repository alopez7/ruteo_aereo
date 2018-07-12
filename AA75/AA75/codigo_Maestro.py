#from Graficar import*
from gurobipy import*
from Datos_REC import *
from codigo_ANS import*


class Maestro():

	''' Inicializa la clase '''
	def __init__(self):
		''' ¿? '''
		self.SPs = []
		''' Iteracion '''
		self.It = 0
		''' Utilidad final '''
		self.U_final = 0
		''' Utilidad relajada '''
		self.U_relajada = 0
		''' Guarda la ruta optima para esta iteracion para cada avion '''
		self.rutas_finales = [] #Almacena las rutas finales de cada avion
		for k in aviones:
			self.rutas_finales.append(0)

	''' Resuelve el modelo de optimizacion de las rutas '''
	def OptimizarMaestro(self):

		''' Creo el modelo con sus respectivos parametros '''
		m=Model('M')
		m.params.FeasibilityTol = 1e-3
		m.params.MIPFocus = 2
		m.params.Presolve = 0
		x = {}
		c = {}
		q = {}

		''' Para cada avion '''
		for k in aviones:

			''' Para cada ruta del avion '''
			for r in range(0,len(self.SPs[k-1].rutas)):

				''' Creo la variable x_ruta_avion de tipo Real '''
				x[r+1,k] = m.addVar(vtype=GRB.CONTINUOUS, name="x"+str([r+1,k]))

				''' Guardo los costos de las rutas '''
				c[r+1,k] = self.SPs[k-1].rutas[r].CostoRuta

				''' Guardo las cargas de los pickups '''
				for i in range(0,len(self.SPs[k-1].rutas[r].Cargas)): #Cargas son nodos_pickup
					q[i+1,r+1,k] = self.SPs[k-1].rutas[r].Cargas[i]

		''' Creo el modelo '''
		m.update()

		''' FO:  '''
		m.setObjective(quicksum(quicksum(c[r+1,k]*x[r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) for k in aviones), GRB.MAXIMIZE)

		''' R1: '''
		#Restriccion 1 Vinculante Pickup
		for i in range(1,(num_nodos/2)+1):
			m.addConstr(quicksum(quicksum(x[r+1,k]*q[i,r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) for k in aviones) <= peso[i], "Peso"+str([i]))


		''' R2: '''
		#Restriccion 2 Convexidad
		for k in aviones:
			m.addConstr(quicksum(x[r+1,k] for r in range(0,len(self.SPs[k-1].rutas))) == 1, "Convexidad"+str([k]))

		''' Optimizo '''
		m.update()
		m.optimize()

		''' Escribo el resultado '''
		m.write("Modelo_MIP2.rlp")

		''' Variables donde guardare los resultados '''
		salida2=[]
		dual_pi = []
		dual_gamma = []
		cons = m.getConstrs()

		''' Obtengo dual de la restriccion 1 (dual_pi) '''
		#Dual Restriccion 1
		for i in range(0, num_nodos/2):
			dual_pi.append(cons[i].getAttr("Pi"))

		''' Obtengo dual de la restriccion 2 (dual_gamma) '''
		#Dual Restriccion 2
		for k in range(num_nodos/2,len(cons)):
			dual_gamma.append(cons[k].getAttr("Pi"))
		duales = []
		duales.append(dual_pi)
		duales.append(dual_gamma)


		''' Si me toca resolverlo en version entera, lo hago '''
		if self.It == "Entero":

			''' Digo que las variables son enteras ahora '''
			for k in aviones:
				for r in range(0,len(self.SPs[k-1].rutas)):
					x[r+1,k].vtype = GRB.INTEGER

			''' Optimizo '''
			m.update()
			m.optimize()
			m.write("Modelo_IP.rlp")

			''' Calculo la utilidad total '''
			uTotal = 0
			for k in aviones:
				for r in range(0,len(self.SPs[k-1].rutas)):
					if x[r+1,k].x>0.8:
						salida2.append([x[r+1,k].VarName, "/ Utilidad", round(c[r+1,k],2),"/ Nodos",self.SPs[k-1].rutas[r].nodos, "/ q",self.SPs[k-1].rutas[r].q])
						uTotal+= round(c[r+1,k],2)
			uTotal = "Total Utilidad: "+str(uTotal)
			self.U_final = uTotal

			salida2.append(uTotal)
			archivo = "Resultados Maestro IP.txt"
			''' escribo el output '''
			self.write_table(archivo,salida2)
			self.U_relajada = m.objVal

		''' Si no me toca, retorno lo duales '''
		else:
			return duales

	#Guarda el objeto r de la clase Ruta en su posicion dentro del arreglo de rutas finales
	def Guardar_rutas_finales(self,r,k):
		self.rutas_finales[k-1] = r

	#Funcion para escribir archivos
	def write_table(self, file_route, table):
		f = open(str(file_route),"w")
		for item in table:
			f.write(str(item)+"\n")
		f.close()
