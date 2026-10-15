#pragma once

#include <string>

namespace vsc {
    struct SqlError {
        int codeError{0};
        std::string message{};
        std::string source{};
    };
}