#pragma once

#include "types.h"
#include <optional>

namespace vsc {
    enum class DbTypes {
        none,
        sqlite,
        mysql,
        psql,
        oracle,
        access
    };

    struct ConnectionParameters {
        DbTypes type{DbTypes::none};

        std::optional<int> port{};
        std::optional<Text> host{};
        std::optional<Text> dataBaseName{};
        std::optional<Text> pathDb{};
        std::optional<Text> login{};
        std::optional<Text> password{};
    };
}