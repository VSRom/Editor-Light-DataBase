#pragma once

#include "column_metadata.h"

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <vector>

namespace vsc {
    struct TableList {
        private:
            std::unique_ptr<std::pmr::monotonic_buffer_resource> arena;

        public:
            [[nodiscard]] std::pmr::polymorphic_allocator<char> allocator() const noexcept { return arena.get(); }

            std::pmr::vector<Text> tables;

            explicit TableList(std::size_t initial_buffer_size = 4096)
                : arena(std::make_unique<std::pmr::monotonic_buffer_resource>(initial_buffer_size)),
                tables(arena.get())
            { }

            TableList(TableList&&) noexcept = default;

            TableList& operator=(TableList&&) = delete;
            TableList(const TableList&) = delete;
            TableList& operator=(const TableList&) = delete;
    };

    struct TableMetadata {
        private:
            std::unique_ptr<std::pmr::monotonic_buffer_resource> arena;

        public:
            [[nodiscard]] std::pmr::polymorphic_allocator<char> allocator() const noexcept { return arena.get(); }

            Text tableName;
            std::pmr::vector<ColumnMetaData> columns;

            explicit TableMetadata(std::size_t initial_buffer_size = 4096)
                : arena(std::make_unique<std::pmr::monotonic_buffer_resource>(initial_buffer_size)),
                tableName(arena.get()), columns(arena.get())
            { }

            TableMetadata(TableMetadata&&) noexcept = default;

            TableMetadata& operator=(TableMetadata&&) = delete;
            TableMetadata(const TableMetadata&) = delete;
            TableMetadata& operator=(const TableMetadata&) = delete;
    };
}

