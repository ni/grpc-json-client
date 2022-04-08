# Introduction

The gRPC JSON Client is a library that allows languages without [native gRPC support](https://grpc.io/docs/languages/) (such as MATLAB) to communicate with gRPC services using [JSON](https://www.w3schools.com/js/js_json_intro.asp).  The use cases for the library are broad, but the main purpose is to enable remote control of NI instrumentation by pairing it with [grpc-device](https://github.com/ni/grpc-device), NI's gRPC Device Server.  Since strings are the primary communications mechanism, the library offers a familiar look and feel to those already comfortable with traditional message-based communications standards like SCPI.

In this Getting Started Guide, we'll cover how to formulate and interpret gRPC JSON messages with an eye towards NI's driver implementation.  First, we'll examine some of the basic concepts of gRPC and Protocol Buffers, the underlying technologies from which the client and server are built on.  Then, we'll establish a methodology for composing messages by mapping NI instrument driver C API calls to their corresponding gRPC calls.  Finally, we'll end on how to build JSON strings from message contents.  By the end of this guide, you will become familiar with the relevant concepts, procedures, and documentation required to develop gRPC applications with NI devices.

## gRPC and Protocol Buffers

[gRPC](https://grpc.io/) is an open source remote procedure call (RPC) framework built and maintained by Google.  The framework internally uses Google's [Protocol Buffers](https://developers.google.com/protocol-buffers) to structure and serialize data into messages that are sent between a client and a server during the course of a RPC.  If you're starting to worry this might get complicated, don't.  A deep understanding of these technologies isn't required to be successful here.  However, it is important to know that the topics we'll discuss carry a dependency on gRPC, so there are still a few important pieces we need to cover. 

## Services

gRPC is rooted in services.  Services are entities which perform some action on our behalf and return a result.  We typically don't know or care how the result is obtained, just that the service returns it to us in a reasonable time.

Think of going to a restaurant and ordering a glass of water.  The waiter or waitress serving the request provides a beverage service that includes ordering water.  How they obtain it typically isn't your concern.  All you care about is that you ordered a drink and expect to receive it in a timely manner.  Your request may also include a few parameters, such as the size, the amount of ice, and an option to include a garnish.

Whether you decide to use them or not, the waiter likely provides other services too, such as a food service or a bottle service.

The conclusion to draw from this analogy is that a server (the waiter) is capable of providing multiple services, each with its own set of actions.  Those actions in turn have their own sets of input and output parameters.

# Proto Files

At the heart of gRPC is the [`.proto`](https://developers.google.com/protocol-buffers/docs/proto3) file.  These files define services, methods, and messages using the Protocol Buffer syntax.  Their primary purpose is for generating [interface code and stubs](https://developers.google.com/protocol-buffers/docs/proto3#services), but we won't be generating any source code here.  Our primary use for them will be as a reference since they house all of the information required for properly invoking a RPC. 

Here is a subset of the [`nirfsa.proto`](https://github.com/ni/grpc-device/blob/main/generated/nirfsa/nirfsa.proto) file in the `grpc-device` project.  We will reference it several times in the following sections.

```protobuf
package nirfsa_grpc;

import "nidevice.proto";
import "session.proto";

service NiRFSA {
  rpc Init(InitRequest) returns (InitResponse);
}
```

**Note**: Several lines from the original file have been intentionally omitted for simplicity.

## Service Definitions

Skipping the first few lines for a moment, we can see that a [service](https://developers.google.com/protocol-buffers/docs/proto#services) named `NiRFSA` is defined.  The service in turn defines a single method called `Init` (wrapped in curly braces) which accepts an `InitRequest` and returns an `InitResponse`.  These are message types and their definitions are further down in the file.  We will cover message definitions soon.  For now, let's examine what the other lines are doing too.  

## Package Specifier

The [`package`](https://developers.google.com/protocol-buffers/docs/proto3#packages) specifier creates a namespace for all definitions in the `.proto` file.  Namespace elements are accessed via dot notation.  Therefore, the full name of the service defined above is `nirfsa_grpc.NiRFSA` and the full name of the method is `nirfsa_grpc.NiRFSA.Init`.  This helps prevent name collisions when importing other `.proto` files.

## Import Statements

The [`import`](https://developers.google.com/protocol-buffers/docs/proto3#importing_definitions) statement allows definitions from other `.proto` files to be referenced by the current file.  Imports allow shared components to be placed in their own `.proto` files rather than requiring redefinition wherever they are needed.  All of the NI instrument driver services carry a dependency on [`session.proto`](https://github.com/ni/grpc-device/blob/main/source/protobuf/session.proto).  More on this dependency will be covered later in the [The Session Message](#the-session-message) section.

## Message Definitions

Further down the `.proto` file we will find the definitions for the `InitRequest` and `InitResponse` messages.

```protobuf
message InitRequest {
  string session_name = 1;
  string resource_name = 2;
  bool id_query = 3;
  bool reset = 4;
}

message InitResponse {
  int32 status = 1;
  nidevice_grpc.Session vi = 2;
  string error_message = 3;
}
```

### Fields

Each [message](https://developers.google.com/protocol-buffers/docs/proto#simple) defines a set of fields (wrapped in curly braces) and each field has a name, a [datatype](https://developers.google.com/protocol-buffers/docs/proto3#specifying_field_types), and a [unique number](https://developers.google.com/protocol-buffers/docs/proto3#assigning_field_numbers).  The unique number is a low level implementation detail that can be safely ignored for our purposes.  In this case, the `resource_name` field of the `InitRequest` message is a `string`.

### Nested Messages

Protocol Buffers also support [nested messages](https://developers.google.com/protocol-buffers/docs/proto3#nested).  Notice the `InitResponse` message's `vi` field is a `nidevice_grpc.Session` message type.  This type is defined in `session.proto` and was imported via the `import "session.proto"` statement discussed [earlier](#import-statements).

## The Session Message

NI's instrument drivers require sessions to identify resources and manage memory.  Sessions are the mechanism that allows the same driver software to communicate with multiple devices simultaneously.  Most if not all methods will require a session to identify which resource (or set of resources) an action should be performed on.  Sessions are encapsulated into their own message type to provide consistency across drivers and to enable a common set of [utilities](https://github.com/ni/grpc-device/wiki/Session-Utilities-API-Reference).  Here is the `Session` message definition from the [`session.proto`](https://github.com/ni/grpc-device/blob/main/source/protobuf/session.proto) file.

```protobuf
message Session {
  oneof session {
    string name = 1;
    uint32 id = 2;
  }
}
```

The [`oneof`](https://developers.google.com/protocol-buffers/docs/proto3#oneof) field named `session` specifies that at most one of the enclosing types can be used.  For the `Session` message above, the `session` field can either contain the name of the session as a `string` or the ID of the session as an `uint32`.  As we saw [previously](#message-definitions), the `InitResponse` message returns a nested `Session` message in its `vi` field.  The value of this field can be used by subsequent RPCs to control the initialized device.

The semantics of using either the `name` or `id` fields will be covered later in the [Session Names](#session-names) section.

# Driver Services

Every instrument driver available in NI's Device Server is exposed as a separate service.  As we've [already seen](#proto-files), NI-RFSA is defined in the `nirfsa_grpc.NiRFSA` service.  Other drivers, like NI-RFSG and NI-DAQmx, are defined in the [`nirfsg_grpc.NiRFSG`](https://github.com/ni/grpc-device/blob/main/generated/nirfsg/nirfsg.proto) and [`nidaqmx_grpc.NiDAQmx`](https://github.com/ni/grpc-device/blob/main/generated/nidaqmx/nidaqmx.proto) services respectively.  For a full list of supported drivers, see the table [here](https://github.com/ni/grpc-device#supported-ni-drivers).

## Mapping to the C API

Every driver service is derived from the driver's C API.  The [Converting C API Calls to gRPC](https://github.com/ni/grpc-device/wiki/Creating-a-gRPC-Client#converting-c-api-calls-to-grpc) topic in the `grpc-device` wiki covers mapping the driver C functions to gRPC service methods in great detail.  Here are the important points:
1. Every C function call has a corresponding service method.
2. Every method accepts a `{FUNCTION_NAME}Request` message that encapsulates the function's input parameters.
3. Every method returns a `{FUNCTION_NAME}Response` message that encapsulates the function's output parameters.
4. Constants have been grouped into [Enumerations](https://developers.google.com/protocol-buffers/docs/proto#enum).

Let's examine the [C prototype](https://www.ni.com/docs/en-US/bundle/ni-rfsa-20.7-c-api-ref/page/rfsacref/cvinirfsa_init.html) of the `NiRFSA.Init` method next to the `nirfsa.proto` definitions.

```C
ViStatus niRFSA_init(ViRsrc resourceName, ViBoolean IDQuery, ViBoolean reset, ViSession* vi);
```

```protobuf
service NiRFSA {
  rpc Init(InitRequest) returns (InitResponse);
}

message InitRequest {
  string session_name = 1;
  string resource_name = 2;
  bool id_query = 3;
  bool reset = 4;
}

message InitResponse {
  int32 status = 1;
  nidevice_grpc.Session vi = 2;
  string error_message = 3;
}
```

As we expect, each input parameter is defined as a field in the `InitRequest` message and each output parameter is defined as a field in the `InitResponse` message.  The parameter names have changed slightly to meet the `proto3` style guidelines, but their semantics haven't.

Now for the bigger differences.  The `vi` output parameter is defined as the `nidevice_grpc.Session` message we [discussed earlier](#the-session-message).  The remaining fields, `session_name` and `error_message`, are extra fields.  As a rule of thumb, fields that map directly to the C prototype offer core functionality of the driver while the extra fields are accessories for features specific to NI's Device Server.  If you aren't yet aware of how to use them, that's okay, you can still expect correct behavior by using their default values.  In this situation, the `error_message` field returns the corresponding error message for a non-zero `status` field if an error occurs while initializing the session.  The [General API Differences](https://github.com/ni/grpc-device/wiki/General-API-Differences) section of the `grpc-device` wiki describes other subtle differences that exist due to the languages' inherent differences.

# Building JSON Messages

Now that we've made the connection between the instrument driver C APIs and related services, we can start building JSON messages.  JSON defines a [succinct set of data types](https://www.w3schools.com/js/js_json_datatypes.asp), each with a [mapping to Protocol Buffers types](https://developers.google.com/protocol-buffers/docs/proto3#json).  A message maps to a [JSON Object Literal](https://www.w3schools.com/js/js_json_objects.asp), so your JSON strings will always start and end with a curly brace.  Each field is represented as a key/value pair in the JSON object.  The key is the name of the field and the value is the JSON string representation of the field's value.  Here is the `InitRequest` message in JSON with the `resource_name`, `id_query`, and `reset` fields set.  Fields that are not explicitly set will assume the default value for their type.

```json
{
  "resource_name": "VST_01",
  "id_query": true,
  "reset": false
}
```

Here is a response we might expect to see after passing this message to the `NiRFSA.Init` method.

```json
{
  "status": 0,
  "vi": {
    "id": 1
  },
  "error_message": ""
}
```

That's all there is to it!  JSON is everywhere, so don't forget to research whether the language you are using offers any neat tools for working with it.

# Session Names

[Earlier in this guide](#the-session-message), we introduced the `nidevice_grpc.Session` message defined in `session.proto`.  This message is nested in requests for every method that acts on a session.  When a session is initialized, the response will include a `Session` instance (typically named `vi`) with the `id` field set.  In most situations, it is sufficient to store this value in a variable for use in proceeding calls.  However, occasionally it may be more desirable to specify a unique name for a session instead of an ID.  This can facilitate [reserving sessions](https://github.com/ni/grpc-device/wiki/Session-Utilities-API-Reference#reservereserverequest) by using the session name as the `reservation_id`.

NI's Device Server maintains a mapping of session names to session IDs.  If a name is supplied instead of an ID, the server will lookup the ID for the named session.  For this to work properly though, a unique name needs to be supplied to the server when the session is initialized.  To do so, supply an argument to the `session_name` parameter of the request when initializing the session.
