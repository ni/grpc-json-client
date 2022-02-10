# CMakeList.txt : CMake project for grpc_json_client, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("grpc_dynamic_client")

set(CMAKE_CXX_STANDARD 11)

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

# Find GTest installation
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
find_package(GTest CONFIG REQUIRED)
message(STATUS "Using GTest ${GTest_VERSION}")

set(_GTEST GTest::gtest)
set(_GTEST_MAIN GTest::gtest_main)

# Paths
set(protos_dir "${CMAKE_SOURCE_DIR}/third_party/grpc/src/proto/grpc/reflection/v1alpha/")
set(reflection_proto_path "${protos_dir}/reflection.proto")
set(src_files_dir "${CMAKE_SOURCE_DIR}/src/")
set(generated_files_dir "${CMAKE_SOURCE_DIR}/generated/")
set(tests_dir "${CMAKE_SOURCE_DIR}/tests/")

# Generate sources
set(reflection_pb_hdr "${generated_files_dir}/reflection.pb.h")
set(reflection_pb_src "${generated_files_dir}/reflection.pb.cc")
set(reflection_grpc_hdr "${generated_files_dir}/reflection.grpc.pb.h")
set(reflection_grpc_src "${generated_files_dir}/reflection.grpc.pb.cc")
add_custom_command(
    OUTPUT "${reflection_pb_hdr}" "${reflection_pb_src}" "${reflection_grpc_hdr}" "${reflection_grpc_src}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${generated_files_dir}"
         --cpp_out "${generated_files_dir}"
         -I "${protos_dir}"
         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${reflection_proto_path}"
    DEPENDS "${reflection_proto_path}"
)

# Add source to this project's library
add_library(grpc_dynamic_client SHARED
    "${src_files_dir}/dynamic_client.cpp"
    "${src_files_dir}/error_code.h"
    "${src_files_dir}/exceptions.cpp"
    "${src_files_dir}/exports.cpp"
    "${src_files_dir}/unary_unary_dynamic_client.cpp"
    ${reflection_proto_src}
    ${reflection_grpc_src}
)
target_include_directories(grpc_dynamic_client PRIVATE ${generated_files_dir})
target_link_libraries(grpc_dynamic_client ${_REFLECTION} ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP})

# Add tests
add_executable(tests
    "${tests_dir}/test_dynamic_client.cpp"
)
#target_include_directories(tests PRIVATE
#    ${src_files_dir}
#    ${generated_files_dir}
#)
target_link_libraries(tests 
    ${_GTEST}
    ${_GTEST_MAIN}
)
