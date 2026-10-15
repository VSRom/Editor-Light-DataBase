#pragma once

#include <string>

namespace vsc {
    struct ConnectionParameters {
        int port{0};
        std::string host{0};
        std::string login{};
        std::string password{};
        std::string path{};
        std::string driver{};
        std::string option{};

    };
}