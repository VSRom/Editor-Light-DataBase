#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory_resource>

namespace vsc {
    using Text = std::pmr::string;
    using Blob = std::pmr::vector<std::uint8_t>;
}