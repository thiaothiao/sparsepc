#pragma once

#include <QMetaEnum>

namespace Sparsely
{
    namespace Enums
    {
        Q_NAMESPACE

        // Sparse principal component methods enumeration
        enum class Method : std::uint8_t
        {
            DCA = 0U,          // Dca based
            FGSPCA,            // Gspca based, forward
            BGSPCA,            // Gspca based, backward
            CUSTOM,            // Statically customizable user defined method
            CONTIGUOUSSUPPORT, // Contiguous support finder
            USERDYNAMICLIB     // addon based user defined method
        };

        // Graphs plot types enumeration
        enum class PlotType : std::uint8_t
        {
            FILLED = 0U, // filled plots
            IMPULSES     // impulse plots
        };

        // Data types
        enum class ScaleType : std::uint8_t
        {
            GENERAL = 0U, // general data analysis
            ANNUAL,       // annual data analysis
            SMALL,
            MEDIUM,
            LARGE,
            VERYLARGE
        };

        Q_ENUM_NS(Method)
        Q_ENUM_NS(PlotType)
        Q_ENUM_NS(ScaleType)
    }; // namespace Enums
} // namespace Sparsely
