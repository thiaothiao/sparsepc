#pragma once

#include <QMetaEnum>

namespace sparsely
{
    namespace Enums
    {
        Q_NAMESPACE

        // Sparse principal component methods enumeration
        enum class Method : std::uint8_t
        {
            DCA = 0U,      // Dca based
            FGSPCA,        // Gspca based, forward
            BGSPCA,        // Gspca based, backward
            CUSTOM,        // statically customizable user defined method
            USERDYNAMICLIB // addon based user defined method
        };

        // Graphs plot types enumeration
        enum class PlotType : std::uint8_t
        {
            FILLED = 0U, // filled plots
            IMPULSES     // impulse plots
        };

        Q_ENUM_NS(Method)
        Q_ENUM_NS(PlotType)
    }; // namespace Enums
} // namespace sparsely
