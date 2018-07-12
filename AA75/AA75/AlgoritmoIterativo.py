import os
from codigo_Main import *

TotalInstancias = 4
e = 4
Inst = list()

Inst.append("/resultados_AA10_2")
Inst.append("/resultados_AA15_2")
Inst.append("/resultados_AA20_2")
Inst.append("/resultados_AA75_REC_P_10")

dirBaseInicio = os.getcwd()

while (e <= TotalInstancias): #Iteracion entre las diferentes instancias: AA10, AA15, AA20
	print "*****************************************"
	print "INICIO INSTANCIA ", Inst[e-1]
	print ""
	dir = dirBaseInicio + Inst[e-1]
	os.chdir(dir) #Entro al directorio

	#Criterios de parada:
	TpoEjecucion = 3600 #limite de tiempo de ejecucion en seg
	Num_Iteraciones_Max = 10 #limite de iteraciones
	Num_no_mejoras = 3 #limite de no mejoras en la funcion objetivo

	#Contadores:
	tpo = 0 #contador de tiempos #No desactivar este contador
	i = 1 #contador de iteraciones
	j = 0 #contador de no mejoras

	mejorFO = 0 #Activarlo si se considera el criterio: Num_no_mejoras

	Resumen = list()
	Resumen.append(["Corrida", "Tiempo (seg)", "Utilidad Relajada (US$)", "Utilidad Final (US$)"])
	dirBase = os.getcwd()

	while(i<=Num_Iteraciones_Max and j<Num_no_mejoras and tpo < TpoEjecucion):
	#while(j<Num_no_mejoras):
	#while(tpo < TpoEjecucion):
		print "-----------------------------------"
		print "INICIO CORRIDA ", i
		print ""
		##########################################################################

		dir = dirBase + "/Corrida_ "+str(i) #Directorio de Corrida
		os.mkdir(dir) #Creo directorio donde se guarda los resultados de la Corrida
		os.chdir(dir) #Entro al directorio creado

		##########################################################################
		if e == 1:
			m = Main_AA10()
		elif e == 2:
			m = Main_AA15()
		elif e == 3:
			m = Main_AA20()
		elif e == 4:
			m = Main_AA75()

		m.Iniciar() #Ejecutamos algoritmo de Generacion de Columnas Hibrido

		tpo = tpo + m.TpoCorrida
		Resumen.append([i, m.TpoCorrida, m.U_relajada, m.U_final]) #Guardamos tiempo de corrida actual FALTAAAAAAAAAAAAAAAAAA

		#Activar estas lineas si se considera el criterio de parada: Num_no_mejoras
		if mejorFO >= m.U_final:
			j += 1 #contador de no mejoras
		else:
			mejorFO = m.U_final
			j=0



		print "-----------------------------------"
		print "CORRIDA ", str(i), " FINALIZADA - TIEMPO: ", str(m.TpoCorrida/60), " min", " TIEMPO TRANSCURRIDO: ", str(tpo/60)
		print "-----------------------------------"

		i = i+1
		##########################################################################

		os.chdir(dirBase) #Vuelvo al directorio base

		##########################################################################
	print "ALGORITMO FINALIZADO EN ", str(tpo/60), " minutos"

	m.write_table("Resumen_corridas_FD.txt",Resumen) #Guarda en la misma carpeta los resultados de la instancia actual
	e = e+1
	os.chdir(dirBaseInicio)
