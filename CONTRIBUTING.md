# CONTRIBUTING
Contributions to grpc-json-client are welcome by all!
## Build Environment Setup
### Dependencies
1. Install [CMake](https://cmake.org/) and add it to `path`.
2. Install [gRPC](https://github.com/grpc/grpc) by following the instructions [here](https://github.com/grpc/grpc/blob/master/BUILDING.md).
### Getting the Source
1. Clone the repo:
```
> git clone https://github.com/ni/grpc-json-client.git
> cd grpc-json-client
```
2. Initialize submodules:
```
> git submodule update --init
```
### Building on Windows
1. Create a build directory:
```
> mkdir build
> cd build
```
2. Generate CMake cache:
```
> cmake .. "-DCMAKE_PREFIX_PATH=C:\Program Files (x86)\grpc"
``` 
3. Build with Debug configuration:
```
> cmake --build . --config Debug
```
### Tips
The "Desktop development with C++" workload for Visual Studio includes IDE support for CMake. A `CmakeSettings.json` file is provided that contains the default build configuration.
