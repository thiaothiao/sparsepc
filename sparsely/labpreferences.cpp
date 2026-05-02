#include <labpreferences.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>
#include <QVariantList>
#include <QtLogging>

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

    auto loadJson(const QString &filePath)
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
} // namespace

namespace sparsely
{
    Preferences::Preferences()
        : maximumNumberOfComponents{6},
          componentsColors{"red", "green", "blue", "magenta", "yellow", "cyan"},
          standardComponentsLineWidthFilledPlot{1.0},
          sparseComponentsLineWidthFilledPlot{2.0},
          standardComponentsLineWidthImpulsesPlot{2.0},
          sparseComponentsLineWidthImpulsesPlot{4.0},
          standardComponentsLineStyle{Qt::SolidLine},
          sparseComponentsLineStyle{Qt::DotLine},
          standardComponentsFillingColorsAlpha{0.125},
          sparseComponentsFillingColorsAlpha{0.25},
          plotType{Enums::PlotType::FILLED}, method{Enums::Method::DCA},
          addonsPath{tr("Addons path here")}
    {
    }

    // Convert Struct to JSON
    QJsonObject Preferences::toJson() const
    {
        QJsonObject obj;
        obj["maximumNumberOfComponents"] =
            QString::number(maximumNumberOfComponents);
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
            const char *key = metaEnum.valueToKey(standardComponentsLineStyle);
            obj["standardComponentsLineStyle"] = QString::fromUtf8(key);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const char *key = metaEnum.valueToKey(sparseComponentsLineStyle);
            obj["sparseComponentsLineStyle"] = QString::fromUtf8(key);
        }
        obj["standardComponentsFillingColorsAlpha"] =
            QString::number(standardComponentsFillingColorsAlpha, 'f', 3);
        obj["sparseComponentsFillingColorsAlpha"] =
            QString::number(sparseComponentsFillingColorsAlpha, 'f', 3);
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::PlotType>();
            const char *key = metaEnum.valueToKey(std::to_underlying(plotType));
            obj["plotType"] = QString::fromUtf8(key);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::Method>();
            const char *key = metaEnum.valueToKey(std::to_underlying(method));
            obj["method"] = QString::fromUtf8(key);
        }

        obj["addonsPath"] = addonsPath;

        return obj;
    }

    // Load Struct from JSON
    void Preferences::fromJson(const QJsonObject &obj)
    {
        if (obj.isEmpty())
        {
            return;
        }
        maximumNumberOfComponents =
            obj["maximumNumberOfComponents"].toString().toInt();
        componentsColors =
            obj["componentsColors"].toString().split(u',', Qt::SkipEmptyParts);
        standardComponentsLineWidthFilledPlot =
            obj["standardComponentsLineWidthFilledPlot"].toString().toDouble();
        sparseComponentsLineWidthFilledPlot =
            obj["sparseComponentsLineWidthFilledPlot"].toString().toDouble();
        standardComponentsLineWidthImpulsesPlot =
            obj["standardComponentsLineWidthImpulsesPlot"]
                .toString()
                .toDouble();
        sparseComponentsLineWidthImpulsesPlot =
            obj["sparseComponentsLineWidthImpulsesPlot"].toString().toDouble();
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const QString jsonVal =
                obj["standardComponentsLineStyle"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            standardComponentsLineStyle = static_cast<Qt::PenStyle>(enumVal);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Qt::PenStyle>();
            const QString jsonVal = obj["sparseComponentsLineStyle"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            sparseComponentsLineStyle = static_cast<Qt::PenStyle>(enumVal);
        }
        standardComponentsFillingColorsAlpha =
            obj["standardComponentsFillingColorsAlpha"].toString().toDouble();
        sparseComponentsFillingColorsAlpha =
            obj["sparseComponentsFillingColorsAlpha"].toString().toDouble();
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::PlotType>();
            const QString jsonVal = obj["plotType"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            plotType = static_cast<Enums::PlotType>(enumVal);
        }
        {
            const QMetaEnum metaEnum = QMetaEnum::fromType<Enums::Method>();
            const QString jsonVal = obj["method"].toString();
            const int enumVal = metaEnum.keyToValue(jsonVal.toUtf8().data());
            method = static_cast<Enums::Method>(enumVal);
        }
        addonsPath = obj["addonsPath"].toString();
    }

    void Preferences::load()
    {
        const auto dataPath =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

        if (!QDir().mkpath(dataPath))
        { // critical
            return;
        }

        QDir dir(dataPath);
        if (!dir.exists("configurations"))
        {
            if (!dir.mkdir("configurations"))
            { // critical
                return;
            }
        }

        if (!dir.cd("configurations"))
        { // critical
            return;
        }

        if (!dir.exists("sparsely.json"))
        { // use default preferences
            return;
        }

        fromJson(loadJson(dir.absoluteFilePath("sparsely.json")));
        qDebug() << "Preferences loaded";
    }

    void Preferences::save() const
    {
        const auto dataPath =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

        if (!QDir().mkpath(dataPath))
        { // critical
            return;
        }

        QDir dir(dataPath);
        if (!dir.exists("configurations"))
        {
            if (!dir.mkdir("configurations"))
            { // critical
                return;
            }
        }

        if (!dir.cd("configurations"))
        { // critical
            return;
        }

        saveJson(toJson(), dir.absoluteFilePath("sparsely.json"));
        qDebug() << "Preferences saved";
    }
} // namespace sparsely
