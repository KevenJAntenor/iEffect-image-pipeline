set terminal pngcairo size 1024,768 enhanced font 'Verdana,10'
set output 'log-example.png'

plot 'log-example.txt' using 1:2 with steps ls 2
