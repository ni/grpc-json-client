# gRPC JSON Client Core Library

This project aims to enable languages that don't have native gRPC support (LabVIEW, MATLAB, etc.)
to communicate with gRPC services using JSON via a shared C/C++ library.

Languages that have their own protobuf compilers should not use this library.

## Sister Projects

[grpc-json-client-matlab](https://github.com/ni/grpc-json-client-matlab) - MATLAB implementation

## Installation

Download the latest release binaries from the
[releases](https://github.com/ni/grpc-json-client/releases) page.

To build the project from source, follow the instructions in
[BUILDING.md](https://github.com/ni/grpc-json-client/blob/main/BUILDING.md).

## Usage

The library entrypoints are detailed in
[grpc_json_client.h](https://github.com/ni/grpc-json-client/blob/main/src/grpc_json_client.h).
Place the shared library in a location where your program can load it then use your language's C
interop tools to make the appropriate library calls.
