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

        enum class SaveStatus : std::uint8_t
        {
            OK = 0U,
            NOK,
            EMPTY
        };

        Q_ENUM_NS(Method)
        Q_ENUM_NS(PlotType)
        Q_ENUM_NS(SaveStatus)
    }; // namespace Enums
} // namespace sparsely
