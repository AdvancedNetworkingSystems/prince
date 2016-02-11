make
../bin/main

# visualization
echo "Start plotting";
gnuplot -e "FILENAME='simple.edges.out'" plot_BC_comparison.gp
gnuplot -e "FILENAME='ninux_unweighted_connected.edges.out'" plot_BC_comparison.gp
gnuplot -e "FILENAME='ninux_30_1.edges.out'" plot_BC_comparison.gp
gnuplot -e "FILENAME='jsoninfo_topo.json.out'" plot_BC_comparison.gp
gnuplot -e "FILENAME='olsr-netjson.json.out'" plot_BC_comparison.gp
echo "Done with plotting"