#pragma once

#include "types.h"

namespace vsc {
    struct ColumnMetaData {
        Text nameColumn{};
        Text type{};
        bool isPk{false};
        bool isNullable{true};
        int  ordinal{0};
    };
}