set datafile separator ","

while(1){
	set multiplot layout 3,1
	set tmargin 3

	set style line 1 lt 1 lw 5 lc rgb "black"
	set style line 2 lt 2 lw 5 lc rgb "red"

#	set title "< RTT >"
#	plot "Virt5gc-rtt.data" using 1:3 w lp t 'rtt' #"Virt5gc-rtt.data" using 1:($2==0? $3: 1/0) w lp ls 1 t 'node 0',\
#	"Virt5gc-rtt.data" using 1:($2==1? $3: 1/0) w lp ls 2 t 'node 1'

	set title "< Throughput >"
	plot "Virt5gc-throughput.data" using 1:3 w lp t 'throughput' 

	set title "< Throughput >"
	plot "Virt5gc-throughput.data" using 1:3 w lp t 'throughput' 


	unset multiplot

	pause 1
#	print "a"
}
