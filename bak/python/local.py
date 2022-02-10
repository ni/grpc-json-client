import google.protobuf.descriptor_pb2
import google.protobuf.descriptor_pool
import google.protobuf.message_factory
import google.protobuf.json_format
import google.protobuf.service_reflection

with open("myproto.desc", "rb") as fd:
    data = fd.read()

fds = google.protobuf.descriptor_pb2.FileDescriptorSet.FromString(data)
pool = google.protobuf.descriptor_pool.DescriptorPool()
for file_descriptor in fds.file:
    pool.Add(file_descriptor)
descriptor = pool.FindMessageTypeByName("MyMessage")
print("descriptor:", type(descriptor))

factory = google.protobuf.message_factory.MessageFactory(pool)
# messages = factory.GetMessages(["myproto.proto"])
#my_message = messages["MyMessage"]
prototype = factory.GetPrototype(descriptor)
print("prototype:", type(prototype))

message = prototype()
print("message:", type(message))

message = google.protobuf.json_format.ParseDict({"my_int": 1}, message)
print(message.my_int)
print(prototype.SerializeToString(message) == message.SerializeToString())
