import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
line, = ax1.plot([],[],lw=2)
def init():
        line.set_data([],[])
        return line,

#trace handover event from handover starting trace file, "
def getListForHandover(a):
      time = []; imsi =[];  rnti = []; s_cell_1 =[];  e_cell_1 =[] ;s_cell_2=[];e_cell_2=[] ;l = 0; m=0
      while True:
                line = a.readline()
                line = line.split()
                if not line : break
                time.append(float(line[0])) # time
                imsi.append(int(line[1]))   #imsi
                rnti.append(int(line[2]))   #rnti
                if int(line[4])>=6:
                  s_cell_1.append(int(line[3]))  # starting cellID
                  e_cell_1.append(int(line[4]))  # end cellID
                  l = l+1;
                else:
                  s_cell_2.append(int(line[3]))
                  e_cell_2.append(int(line[4]))
                  m = m+1;

      return time, imsi, rnti,  e_cell_1, e_cell_2, l, m
      
def animate(i):
	ue1 = np.loadtxt('ue1_position.txt')
	enb = np.loadtxt('enb_topology.txt') 
	building = np.loadtxt('building_topology.txt')
	handoverState = open('EnbHandoverStartStats.txt','r')
	
	ax1.clear()
	final_position = len (ue1)-1
	ue1_x = ue1[final_position][0].tolist()
	ue1_y = ue1[final_position][1].tolist()
	ax1.plot(ue1_x,ue1_y, label='UE')
	
	lte_position = len (enb)-1
	lte_x = enb[lte_position][0]
	lte_y = enb[lte_position][1]
	ax1.plot(enb[:,0], enb[:,1],'ro', ms=10, label = 'mmWave eNB')
	ax1.plot(lte_x, lte_y , 'bo',ms=10, label='LTE eNB')
	
	rnti = []; c1 = [];c2= []; c3 = []; t =0; time =[]
	(time,imsi,rnti,e_cell_1,e_cell_2,t1,t2) = getListForHandover(handoverState)
	
	vertice, vertice1 = [],[]
	vertice.append([ue1_x,enb[e_cell_1[t1-1]-2]][0])
 	vertice.append([ue1_y,enb[e_cell_1[t1-1]-2]][1])
 	vertice1.append([ue1_x,enb[e_cell_2[t2-1]-2]][0])
 	vertice1.append([ue1_y,enb[e_cell_2[t2-1]-2]][1])
 	
 	ax1.legend(loc=10,bbox_to_anchor=(0.5,-0.05),fancybox=True, shadow=True,ncol=3)
	
	ax1.plot (vertice[0],vertice[1])
	ax1.plot (vertice1[0],vertice1[1])
	ax1.plot([lte_x,ue1_x],[lte_y,ue1_y])
	i =0
	while i< len(building):
		rect = plt.Rectangle((building[i][0],building[i][2]),(building[i][1]-building[i][0]), (building[i][3]-building[i][2]), facecolor = "black")
		i = i+1
		plt.gca().add_patch(rect)
	plt.annotate('blockage',xy =(building[i-1][1],building[i-1][3]),xytext = (building[i-1][1]+7, building[i-1][3]+3),
	arrowprops = dict(facecolor = 'black', shrink = 0.05,width =2))
	
	plt.annotate(ue1[final_position][2],xy=(50,-0.5))
	
ani = animation.FuncAnimation(fig, animate,init_func = init,frames =200, interval=100)
plt.show()
	
	
	
	

