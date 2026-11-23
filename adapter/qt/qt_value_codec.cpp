#include "qt_value_codec.h"

std::expected<QVariant, vsc::SqlError> vsc::qt::toQtValue(const vsc::Value& value) {
    switch(value.type()) {
    case ValueType::null: return QVariant{};

    case ValueType::integer: {
        if (auto number = value.asInt64())
            return QVariant::fromValue<qlonglong>(*number);

        break;
    }

    case ValueType::real: {
        if (auto number = value.asDouble())
            return QVariant::fromValue<double>(*number);

        break;
    }

    case ValueType::boolean: {
        if (auto number = value.asBool())
            return QVariant::fromValue<bool>(*number);

        break;
    }

    case ValueType::text: {
        if (auto number  = value.asString())
            return QVariant{ QString::fromUtf8(number->data(),
                                  static_cast<qsizetype>(number->size()))
            };

        break;
    }

    case vsc::ValueType::blob: {
        if (auto blob = value.asBlob()) {
            const char* data = reinterpret_cast<const char*>(blob->data());

            return QVariant{ QByteArray{ data, static_cast<qsizetype>(blob->size()) } };
        }

        break;
    }

    case ValueType::decimal: {
        if (auto decimal = value.asDecimal()) {
            return QVariant{ QString::fromUtf8(decimal->repr.data(),
                                  static_cast<qsizetype>(decimal->repr.size()))
            };
        }

        break;
    }
    }

    return std::unexpected(vsc::SqlError{
        vsc::SqlError::MessageError::internal,
        vsc::SqlError::SourceError::statement,
        "Value conversion failed",
        {}
    });
}

std::expected<vsc::Value, vsc::SqlError> vsc::qt::fromQtValue(const QVariant& value, vsc::Text::allocator_type alloc) {
    if (!value.isValid() || value.isNull())
        return vsc::makeNull();

    switch (value.metaType().id()) {
    case QMetaType::Int:
    case QMetaType::LongLong:
        return vsc::makeInt(value.toLongLong());

    case QMetaType::Double:
    case QMetaType::Float:
        return vsc::makeReal(value.toDouble());

    case QMetaType::Bool:
        return vsc::makeBool(value.toBool());

    case QMetaType::QString: {
        const QByteArray utf8 = value.toString().toUtf8();

        return vsc::makeText(
            alloc,
            std::string_view{
                utf8.constData(),
                static_cast<std::size_t>(utf8.size())
            });
    }

    case QMetaType::QByteArray: {
        const QByteArray bytes = value.toByteArray();

        const auto* data =
            reinterpret_cast<const std::uint8_t*>(bytes.constData());

        return vsc::makeBlob(
            alloc,
            std::span<const std::uint8_t>{
                data,
                static_cast<std::size_t>(bytes.size())
            });
    }

    default:
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Unsupported Qt value type",
            {}
        });
    }
}