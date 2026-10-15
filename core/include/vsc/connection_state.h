#pragma once

namespace vsc {
    enum class ConState {
        disposed,
        open,
        opening,
        closed,
        closing,
        failed
    };
}