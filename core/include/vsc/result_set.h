#pragma once

#include "column_metadata.h"
#include "value.h"
#include <vector>

namespace vsc {
    struct ResultSet {
        std::vector<vsc::ColumnMetaData> metadata;
        std::vector<std::vector<vsc::Value>> value;
        std::vector<std::int64_t> affectedRows;
        std::int64_t lastInsertId;
    };
}