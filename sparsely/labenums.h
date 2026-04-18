#ifndef SPARSEPC_ENUMS_HPP
#define SPARSEPC_ENUMS_HPP

#include <QMetaEnum>

namespace sparsely
{
    class Enums : public QObject
    {
        Q_OBJECT

      public:
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

        Q_ENUM(Method)
        Q_ENUM(PlotType)
        Q_ENUM(SaveStatus)
    };
} // namespace sparsely
#endif // SPARSEPC_ENUMS_HPP
