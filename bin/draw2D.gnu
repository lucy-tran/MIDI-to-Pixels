set terminal x11 title 'Music Visualization'
# set title 'Music Visualization'
set border 0
unset xtics; unset ytics; unset ztics
set rmargin 2; set lmargin 2; set bmargin 2; set tmargin 2;
rgb(r,g,b) = 65536 * int(r) + 256 * int(g) + int(b)
set terminal png size 2400,300 
set output 'output.png'
plot 'rgb.dat' using 1:2:(rgb($3,$4,$5)) with points pt 7 lc rgb variable notitle
