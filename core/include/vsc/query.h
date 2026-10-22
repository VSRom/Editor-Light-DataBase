#pragma once

#include "value.h"
#include <optional>
#include <memory_resource>

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