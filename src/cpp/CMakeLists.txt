# CMakeList.txt : CMake project for grpc_json_client, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("grpc_json_client")

# Find Protobuf installation
# Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
set(_REFLECTION gRPC::grpc++_reflection)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Paths
set(protos_path "${CMAKE_SOURCE_DIR}/protos/")
set(reflection_proto "${protos_path}/reflection.proto")
set(generated_files_path "${CMAKE_SOURCE_DIR}/generated/")

# Generate sources
set(reflection_pb_hdr "${generated_files_path}/reflection.pb.h")
set(reflection_pb_src "${generated_files_path}/reflection.pb.cc")
set(reflection_grpc_hdr "${generated_files_path}/reflection.grpc.pb.h")
set(reflection_grpc_src "${generated_files_path}/reflection.grpc.pb.cc")
add_custom_command(
    OUTPUT "${reflection_pb_hdr}" "${reflection_pb_src}" "${reflection_grpc_hdr}" "${reflection_grpc_src}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${generated_files_path}"
         --cpp_out "${generated_files_path}"
         -I "${protos_path}"
         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${reflection_proto}"
    DEPENDS "${reflection_proto}"
)

# Add source to this project's library
add_executable(grpc_json_client "grpc_json_client.cpp" "proto_reflection_descriptor_database.cc" ${reflection_proto_src} ${reflection_grpc_src})
target_link_libraries(grpc_json_client ${_REFLECTION} ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP})

# TODO: Add tests and install targets if needed.
