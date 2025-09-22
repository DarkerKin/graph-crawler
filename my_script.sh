#!/bin/bash
#SBATCH --job-name=particle_sim
#SBATCH --partition=Centaurus
#SBATCH --time=20:00:00
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err
#SBATCH --mem=20G

module load gcc curl rapidjson   # load modules if required



make

echo "Start Node,Depth,Nodes Visited,Time (s)" > results.csv

# Define tests
tests=(
  "Tom Hanks 1"
  "Tom Hanks 2"
  "Tom Hanks 3"
  "Leonardo DiCaprio 2"
  "Forrest Gump 2"
)

for test in "${tests[@]}"; do
    node=$(echo $test | awk '{print $1 " " $2}')
    depth=$(echo $test | awk '{print $3}')

    echo "Running: $node depth $depth"

    output=$(./graph_crawler "$node" $depth 2>&1)

    time=$(echo "$output" | grep "Execution time" | awk '{print $3}')
    visited=$(echo "$output" | grep "Total nodes visited" | awk '{print $4}')

    echo "\"$node\",$depth,$visited,$time" >> results.csv
done
