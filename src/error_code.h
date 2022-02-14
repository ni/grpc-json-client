#pragma once

namespace ni
{
    namespace dynclient
    {
        enum ErrorCode : int
        {
            NONE = 0,
            UNKNOWN = -1,
            REFLECTION_ERROR = -2,
            SERVICE_NOT_FOUND = -3,
            METHOD_NOT_FOUND = -4,
            SERIALIZATION_ERROR = -5,
            DESERIALIZATION_ERROR = -6
        };
    }
}
