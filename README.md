# Maximum Agreement Forest - Exact Solver

This repository contains an exact Branch-and-Bound solver for the **Maximum Agreement Forest (MAF)** problem on multiple rooted phylogenetic trees. It was developed for the Exact Track of the **PACE 2026 Challenge**.

The solver determines the minimum number of edge cuts required to decompose a set of input trees into a common subforest (the agreement forest).

## Prerequisites

To compile the solver, ensure you have the following installed on your system:
* A modern C++ compiler (e.g., GCC or Clang)
* CMake (version 3.23 or higher)
* Make (or another compatible build system)

## Installation Guide

We provide a simple setup script to build the project in an isolated `build/` directory using CMake.

1. Clone or navigate to the repository directory.
2. Ensure the setup script is executable:
   ```bash
   chmod +x setup.sh
   ```
3. Run the setup script to build the project in Release mode:
   ```bash
   ./setup.sh
   ```

This script will automatically create a `build` directory, configure the CMake project for optimal performance (`-O3 -flto`), and compile the final executable binary.

## How to Use

The binary reads instances from the **standard input** (`stdin`) and prints the result to the **standard output** (`stdout`).

### Input Format
The solver expects the standard PACE 2026 MAF format. It starts with a descriptor line indicating the number of trees and leaves:
```text
# p <number_of_trees> <number_of_leaves>
```
Followed by the individual phylogenetic trees represented as strings (Newick-like format).

### Running the Solver

Once the project is successfully built, you can run the solver by piping an input instance file into the binary. 

For example, to run an instance located at `pace26_exact_pub/instance.txt`:

```bash
cat pace26_exact_pub/instance.txt | ./build/mySolution
```
or alternatively using input redirection:
```bash
./build/mySolution < pace26_exact_pub/instance.txt
```