#pragma once

#include "sql_error.h"

#include <expected>

namespace vsc {
    class ITransaction {
    public:
        virtual ~ITransaction() = default;

        ITransaction(const ITransaction&) = delete;
        ITransaction& operator=(const ITransaction&) = delete;
        ITransaction(ITransaction&&) = delete;
        ITransaction& operator=(ITransaction&&) = delete;

        [[nodiscard]] virtual std::expected<void, SqlError> commit() = 0;
        [[nodiscard]] virtual std::expected<void, SqlError> rollback() = 0;

    protected:
        ITransaction() = default;
    };
}