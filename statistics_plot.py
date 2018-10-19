import numpy as np
import matplotlib as mpl
#import matplotlib.pylab as plt
from matplotlib import pyplot as plt
data_s = np.loadtxt("statistc_rlcAm_NYU.txt")
#data_m = np.loadtxt("statistc_naive.txt")
data_m_2 =np.loadtxt("statistc_rlcAm.txt")

plt.figure(1)
#plt.plot(data_m[:,0],data_m[:,2],'rs--',label='Dual Handover(Naive)')
plt.plot(data_m_2[:,0], data_m_2[:,2],'ks--', label="Dual Handover(New)")
plt.plot(data_s[:,0],data_s[:,2],'bs--',label='Single Handover')
plt.legend (loc=2)
plt.title("Message Loss Rate")
plt.xlabel("file size(Mbye)")
plt.ylabel("loss rate")
plt.grid(True)

plt.figure(2)
plt.title ("Total Resource Usage")
barwidth = 0.2
plt.bar(data_s[:,0], data_s[:,3],barwidth, color = 'b',align= 'center', label = "Single Handover")
#plt.bar(data_m[:,0]+barwidth,data_m[:,3],barwidth , color = 'r',align='center',  label="Dual Handover(Naive)")
plt.bar(data_m_2[:,0]+barwidth+barwidth,data_m_2[:,3],barwidth , color = 'k',align='center',  label="Dual Handover(New)")
#plt.xticks(data[:,0]+0.05)
plt.legend (loc=2)
plt.xlabel("file size(Mbyte)")
plt.ylabel("total resource usage amount(byte)")

plt.show()
