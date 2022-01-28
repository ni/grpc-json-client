classdef DynamicClient

    properties (Constant)
        LIBRARY = 'grpc_dynamic_client';
        HEADER = 'exports.h'
    end

    properties (Access = 'private')
        handle
    end

    methods
        function obj = DynamicClient(target)
            if ~libisloaded(obj.LIBRARY)
                loadlibrary(obj.LIBRARY, obj.HEADER);
            end
            [~, ~, obj.handle] = calllib(obj.LIBRARY, 'Init', target, []);
        end

        function response = query(obj, service, method, request)
            [~, ~, ~, ~, ~, response_handle] = calllib(obj.LIBRARY, 'Query', obj.handle, service, method, request, []);
            [~, ~, ~, size] = calllib(obj.LIBRARY, 'ReadResponse', response_handle, [], 0);
            buffer = blanks(size);
            [~, ~, response] = calllib(obj.LIBRARY, 'ReadResponse', response_handle, buffer, size);
        end

        function response = query_struct(obj, service, method, request)
            encoded_request = jsonencode(request);
            encoded_response = obj.query(service, method, encoded_request);
            response = jsondecode(encoded_response);
        end

        function close(obj)
            calllib(obj.LIBRARY, 'Close', obj.handle);
        end
    end
end