#pragma once

namespace vsc {
    enum class ConState {
        closed,
        disposed,           // Терминальное
        open,
        failed
    };
}