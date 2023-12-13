


### Words counter benchmarks

This is a series of a *very simple*, *not production ready* words counter
with several versions.

  - A monothread version
  - A simple futures multithreaded version
  - A futures version with load balancing
  - A futures version with memory mapped files
  
  
Just run the program


WARNING: the program has been tested under MacOS and Linux on my machines only.
If you have any trouble, please contact me.


### Requirements


  - gcc12 in Linux or gcc13 in MacOS
  - conan at least 1.62.0
  - python3
  - meson build system at least 1.3.0
  
  
#### Linux

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

#### MacOS 

``` sh

# This can take a while since Conan will download packages and probably build
meson setup -Dbuildtype=release build-release

# Please, note that for the first time the compilation stage could look stuck.
# This is because it needs to download the resources and uncompress.
meson compile -C build-release

ulimit -n 10240
# Run benchmarks
build-release/benchmarks/benchmarks

# Run implementations
./run-words-counter-implementations.sh
```


### Results

You can find the results in words-counter-results


### Results in my machine


### Benchmarks

MacOS:



### 
