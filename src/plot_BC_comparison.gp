set term postscript enhanced color
set terminal postscript eps

if (!exists("FILENAME")) FILENAME='simple.edges.out'
if (!exists("SUFFIX")) SUFFIX=''

INPUT_FILEPATH = sprintf("../output/%s.out", FILENAME)
set output sprintf("../output/visualization/%s.eps", FILENAME)

set title sprintf("Centrality for %s", FILENAME) noenhanced # to display underscore instead of subscript
set xlabel "Router (each integer corresponds to one router)"
set ylabel "Betweenness Centrality"

plot INPUT_FILEPATH using 2 title sprintf('brandes %s (targets inclusion)', SUFFIX) pointtype 5 pointsize 1, \
     INPUT_FILEPATH using 3 title sprintf('heuristic %s (substracted by bc\_inter)', SUFFIX) pointtype 7 pointsize 0.7