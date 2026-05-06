#pragma once

#include <QCoreApplication>
#include <QJsonObject>
#include <QPen>
#include <QString>
#include <QStringList>

#include <labenums.h>

namespace Sparsely
{
    struct Preferences
    {
        Preferences();
        Preferences(const Preferences &) = default;
        Preferences &operator=(const Preferences &) = default;
        Preferences(Preferences &&) = default;
        Preferences &operator=(Preferences &&) = default;
        // return preferences values as a json object
        QJsonObject toJson() const;
        // update preferences values from a json object
        void fromJson(const QJsonObject &obj);
        // save the preferences values to the dedicated file
        void save() const;
        // load the preferences values from the dedicated file
        void load();
        // maximum number of components allowed
        int maximumNumberOfComponents;
        // list of colors to be used at each principal component round
        QStringList componentsColors;
        // standard components filled plot line width
        double standardComponentsLineWidthFilledPlot;
        // sparse components filled plot line width
        double sparseComponentsLineWidthFilledPlot;
        // standard components impulses plot line width
        double standardComponentsLineWidthImpulsesPlot;
        // sparse components impulses plot line width
        double sparseComponentsLineWidthImpulsesPlot;
        // standard components plot line style
        Qt::PenStyle standardComponentsLineStyle;
        // sparse components plot line style
        Qt::PenStyle sparseComponentsLineStyle;
        // standard components plot filled part color alpha
        double standardComponentsFillingColorsAlpha;
        // sparse components plot filled part color alpha
        double sparseComponentsFillingColorsAlpha;
        // graphs plot type enumerate value
        Enums::PlotType plotType;
        // sparse principal component method enumerate value
        Enums::Method method;
        // path to an eventual addon folder
        QString addonsPath;

        Q_DECLARE_TR_FUNCTIONS(Preferences)
    };
} // namespace Sparsely
