#pragma once

#include <QJsonObject>
#include <QPen>
#include <QString>
#include <QStringList>

#include <labenums.h>

namespace sparsely
{
    struct Preferences
    {
        Preferences();

        Preferences(const Preferences &) = default;
        Preferences &operator=(const Preferences &) = default;
        Preferences(Preferences &&) = default;
        Preferences &operator=(Preferences &&) = default;
        QJsonObject toJson() const;
        static Preferences fromJson(const QJsonObject &obj);
        void save() const;
        static Preferences load();

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
} // namespace sparsely
