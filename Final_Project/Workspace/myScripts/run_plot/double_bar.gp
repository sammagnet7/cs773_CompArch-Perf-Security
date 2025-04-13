# mpki.gp - uses input_file and output_png from command line

set terminal pngcairo enhanced font "Arial,16" size 1200,600
set output output_png

set style data histogram
set style histogram clustered gap 1
set style fill solid border -1
set boxwidth 0.9

set title var_title
set ylabel var_ylabel
set xlabel var_xlabel
set grid y

# Rotate xtick labels
set xtics rotate by 45 right

stats input_file using 2:3 nooutput
set yrange [0:STATS_max_y * 1.2]

plot input_file using 2:xtic(1) title "Baseline", \
     '' using 3 title gem_flavour, \
     '' using 0:2:(sprintf("%.3f", column(2))) with labels font "Arial,14" offset -1.0,1.8 rotate by 90 tc rgb "#B030B0" notitle, \
     '' using 0:3:(sprintf("%.3f", column(3))) with labels font "Arial,14" offset 1.0,1.8 rotate by 90 tc rgb "#30D5C8" notitle
