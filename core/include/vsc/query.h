#pragma once

#include "value.h"
#include <optional>

namespace vsc {
    struct CmdQuery {
        Text sqlText{};
        std::pmr::vector<Value> sqlParams{};
        std::optional<int> timeOutMs{};

        enum class Mode {
            read,
            write,
            ddl,
            metadata
        };

        Mode mode{Mode::read};
    };
}