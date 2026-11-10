#pragma once

#include <QString>
#include <QByteArray>
#include <cstddef>

#include "vsc/types.h"

namespace vsc::qt {
    inline QString toQtText(const vsc::Text& text)
    {
        return QString::fromUtf8(
            text.data(),
            static_cast<qsizetype>(text.size()));
    }

    inline vsc::Text toVscText(const QString& text, vsc::Text::allocator_type alloc = {})
    {
        const QByteArray utf8 = text.toUtf8();
        vsc::Text output{alloc};
        output.assign(utf8.constData(), static_cast<std::size_t>(utf8.size()));

        return output;
    }
}