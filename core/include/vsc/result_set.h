#pragma once

#include "column_metadata.h"
#include "value.h"
#include <vector>
#include <cstdint>
#include <optional>

namespace vsc {
    struct ResultSet {
        enum class ResultKind {
            rows,
            affected,
            none
        };

        ResultKind kind{ResultKind::none};

        std::vector<vsc::ColumnMetaData> cols;
        std::vector<std::vector<vsc::Value>> rows;
        std::int64_t affectedRows{-1};
        std::optional<std::int64_t> lastInsertId{};
    };
}