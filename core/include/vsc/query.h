#pragma once

#include "value.h"
#include <string>

namespace vsc {
    struct CmdQuery {
        std::string sqlText{};
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