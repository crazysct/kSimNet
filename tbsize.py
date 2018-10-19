
#tb = open ("RxPacketTrace.txt")

def tbSize(tb):
	l = 0;
	tb_size_DL = 0 ;
	while True:
        	line = tb.readline()
        	#print (line)
        	line = line.split()
        	if not line: break
        	tb_size_DL=tb_size_DL + int (line[2])
	return str(tb_size_DL)


def TxFailure(f):
	c =0; k =0
	while True:
        	line = f.readline()
	        line = line.split()
        	k = k+1
	        if not line: break
        	if float(line[1]) ==200:
                	c = c+1
	return str(c), str(float(c)/k)
	

file = open ("statistc_rlcAm.txt",'w')
tb_500 = open ("RLC_AM_CASE/500_packets/UE-1-TbSize.txt")
tb_1000 = open ("RLC_AM_CASE/1000_packets/UE-1-TbSize.txt")
tb_1500 = open ("RLC_AM_CASE/1500_packets/UE-1-TbSize.txt")
tb_3000 = open ( "RLC_AM_CASE/3000_packets/UE-1-TbSize.txt")
tb_5000 = open ( "RLC_AM_CASE/5000_packets/UE-1-TbSize.txt")
tb_7500 = open ("RLC_AM_CASE/7500_packets/UE-1-TbSize.txt")
tb_10000 = open ("RLC_AM_CASE/10000_packets/UE-1-TbSize.txt")

f_500 = open ("RLC_AM_CASE/500_packets/fileDelay.txt")
tx_f_500, ratio_500 = TxFailure(f_500)

f_1000 = open ("RLC_AM_CASE/1000_packets/fileDelay.txt")
tx_f_1000, ratio_1000 = TxFailure(f_1000)

f_1500 = open ("RLC_AM_CASE/1500_packets/fileDelay.txt")
tx_f_1500, ratio_1500 = TxFailure(f_1500)

f_3000 = open ("RLC_AM_CASE/3000_packets/fileDelay.txt")
tx_f_3000, ratio_3000 = TxFailure(f_3000)

f_5000 = open ("RLC_AM_CASE/5000_packets/fileDelay.txt")
tx_f_5000, ratio_5000 = TxFailure(f_5000)

f_7500 = open ("RLC_AM_CASE/7500_packets/fileDelay.txt")
tx_f_7500, ratio_7500 = TxFailure(f_7500)

f_10000 = open ("RLC_AM_CASE/10000_packets/fileDelay.txt")
tx_f_10000, ratio_10000 = TxFailure(f_10000)

numberOfPacket = [500,1000,1500,3000,5000,7500,10000]
Packets = []
for i in numberOfPacket:
	Packets.append(i*1400/(1024*1024.0))

file.write(str(Packets[0]) + "\t" + tx_f_500+"\t" + ratio_500+"\t" + tbSize(tb_500)+"\n")
file.write(str(Packets[1]) + "\t" + tx_f_1000+"\t" + ratio_1000+"\t" + tbSize(tb_1000)+ "\n")
file.write(str(Packets[2]) + "\t" + tx_f_1500+"\t" + ratio_1500+"\t" + tbSize(tb_1500)+ "\n")
file.write(str(Packets[3]) + "\t" + tx_f_3000+"\t" + ratio_3000+"\t" + tbSize(tb_3000)+ "\n")
file.write(str(Packets[4]) + "\t" + tx_f_5000+"\t" + ratio_5000+"\t" + tbSize(tb_5000)+ "\n")
file.write(str(Packets[5])+"\t" + tx_f_7500 + "\t"+ ratio_7500 +"\t" +  tbSize(tb_7500) + "\n")
file.write(str(Packets[6]) + "\t" + tx_f_10000+"\t" + ratio_10000+"\t" + tbSize(tb_10000)+ "\n")
file.close()
