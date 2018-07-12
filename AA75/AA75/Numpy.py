import numpy as np
import time


start1=time.time()
c3=np.ones((1,7))
c3*=0.142857145
#c3=np.array([1:0.142857145:7])
print "c3 >",c3
end1=time.time()
elapsed1=end1-start1
print elapsed1

start=time.time()
P=[]
for i in range(0,7):
	P.append(0.142857145)
print "P >", P	
end=time.time()
elapsed=end-start
print elapsed
print elapsed1/elapsed

