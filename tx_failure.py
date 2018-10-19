f = open ("fileDelay.txt")
c =0
k = 0
s = 0
while True:
	line = f.readline()
	line = line.split()
	#print(line[1])
	k = k+1
	if not line: break
	if float(line[1]) ==200:
		c = c+1
		s = s + (1000-int(line[4]))
		#print (line[3])
print (c)
print (float(c)/k)
print (s)	
