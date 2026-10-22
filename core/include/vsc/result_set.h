#pragma once

#include "column_metadata.h"
#include "value.h"

#include <optional>
#include <vector>
#include <cstdint>
#include <memory_resource>
#include <memory>

namespace vsc {
    struct ResultSet {
        std::unique_ptr<std::pmr::monotonic_buffer_resource> arena;

        enum class ResultKind {
            rows,
            affected,
            none
        };

        ResultKind kind{ResultKind::none};
        std::pmr::vector<vsc::ColumnMetaData> cols;
        std::pmr::vector<std::pmr::vector<vsc::Value>> rows;

        std::int64_t affectedRows{-1};
        std::optional<std::int64_t> lastInsertId{};

        explicit ResultSet(size_t initial_buffer_size = 4096)
            : arena(std::make_unique<std::pmr::monotonic_buffer_resource>(initial_buffer_size))
            , cols(arena.get())
            , rows(arena.get())
        { }

        ResultSet(ResultSet&&) noexcept = default;
        ResultSet& operator=(ResultSet&&) noexcept = default;

        ResultSet(const ResultSet&) = delete;
        ResultSet& operator=(const ResultSet&) = delete;
    };
}