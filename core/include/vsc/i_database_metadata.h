#pragma once

#include "schema_metadata.h"
#include "sql_error.h"

#include <expected>
#include <string_view>

namespace vsc {
    class IDatabaseMetadata {
    public:
        [[nodiscard]] virtual std::expected<vsc::TableList, vsc::SqlError> listUserTables() = 0;
        [[nodiscard]] virtual std::expected<vsc::TableMetadata, vsc::SqlError> describeTable(std::string_view tableName) = 0;

        virtual ~IDatabaseMetadata() = default;

        IDatabaseMetadata(const IDatabaseMetadata&) = delete;
        IDatabaseMetadata& operator=(const IDatabaseMetadata&) = delete;
        IDatabaseMetadata(IDatabaseMetadata&&) = delete;
        IDatabaseMetadata& operator=(IDatabaseMetadata&&) = delete;

    protected:
        IDatabaseMetadata() = default;
    };
}
