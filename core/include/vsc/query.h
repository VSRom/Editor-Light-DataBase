#pragma once

#include "value.h"

namespace vsc {
    struct CmdQuery {
        Text sqlText{};
        std::vector<vsc::Value> sqlParams{};
        bool timeOut{false};

        enum class Mode {
            read,
            write,
            ddl,
            metadata
        };
    };
}