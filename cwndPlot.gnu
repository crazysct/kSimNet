set multiplot layout 2,2  title "Sinr Statistics"
set tmargin 4
set autoscale y
set key box opaque

set grid
set size 0.5, 0.5
set origin 0.0,0.5
plot "UE-1-TCP-CWND.txt" with lines

set grid
set size 0.5, 0.5
set origin 0.5,0.5
plot "UE-1-TCP-RTT.txt" with lines

set grid
set size 0.5, 0.5
set origin 0.0,0.0
plot "UE-2-TCP-CWND.txt" with lines

set grid
set size 0.5, 0.5
set origin 0.5,0.0
plot "UE-2-TCP-RTT.txt" with lines

pause 0.5

reread
