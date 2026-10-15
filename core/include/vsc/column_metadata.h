#pragma once

#include <string>

namespace vsc {
    struct ColumnMetaData {
        std::string nameColumn{};
        std::string type{};
        bool isPk{false};
        int ordinal{0};
    };
}