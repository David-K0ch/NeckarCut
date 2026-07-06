# NeckarCut - PACE 2026 Submission Guide


This repository contains the source code for the executable

`mySolution` developed for the PACE 2026 challenge. The project

implements optimized algorithms using customized internal data

structures (such as `myForest`, `myTree`, and an aligned `ObjectPool`)

and is optimized for high-performance execution.


---


## 1. External Dependencies



To maximize performance and compatibility across target test

environments, this project has been designed to be fully standalone. It

relies strictly on standard language compliance and build tools.


### Required Software

* **CMake**: Version 3.23 or higher.


* **C++ Compiler**: A modern compiler supporting standard C++20

conventions (e.g., `GCC` >= 11 or `Clang` >= 13). This is required

to support advanced syntax features, compiler-level standard

optimizations, and custom memory constraints (e.g.,

`std::aligned_alloc`).

* **Standard Library**: `libstdc++` or `libc++` providing base functionality.


### External Libraries


* **No External Dependencies**: The project **does not require** any

external third-party libraries (such as Boost, GMP, LEDA, or native

multi-threading extensions).


* All essential memory management structures (`ObjectPool`), linear

buffers (`Array`), queuing primitives (`Queue`), and domain-specific

graph primitives (`Forest`, `Tree`) are completely self-contained within

this repository.


---


## 2. Compilation and Installation



The build system utilizes CMake to configure, optimize, and generate a

static binary. Follow these operations sequentially from the repository

root directory to build the executable:


### Step 1: Create a Build Directory

An out-of-source build is highly recommended to isolate build artifacts from the source files:

```bash
mkdir build
cd build
```

### Step 2: Configure the Project

Run CMake to discover the environment toolchain and prepare the build configuration. By default, the CMakeLists.txt configures high-level compiler optimization targets (-O3, -flto) and forces static execution linkage (-static).

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Step 3: Compile the Executable

Build the binaries using CMake's build interface:

```bash
cmake --build . --config Release
```

Alternatively, for faster parallel compilation leveraging multiple CPU cores, you can use standard make:

```bash
make -j$(nproc)
```

Upon successful compilation, the standalone static binary mySolution will be generated inside the build directory.


## 3. Running the Executable

The binary follows the official PACE specifications for input/output processing. It reads problem instances directly from standard input (stdin) and prints the solution out to standard output (stdout).

```bash
./mySolution < path/to/input_instance.txt
```


Verifying the Static Linkage

To verify that the binary was compiled entirely statically and contains no dynamically shared runtime library requirements (as required by the submission environments), run:

```bash
ldd mySolution
```

Expected Output:

not a dynamic executable 
