#pragma once

#include "types.h"

#include <memory_resource>

namespace vsc {
    struct ColumnMetaData {

        explicit ColumnMetaData(std::pmr::memory_resource* resources)
            : nameColumn(resources), type(resources)
        { }

        Text nameColumn{};
        Text type{};
        bool isPk{false};
        bool isNullable{true};
        int  ordinal{0};
    };
}