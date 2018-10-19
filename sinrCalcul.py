def classify(a):
	i = 0
	new_list = []
	k = 0; s = False
	while k < len(a)-1:
		for u in new_list:
			if u == a[k]: 
				s = True;		
		if s == False : 
			new_list.append(a[k])
		else:
			s = False;
		k = k+1
	return new_list

if __name__ == "__main__":
	sinr = open ("MmWaveSinrTime.txt")
	cellId =[]
	imsi = []
	while True:
        	line = sinr.readline()
        	line = line.split()
	        if not line: break
		imsi.append(line[1])
		cellId.append(line[2])

	fileNameDic = {}
	for imsi_name in classify(imsi):
		for cellId_name in classify(cellId):	
			fileName = "SINR-UE-"+imsi_name+"-"+"Enb-"+cellId_name+".txt"
			imsi_CellId = imsi_name+cellId_name
			fileNameDic[imsi_CellId] = open(fileName,'w')
	sinr = open ("MmWaveSinrTime.txt")
	while True:
              	line = sinr.readline()
                line = line.split()
                if not line: break
		imsi_cellId = line[1]+line[2]
		fileNameDic[imsi_cellId].write(line[0] + "\t"+line[3]+"\n")

        for imsi_name in classify(imsi):
                for cellId_name in classify(cellId):
			imsi_CellId = imsi_name+cellId_name
                        fileNameDic[imsi_CellId].close()
