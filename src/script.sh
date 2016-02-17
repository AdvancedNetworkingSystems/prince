## TUTORIAL:
## The script take 2 command-line arguments:
## $1: specify whether to calculate betweenness centrality as an unweighted graph (false) or weighted graph (true). This argument is for both Heuristic BC and Brandes BC
## $2: specify whether to include targets when calculating betweenness centrality (for the Brandes BC)

## If no argument is supplied, then the default is "./script.sh true true"

#########
## YOU CAN MODIFY THIS PART
#########
# Change the variables to run the Betweenness Centrality for weighted or unweighted graph.
if [ -z "$1" ] # No argument supplied
then
    WEIGHTED_GRAPH="true"; # 2 possible values: [true | false]
else
    WEIGHTED_GRAPH="$1";
fi

if [ -z "$2" ] # No argument supplied
then
   TARGETS_INCLUSION="true"; # 2 possible values: [true | false]
else
    TARGETS_INCLUSION="$1";
fi

#########
## TRY TO AVOID MODIFYING ANYTHING BELOW THIS LINE
#########
## Compile the source code
make

## Create output directories if they are not existed
declare -a dir_arr=("../output" "../output/visualization")
for i in "${dir_arr[@]}"
do
    echo $i;
    if [ ! -d $i ]; then
        mkdir $i;
    fi
done

filepath="../input/simple.edges"
input_type=1
## Running the script
./graph-parser $filepath $input_type $WEIGHTED_GRAPH $TARGETS_INCLUSION

## Plotting the results
if [ $WEIGHTED_GRAPH = "true" ]
then
    SUFFIX="weighted"; # the suffix used in the filename
else
    SUFFIX="unweighted";
fi

## declare an array variables
declare -a arr=("simple" "ninux_unweighted_connected" "ninux_30_1" "jsoninfo_topo" "olsr-netjson")

# loop through the array and format the option for gnuplot
# gnuplot -e "FILENAME='simple'; SUFFIX='$SUFFIX'" plot_BC_comparison.gp
for i in "${arr[@]}"
do
    option="FILENAME='"$i"_"$SUFFIX"'; SUFFIX='$SUFFIX'";
    gnuplot -e "$option" plot_BC_comparison.gp
done
