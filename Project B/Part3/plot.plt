set terminal png size 1200, 800
set output "average_arrival_rate_lambda.png"
set title "Packets/Second"
set xlabel "Node Number"
set ylabel "Average Arrival Rate (Lambda)"
plot "lambda.data" using 1:2 with linespoints title "Lambda" lw 2

set terminal png size 1200, 800
set output "ThroughputGoodput.png"
set title "Throughput and Goodput"
set xlabel "Node Number"
set ylabel "Throughput and Goodput"
plot "throughputGoodput.data" using 1:2 with linespoints title "Throughput" lw 2, "throughputGoodput.data" using 1:3 with linespoints title "Goodput" lw 2