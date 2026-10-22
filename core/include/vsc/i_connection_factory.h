#pragma once

#include "connection_params.h"
#include "i_connection.h"
#include "sql_error.h"

#include <expected>
#include <memory>

namespace vsc {
    class IConnectionFactory {
    public:
        virtual ~IConnectionFactory() = default;

        IConnectionFactory(const IConnectionFactory&) = delete;
        IConnectionFactory& operator=(const IConnectionFactory&) = delete;
        IConnectionFactory(IConnectionFactory&&) = delete;
        IConnectionFactory& operator=(IConnectionFactory&&) = delete;

        [[nodiscard]] virtual std::expected<std::unique_ptr<IConnection>, SqlError>
        open(const ConnectionParameters& parameters) = 0;

    protected:
        IConnectionFactory() = default;
    };
}