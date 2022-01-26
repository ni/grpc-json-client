## Instructions for Installing gRPC
1. Install dependencies
    - Visual Studio
        - Select the "Desktop development with C++ workload" in the Visual Studio Installer
    - [CMake](https://cmake.org/download/)
        - Select the option to add to `PATH` during install
    - [nasm](https://www.nasm.us/)
        - Manually add it to `PATH` after install
        - Run installer as admin to install for all users if desired

Note: Run remaining steps in PowerShell as Administrator

2. Clone the grpc repository
    - The `-b` option specifies the release version to clone (1.43 was the latest while writing this)
    - `--depth 1` option creates a shallow clone containing one commit since we don't need the full history to build the project
```
> git clone -b v1.43.0 --depth 1 https://github.com/grpc/grpc
> cd grpc
```

3. Initialize submodules
```
> git submodule update --init --depth 1
```

4. Build with CMake (this will take a while)
    - Warnings raised during the build can be safely ignored
```
> md .build
> cd .build
> cmake ..
> cmake --build . --config Release
```

5. Install grpc
    - Default install directory is `C:\Program Files (x86)\grpc`
```
> cmake --build . --config Release --target install
```

## Opening the Visual Studio CMake Project
The desktop development with C++ workload adds CMake support to Visual Studio.
To open the CMake project in `src/cpp` select the "Open a local folder" option from the
Visual Studio splash screen or the `File -> Open -> Folder..` option from the menu bar.

`CMakeLists.txt` provides the necessary configuration to find and link the Protobuf and gRPC
libraries to the project. It also configures the protobuf compiler to generate reflection
messages and client stubs for the `ServerReflection` service defined in `protos/reflection.proto`.
The generated files are placed in the `generated` directory.

The `protos/reflection.proto` file is taken from [here](https://github.com/grpc/grpc/blob/master/src/proto/grpc/reflection/v1alpha/reflection.proto).
The `proto_reflection_descriptor_database.*` files are taken from [here](https://github.com/grpc/grpc/tree/master/test/cpp/util)
and have been modified to use the generated reflection code.
