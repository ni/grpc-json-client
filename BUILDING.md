## Dependencies

1. Install [CMake](https://cmake.org/) and add it to `path`.
2. [Optional] Install [gRPC](https://github.com/grpc/grpc) by following the instructions
[here](https://github.com/grpc/grpc/blob/master/BUILDING.md).

### Getting the Source

1. Clone the repo:
```
> git clone https://github.com/ni/grpc-json-client.git
> cd grpc-json-client
```
2. Initialize submodules:
```
> git submodule update --init --recursive
```

## Building on Windows

### Create a build directory

```
> mkdir build
> cd build
```

### Generate CMake cache

If gRPC is installed (faster build):
```
> cmake .. "-DCMAKE_PREFIX_PATH=C:\Program Files (x86)\grpc"
```
- The `CMAKE_PREFIX_PATH` value may need to change match the installation directory of gRPC.

If gRPC is not installed:
```
> cmake .. -DGRPC_AS_SUBMODULE=ON
```

### Build with Debug configuration

```
> cmake --build . --config Debug
```