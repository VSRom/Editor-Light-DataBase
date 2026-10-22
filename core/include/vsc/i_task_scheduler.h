#pragma once

#include <cstdint>
#include <functional>

namespace vsc {
    class ITaskScheduler {
    public:
        virtual ~ITaskScheduler() = default;
        virtual void schedule(std::function<void()>, std::uint32_t affinityHint = 0) = 0;

    };
}
