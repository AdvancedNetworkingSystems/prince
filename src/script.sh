## TUTORIAL:
## The script take 2 command-line arguments:
## $1: input folder
## $2: the type of input file, whether it's *.edges or *.json. Filetype is encoded by integer number 1, 2, 3
## $3: specify whether to calculate betweenness centrality as an unweighted graph (false) or weighted graph (true). This argument is for both Heuristic BC and Brandes BC
## $4: specify whether to include targets (and only targets, not sources) when calculating betweenness centrality (for the Brandes BC)

## If no argument is supplied, then the default is "./script.sh ../input/simple.edges 1 true true"
## If no argument is supplied, then the default is
        # ./script.sh ../input 1 false true

#########
## YOU CAN MODIFY THIS PART
#########

if [ -z "$1" ]; then
    inputdir="../input";
else
    inputdir="$1";
fi

if [ -z "$2" ]; then
    input_type=1;
else
    input_type="$2";
fi

# Change the variables to run the Betweenness Centrality for weighted or unweighted graph.
if [ -z "$3" ] # No argument supplied
then
    WEIGHTED_GRAPH="false"; # 2 possible values: [true | false]
else
    WEIGHTED_GRAPH="$3";
fi

if [ -z "$4" ] # No argument supplied
then
   ENDPOINTS_INCLUSION="true"; # 2 possible values: [true | false]
else
    ENDPOINTS_INCLUSION="$4";
fi

#########
## TRY TO AVOID MODIFYING ANYTHING BELOW THIS LINE
#########
## Compile the source code
# rm -f bi_connected_components.o
# rm -f sub_component.o

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

## Running the script
for file in `ls $inputdir | grep edges`;
do

    ./graph-parser ../input/$file $input_type $WEIGHTED_GRAPH $ENDPOINTS_INCLUSION
done


## Plotting the results
if [ $WEIGHTED_GRAPH="true" ]
then
    echo "weighted suffix"
    SUFFIX="weighted"; # the suffix used in the filename
else
    echo "unweighted suffix"
    SUFFIX="unweighted";
fi

for file in `ls ../output/ | grep out`;
do
    filename=${file%.*}
    option="FILENAME='$filename'; SUFFIX='$SUFFIX'";
    gnuplot -e "$option" plot_BC_comparison.gp
done