
set datafile separator ", "

	set multiplot layout 3,1
	set tmargin 2

#	set style line 1 lt 1 lw 1 lc rgb "black"
#	set style line 2 lt 2 lw 1 lc rgb "red"


#	set title "< RTT >"
#	set ylabel 'Time (s)'  
#	set xlabel 'Time (s)' offset 44,1,0
#	plot "Virt5gc-rtt.data" using 1:($2==0? $3 : 1/0) w l ls 1 t 'node 0' ,\
#	"Virt5gc-rtt.data" using 1:($2==1? $3 : 1/0) w l ls 2 t 'node 1'

#	set title "< CWND >"
#	set ylabel 'Window size'
#	set xlabel 'Time (s)' offset 43,1	
#	plot "Virt5gc-cwnd.data" using 1:($2==0? $3 : 1/0) w l ls 1 t 'node 0' ,\
#        "Virt5gc-cwnd.data" using 1:($2==1? $3 : 1/0) w l ls 2 t 'node 1'


	set title "< Throughput >"
	set ylabel 'Mbps'
	set xlabel 'Time (s)' offset 45,1
	

        plot "Virt5gc-throughput.data" using 1:($2==0? $3 : 1/0) with lp lt 1 lc 'blue' pt 1 ps 2 t 'node 0' ,\
        "Virt5gc-throughput.data" using 1:($2==1? $3 : 1/0) with lp lt 1 lc 'red' pt 2 ps 2 t 'node 1'


	set title "< Scaling Delay >"
	set ylabel 'Time (s)'
	set xlabel 'Time (s)' offset 45,1

        plot "Virt5gc-scaling.data" using 1:($3==1? $4 : 1/0) with p pt 1 ps 2 lc 'blue' t 'scale in' ,\
        "Virt5gc-scaling.data" using 1:($3==0? $4 : 1/0) with p pt 2 ps 2 lc 'red' t 'scale out'




	set title "< OVS LTE Result >"
	set ylabel 'Received bytes'
	set xlabel 'Time (s)' offset 42,1
        plot "ovs_lte_result.txt" using 1:($2=='02-06-00:00:00:00:00:05'? $3 : 1/0) lc 'blue' w d t 'port A' ,\
        "ovs_lte_result.txt" using 1:($2=='02-06-00:00:00:00:00:07'? $3 : 1/0) lc 'red' w d t 'port B'




	unset multiplot

	pause 2 

reread
