#pragma once

#include "vsc/value.h"
#include "vsc/sql_error.h"

#include <expected>
#include <QVariant>

namespace vsc::qt {
    std::expected<QVariant, vsc::SqlError> toQtValue(const vsc::Value& value);
    std::expected<vsc::Value, vsc::SqlError> fromQtValue(const QVariant& value, vsc::Text::allocator_type alloc);
}