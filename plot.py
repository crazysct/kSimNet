import matplotlib.pyplot as plt
#import pandas as pd
import matplotlib.animation as animation
from matplotlib import style

def getList(a):
	x = []; y =[] 
	while True:
		line = a.readline()
		line = line.split()
		if not line : break
		x.append(float(line[0]))
		y.append(float(line[1]))
	return x,y

fig = plt.figure(figsize = (5,5))
ax4 = fig.add_subplot(1,1,1)
#ax2 = fig.add_subplot(2,2,2)
def animate(i):
	
	a4_1 = open ( "rlc_Tput_senb2_ue1_bearer_1.txt", 'r')
	a4_2 = open ( "rlc_Tput_senb1_ue1_bearer_1.txt",'r')
	a4_3 = open ("udp_throughput_ue1.txt",'r')
	
	x = []; y = []
	x1 = []; y1= []
	x2 = []; y2 =[]
	x2_1 = [] ; y2_1 = []
	x3_1 = [] ; y3_1 = []
	x3_2 = [] ; y3_2 = []
	x3_3 = [] ; y3_3 = []
	

	
	(x3_1,y3_1) = getList(a4_1)
	(x3_2,y3_2) = getList(a4_2)
	(x3_3,y3_3) = getList(a4_3)

	
	
	ax4.clear()
	ax4.plot(x3_1,y3_1,label = "mmWave Throughput1" )
	ax4.plot(x3_2, y3_2, label = "mmWave Throughput2")
	ax4.plot(x3_3, y3_3, label = "UDP Thoughput")
	ax4.legend(loc=1)
	ax4.grid()
	ax4.set_title("Total and Path Throughput")
	ax4.set_xlabel("time(s)")
	ax4.set_ylabel("Throughput(Mbps)")



ani = animation.FuncAnimation(fig, animate, interval=1000)

plt.show()

