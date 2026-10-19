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

        SqlError(MessageError c, SourceError s, Text m, Text nc) noexcept
            : codeError(c), source(s), message(std::move(m)), nativeCode(std::move(nc))
        { }

        MessageError codeError;
        SourceError source;
        Text message;
        Text nativeCode;
    };
}