#pragma once

#include <QMetaEnum>

namespace sparsely
{
    namespace Enums
    {
        Q_NAMESPACE
        enum class Method : std::uint8_t
        {
            DCA = 0U,
            FGSPCA,
            BGSPCA,
            CUSTOM,
            USERDYNAMICLIB
        };

        enum class PlotType : std::uint8_t
        {
            FILLED = 0U,
            IMPULSES
        };

        Q_ENUM_NS(Method)
        Q_ENUM_NS(PlotType)
    }; // namespace Enums
} // namespace sparsely
