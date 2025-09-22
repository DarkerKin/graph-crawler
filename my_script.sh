#!/bin/bash
#SBATCH --job-name=particle_sim
#SBATCH --partition=Centaurus
#SBATCH --time=20:00:00
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err
#SBATCH --mem=8G

# Load necessary modules
module gcc


# Make sure your project uses the local RapidJSON
make

# Prepare results file
echo "Start Node,Depth,Nodes Visited,Time (s)" > results.csv

# Define tests using '|' as a delimiter for multi-word nodes
tests=(
  "Tom Hanks|1"
  "Tom Hanks|2"
  "Tom Hanks|3"
  "Leonardo DiCaprio|2"
  "Forrest Gump|2"
)

# Run tests
for test in "${tests[@]}"; do
    IFS="|" read -r node depth <<< "$test"
    echo "Running: $node depth $depth"

    output=$(./graph_crawler "$node" $depth 2>&1)

    # Extract results (adjust if your program prints differently)
    time=$(echo "$output" | grep "Execution time" | awk '{print $3}')
    visited=$(echo "$output" | grep "Total nodes visited" | awk '{print $4}')

    echo "\"$node\",$depth,$visited,$time" >> results.csv
done

echo "All tests completed. Results saved in results.csv"
