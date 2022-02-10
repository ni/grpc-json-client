import grpc_reflection.v1alpha.reflection_pb2
import grpc_reflection.v1alpha.reflection_pb2_grpc
import grpc
import google.protobuf.descriptor_pb2
import google.protobuf.descriptor_pool
import google.protobuf.descriptor_database
import google.protobuf.message_factory
import google.protobuf.json_format


with grpc.insecure_channel("sebu-sa-mandalore:31763") as channel:
    reflection_stub = grpc_reflection.v1alpha.reflection_pb2_grpc.ServerReflectionStub(channel)
    request = grpc_reflection.v1alpha.reflection_pb2.ServerReflectionRequest(
        list_services=""
    )
    response_stream = reflection_stub.ServerReflectionInfo(iter([request]))
    for response in response_stream:
        print(response)

    db = google.protobuf.descriptor_database.DescriptorDatabase()

    request = grpc_reflection.v1alpha.reflection_pb2.ServerReflectionRequest(
        file_containing_symbol="nirfsa_grpc.NiRFSA"
    )
    response_stream = reflection_stub.ServerReflectionInfo(iter([request]))
    for response in response_stream:
        for file_descriptor_proto in response.file_descriptor_response.file_descriptor_proto:
            file_descriptor = google.protobuf.descriptor_pb2.FileDescriptorProto.FromString(
                file_descriptor_proto
            )
            db.Add(file_descriptor)
        print(response)

    pool = google.protobuf.descriptor_pool.DescriptorPool(db)
    factory = google.protobuf.message_factory.MessageFactory(pool)
    method = pool.FindMethodByName("nirfsa_grpc.NiRFSA.Init")

    request_prototype = factory.GetPrototype(method.input_type)
    response_prototype = factory.GetPrototype(method.output_type)
    request = google.protobuf.json_format.ParseDict(
        {"session_name": "rfsa-session", "resource_name": "VST2_01"},
        request_prototype(),
        descriptor_pool=pool
    )
    print(request_prototype)
    print(response_prototype)

    rpc = channel.unary_unary(
        "/nirfsa_grpc.NiRFSA/Init",
        request_serializer=request_prototype.SerializeToString,
        response_deserializer=response_prototype.FromString
    )
    response = rpc(request)
    print(google.protobuf.json_format.MessageToJson(response, True, descriptor_pool=pool))
