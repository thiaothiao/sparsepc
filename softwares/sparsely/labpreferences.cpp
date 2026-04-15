#include "labpreferences.h"

#include <QFile>
#include <QDir>
#include <QVariantList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace
{
    void saveJson(const QJsonObject &obj, const QString &fileName)
    {
        QJsonDocument doc(obj);
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return;
        }
        file.write(doc.toJson());
        file.close();
    }

    auto loadJson(const QString& filePath)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return QJsonObject{};
        }
        QByteArray rawData = file.readAll();
        file.close();
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(rawData, &error);
        if (document.isNull())
        {
            return QJsonObject{};
        }
        if (!document.isObject())
        {
            return QJsonObject{};
        }
        return document.object();
    }
}

namespace sparsely
{
    // Convert Struct to JSON
    QJsonObject Preferences::toJson() const
    {
        QJsonObject obj;
        obj["maximumNumberOfComponents"] = QString::number(maximumNumberOfComponents);
        obj["componentsColors"] = componentsColors.join(u',');
        obj["standardComponentsLineWidthFilledPlot"] =
            QString::number(standardComponentsLineWidthFilledPlot, 'f', 2);
        obj["sparseComponentsLineWidthFilledPlot"] =
            QString::number(sparseComponentsLineWidthFilledPlot, 'f', 2);
        obj["standardComponentsLineWidthImpulsesPlot"] =
            QString::number(standardComponentsLineWidthImpulsesPlot, 'f', 2);
        obj["sparseComponentsLineWidthImpulsesPlot"] =
            QString::number(sparseComponentsLineWidthImpulsesPlot, 'f', 2);
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const char* key = metaEnum.valueToKey(standardComponentsLineStyle);
            obj["standardComponentsLineStyle"] = QString::fromUtf8(key);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const char* key = metaEnum.valueToKey(sparseComponentsLineStyle);
            obj["sparseComponentsLineStyle"] = QString::fromUtf8(key);
        }
        obj["standardComponentsFillingColorsAlpha"] =
            QString::number(standardComponentsFillingColorsAlpha, 'f', 3);
        obj["sparseComponentsFillingColorsAlpha"] =
            QString::number(sparseComponentsFillingColorsAlpha, 'f', 3);
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::PlotType>();
            const char* key = metaEnum.valueToKey(std::to_underlying(plotType));
            obj["plotType"] = QString::fromUtf8(key);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::Method>();
            const char* key = metaEnum.valueToKey(std::to_underlying(method));
            obj["method"] = QString::fromUtf8(key);
        }

        return obj;
    }

    // Load Struct from JSON
    Preferences Preferences::fromJson(const QJsonObject &obj)
    {
        Preferences prefs;
        if(obj.isEmpty())
        {
            return prefs;
        }
        prefs.maximumNumberOfComponents = obj["maximumNumberOfComponents"].toString().toInt();
        prefs.componentsColors = obj["componentsColors"].toString().split(u',', Qt::SkipEmptyParts);
        prefs.standardComponentsLineWidthFilledPlot =
            obj["standardComponentsLineWidthFilledPlot"].toString().toDouble();
        prefs.sparseComponentsLineWidthFilledPlot =
            obj["sparseComponentsLineWidthFilledPlot"].toString().toDouble();
        prefs.standardComponentsLineWidthImpulsesPlot =
            obj["standardComponentsLineWidthImpulsesPlot"].toString().toDouble();
        prefs.sparseComponentsLineWidthImpulsesPlot =
            obj["sparseComponentsLineWidthImpulsesPlot"].toString().toDouble();
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const QString jsonVal = obj["standardComponentsLineStyle"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            prefs.standardComponentsLineStyle = static_cast<Qt::PenStyle>(enumVal);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const QString jsonVal = obj["sparseComponentsLineStyle"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            prefs.sparseComponentsLineStyle = static_cast<Qt::PenStyle>(enumVal);
        }
        prefs.standardComponentsFillingColorsAlpha =
            obj["standardComponentsFillingColorsAlpha"].toString().toDouble();
        prefs.sparseComponentsFillingColorsAlpha =
            obj["sparseComponentsFillingColorsAlpha"].toString().toDouble();
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::PlotType>();
            const QString jsonVal = obj["plotType"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            prefs.plotType = static_cast<Enums::PlotType>(enumVal);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::Method>();
            const QString jsonVal = obj["method"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            prefs.method = static_cast<Enums::Method>(enumVal);
        }
        return prefs;
    }

    Preferences Preferences::load(const QString& fileName)
    {
        return Preferences::fromJson(loadJson(fileName));
    }

    void Preferences::save(const QString& fileName) const
    {
        saveJson(toJson(), fileName);
    }
}
