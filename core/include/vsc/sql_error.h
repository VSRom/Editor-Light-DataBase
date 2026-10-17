#pragma once

#include "types.h"

namespace vsc {
    struct SqlError {
        enum class SourceError {
            connection,
            statement,
            transaction,
            driver
        };

        enum class MessageError {
            driverMissing,
            connectionFailed,
            notOpen,
            syntax,
            constraintViolation,
            timeout,
            unsupported,
            internal
        };

        SqlError(MessageError c, SourceError s, Text m, Text nc)
            : codeError(c), source(s), message(m), nativeCode(nc)
        { }

        MessageError codeError;
        SourceError source;
        Text message;
        Text nativeCode;
    };
}