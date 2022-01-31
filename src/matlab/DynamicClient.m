classdef DynamicClient

    properties (Constant, Access = 'private')
        LIBRARY = 'grpc_dynamic_client';
        HEADER = 'exports.h'
    end

    properties (Access = 'private')
        handle
    end

    methods (Access = 'public')
        function obj = DynamicClient(target)
            if ~libisloaded(DynamicClient.LIBRARY)
                loadlibrary(DynamicClient.LIBRARY, DynamicClient.HEADER);
            end
            [~, ~, obj.handle] = calllib(DynamicClient.LIBRARY, 'Init', target, []);
        end

        function response = query(obj, service, method, request)
            [error_code, ~, ~, ~, ~, response_handle] = calllib(DynamicClient.LIBRARY, 'Query', obj.handle, service, method, request, []);
            DynamicClient.check_error(error_code);
            [~, ~, ~, size] = calllib(DynamicClient.LIBRARY, 'ReadResponse', response_handle, [], 0);
            buffer = blanks(size);
            [~, ~, response] = calllib(DynamicClient.LIBRARY, 'ReadResponse', response_handle, buffer, size);
        end

        function response = query_struct(obj, service, method, request)
            encoded_request = jsonencode(request);
            encoded_response = obj.query(service, method, encoded_request);
            response = jsondecode(encoded_response);
        end

        function close(obj)
            calllib(DynamicClient.LIBRARY, 'Close', obj.handle);
        end
    end

    methods (Static, Access = 'private')
        function check_error(error_code)
            if error_code < 0
                error(DynamicClient.get_error_message(error_code));
            end
        end

        function error_message = get_error_message(error_code)
            [~, ~, size] = calllib(DynamicClient.LIBRARY, 'GetErrorMessage', error_code, [], 0);
            buffer = blanks(size);
            [~, error_message] = calllib(DynamicClient.LIBRARY, 'GetErrorMessage', error_code, buffer, size);
        end
    end
end
