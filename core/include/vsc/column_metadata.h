#pragma once

#include <string>

namespace vsc {
    struct ColumnMetaData {
        std::string nameColumn{};
        std::string type{};
        bool isPk{false};
        bool isNullable{true};
        int  ordinal{0};
    };
}