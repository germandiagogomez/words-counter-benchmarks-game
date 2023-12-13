# Words counter benchmarks

This is a series of a *very simple*, *not production ready* words counter
with several versions.

  - A monothread version
  - A simple futures multithreaded version
  - A futures version with load balancing
  - A futures version with memory mapped files
  
  
Just run the program


WARNING: the program has been tested under MacOS and Linux on my machines only.
If you have any trouble, please contact me.


## Requirements


  - gcc12 in Linux or gcc13 in MacOS
  - conan at least 1.62.0
  - python3
  - meson build system at least 1.3.0
  
  
### Linux

``` sh
# This can take a while since Conan will download packages and probably build
meson setup -Dbuildtype=release build-release

# Please, note that for the first time the compilation stage could look stuck.
# This is because it needs to download the resources and uncompress.
meson compile -C build-release

# Run benchmarks
build-release/benchmarks/benchmarks

# Run implementations
./run-words-counter-implementations.sh
```

### MacOS 

``` sh

# This can take a while since Conan will download packages and probably build
meson setup -Denable_internal_profiling=true -Dconan_profile=gcc13_macos --native-file meson/native/compilers/gcc13_macos.ini -Dbuildtype=release build-release

# Please, note that for the first time the compilation stage could look stuck.
# This is because it needs to download the resources and uncompress.
meson compile -C build-release

ulimit -n 10240
# Run benchmarks
build-release/benchmarks/benchmarks

# Run implementations
./run-words-counter-implementations.sh
```


## Results

You can find the results in words-counter-results once you run run-words-counter-implementations.h

Some example of the top-level of the output:

In Mac, with profiling enabled, which is avialable for `BoostFutureMemMappedFileWordsCounter`
it shows this information:


```
Count words threads: 2. Split words threads: 1. Merge results threads: 1

        Split words/nthreads: (8.918432000000001s (normalized: 0.2800526082765165)
        Count words/nthreads: 7.6501795s (normalized: 0.24022751115426308)
        Merge results/nthreads: 15.276948000000003s (normalized: 0.4797198805692203)
```

### Results for each implementation (MacOS)

#### MonoThreadWordsCounter

```
MiB read: 2041.65
Execution time: 62.867s
Processing speed: 32.48 MiB/s
Number of files processed: 1024
Words/s: 5437369.1
```


#### AsyncWordsCounter

```
MiB read: 2041.65
Execution time: 22.136s
Processing speed: 92.23 MiB/s
Number of files processed: 1024
Words/s: 15442314.8
```

#### ExecutorBasedFutureWordsCounter

```
MiB read: 2037.65
Execution time: 17.919s
Processing speed: 113.71 MiB/s
Number of files processed: 1024
Words/s: 19076459.7
```

#### BoostFutureMemMappedFileWordsCounter 

```
Count words threads: 3. Split words threads: 2. Merge results threads: 3

        Split words/nthreads: (4.9611374999999995s (normalized: 0.2657448262770586)
        Count words/nthreads: 5.689743s (normalized: 0.3047727996041453)
        Merge results/nthreads: 8.017921333333334s (normalized: 0.42948237411879614)
        
MiB read: 2037.65
Execution time: 10.214s
Processing speed: 199.50 MiB/s
Number of files processed: 1024
Words/s: 33466916.1
```
