# -*- coding: utf-8 -*-
 
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
from matplotlib.path import Path
# this function is to get UE's position and Enb's from UE and Enb position file,"ue1_position.txt" and "enb_topology.txt"
def getList(a,indicate):
        x = []; y =[] ; l = 0 ; time =[];
        while True:
                line = a.readline()
                line = line.split()
                if not line : break
                x.append(float(line[0]))
                y.append(float(line[1]))
		if indicate == True:
			time.append(float(line[3]))
		l = l+1
	if indicate== True:
        	return x,y,l,time
	else:
		return x,y,l
# get building topologies from building topology file "building_topology.txt"
def getListForBuilding(a):
        x_min = [];x_max =[];  y_min =[] ;y_max=[]; l = 0; 
        while True:
                line = a.readline()
                line = line.split()
                if not line : break
                x_min.append(float(line[0]))
                x_max.append(float(line[1]))
		y_min.append(float(line[2]))
                y_max.append(float(line[3]))
                l = l+1
        return x_min,x_max,y_min,y_max,l
#trace handover event from handover starting trace file, EnbHandoverStartStats.txt"
def getListForHandover(a):
      time = []; imsi =[];  rnti = []; s_cell_1 =[];  e_cell_1 =[] ;s_cell_2=[];e_cell_2=[] ;l = 0; m=0
      while True:
                line = a.readline()
                line = line.split()
                if not line : break
		time.append(float(line[0])) # time
                imsi.append(int(line[1]))   #imsi
		rnti.append(int(line[2]))   #rnti
		if int(line[4])>=6: # this is for special case that 28G eNB's cellIDs are 6,7,8,9 and 73G eNB's cellIDs are 2,3,4,5
                  s_cell_1.append(int(line[3]))  # starting cellID
                  e_cell_1.append(int(line[4]))  # end cellID
		  l = l+1;
 		else:
		  s_cell_2.append(int(line[3]))
		  e_cell_2.append(int(line[4]))
		  m = m+1;
           
      return time, imsi, rnti,  e_cell_1, e_cell_2, l, m
 
fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
line, = ax1.plot([],[],lw=2)

def init():
        line.set_data([],[])
        return line,
 ## animation function
def animate(i):
    graph_data = open('ue1_position.txt','r')
    enb = open('enb_topology.txt') 
    building = open('building_topology.txt')
    handoverState = open('EnbHandoverStartStats.txt')
    xs = []
    ys = []
    l =0
    x_min =[]; m_max =[]
    y_min =[]; y_max= []
    b =0
    xe=[]; ye=[]; v =0; simul_time=[]
    ## get UE information and eNB enformation from the given files
    (xs,ys,l,simul_time) = getList(graph_data,True)
    (xe,ye,v) = getList(enb, False)
    ## get building information 	
    (x_min,x_max,y_min, y_max,b) = getListForBuilding(building)
    rnti = []; c1 = [];c2= []; c3 = []; t =0; time =[]
    ## get Handover state; e_cell_1s  mean 28G eNBs , e_cell_2s  mean 73G eNBs
    
    ## t1-1, t2-1 mean indices of the last handover events for 28G and 73G eNBs 
    (time,imsi,rnti,e_cell_1,e_cell_2,t1,t2) = getListForHandover(handoverState)
    ## xs[l-1],ys[l-1] indicates the last postion of UE
    ax1.clear()
    ax1.plot(xs[l-1],ys[l-1],'kD',label ='UE')
    ## xe, ye is the set of eNBs location
    ax1.plot(xe,ye,'ro',ms=10,label = 'mmWave eNB')
    ## xe[v-1], ye[v-1] indicates the LTE eNB position
    ax1.plot(xe[v-1],ye[v-1],'bo',ms=10, label = 'LTE eNB')
    vertice, vertice1 = [],[]
    ## here e_cell_1[t1-1]-2 is the index of eNB in enb_postion file.
    ## the reason why index is obtained by substracting two is that the starting number of mmWave eNB cellID is 2
    ## we can get eNB index in enb_position file using cellID informatin by reducing it by 2
    vertice.append([xs[l-1],xe[e_cell_1[t1-1]-2]])
    vertice.append([ys[l-1],ye[e_cell_1[t1-1]-2]])
    
    vertice1.append([xs[l-1],xe[e_cell_2[t2-1]-2]])
    vertice1.append([ys[l-1],ye[e_cell_2[t2-1]-2]])
    ax1.legend(loc=10,bbox_to_anchor=(0.5,-0.05),fancybox=True, shadow=True,ncol=3)

    ## this is the line plot between UE and eNBs (28G, 73G)
    
    ax1.plot (vertice[0],vertice[1])
    ax1.plot (vertice1[0],vertice1[1])
    ax1.plot([xe[v-1],xs[l-1]], [ye[v-1],ys[l-1]])
    i =0
    ## this is to plot obstacles from file, "building_topology.txt"
    while i< b:
    	rect = plt.Rectangle((x_min[i],y_min[i]),(x_max[i]-x_min[i]), (y_max[i]-y_min[i]), facecolor = "black")
    	i = i+1
    	plt.gca().add_patch(rect)
    ## indicate "obstacle"
    plt.annotate('blockage',xy =(x_max[i-1],y_max[i-1]),xytext = (x_max[i-1]+7, y_max[i-1]+3),arrowprops = dict(facecolor = 'black', shrink = 0.05,width =2))
    ## show simulation time 
    plt.annotate(simul_time[l-1],xy=(50,-0.5))    

ani = animation.FuncAnimation(fig, animate,init_func = init,frames =200, interval=100)

plt.show()


