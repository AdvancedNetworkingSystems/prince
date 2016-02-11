set term postscript enhanced color
set terminal postscript eps

if (!exists("FILENAME")) FILENAME='simple.edges.out'
INPUT_FILEPATH = sprintf("../output/%s", FILENAME)
set output sprintf("../output/visualization/%s.eps", FILENAME)
set title "Centrality for edge_list"
set xlabel "Router (each integer corresponds to one router)"
set ylabel "Betweenness Centrality"

plot INPUT_FILEPATH using 2 title 'brandes (no inclusion of endpoints)' pointtype 7 pointsize 0.7, \
     INPUT_FILEPATH using 3 title 'heuristic (substracted by bc\_inter)' pointtype 5 pointsize 1