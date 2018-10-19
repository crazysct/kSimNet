#tb = open ("RxPacketTrace.txt")
tb = open ("UE-1-TbSize.txt")
l = 0;
tb_size_DL = 0 ;
while True:
	line = tb.readline()
	#print (line)
	line = line.split()
	if not line: break
	tb_size_DL=tb_size_DL + int (line[2])
	#if not l == 0:
		#print (line)
		#if line[0] == 'DL':
			#tb_size_DL = tb_size_DL + int(line[7])
	l = l+1
print (tb_size_DL)
