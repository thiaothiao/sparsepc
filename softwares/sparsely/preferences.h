#ifndef SPARSEPC_PREFERENCES_HPP
#define SPARSEPC_PREFERENCES_HPP

#include <QPushButton>
#include <QJsonObject>
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

        Q_ENUM(Method)
        Q_ENUM(PlotType)
    };

    struct Preferences
    {
        Preferences():
            maximumNumberOfComponents{6},
            componentsColors{"red", "green", "blue", "magenta", "yellow", "cyan"},
            standardComponentsLineWidthFilledPlot{ 1.0 },
            sparseComponentsLineWidthFilledPlot{ 2.0 },
            standardComponentsLineWidthImpulsesPlot{ 2.0 },
            sparseComponentsLineWidthImpulsesPlot{ 4.0 },
            standardComponentsLineStyle{ Qt::SolidLine },
            sparseComponentsLineStyle{ Qt::DotLine },
            standardComponentsFillingColorsAlpha{ 0.125 },
            sparseComponentsFillingColorsAlpha{ 0.25 },
            plotType{ Enums::PlotType::FILLED },
            method{ Enums::Method::DCA }
        {}

        Preferences(const Preferences&) = default;
        Preferences& operator=(const Preferences&) = default;

        Preferences(Preferences&&) = default;
        Preferences& operator=(Preferences&&) = default;

        QJsonObject toJson() const;

        static Preferences fromJson(const QJsonObject& obj);

        void save(const QString& fileName);

        static Preferences load(const QString& fileName);

        int maximumNumberOfComponents;
        QStringList componentsColors;
        double standardComponentsLineWidthFilledPlot;
        double sparseComponentsLineWidthFilledPlot;
        double standardComponentsLineWidthImpulsesPlot;
        double sparseComponentsLineWidthImpulsesPlot;
        Qt::PenStyle standardComponentsLineStyle;
        Qt::PenStyle sparseComponentsLineStyle;
        double standardComponentsFillingColorsAlpha;
        double sparseComponentsFillingColorsAlpha;
        Enums::PlotType plotType;
        Enums::Method method;
    };
}
#endif //SPARSEPC_PREFERENCES_HPP
