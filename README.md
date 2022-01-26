## Instructions for Installing gRPC
1. Install dependencies
    - Visual Studio
        - Select the "Desktop development with C++ workload" in the Visual Studio Installer
    - [CMake](https://cmake.org/download/)
        - Select the option to add to `PATH` during install
    - [nasm](https://www.nasm.us/)
        - Mnaully add it to `PATH` after install
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
    - Default install director is `C:\Program Files (x86)\grpc`
```
> cmake --build . --config Release --target install
```

## Creating a Visual Studio gRPC Project
Instructions
