#ifndef SPARSEPC_PREFERENCES_DIALOG_HPP
#define SPARSEPC_PREFERENCES_DIALOG_HPP

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QJsonObject>
#include <QMetaEnum>

namespace sparsepc
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

    class PreferencesDialog : public QDialog
    {
        Q_OBJECT
    public:
        PreferencesDialog(QWidget *parent = nullptr);

    public slots:
        void onMaximumNumberOfComponentsValueChanged(int value);
        void onComponentsColorsChanged(const QString &text);
        void onStandardComponentsLineWidthFilledPlotValueChanged(double value);
        void onSparseComponentsLineWidthFilledPlotValueChanged(double value);
        void onStandardComponentsLineWidthImpulsesPlotValueChanged(double value);
        void onSparseComponentsLineWidthImpulsesPlotValueChanged(double value);
        void onStandardComponentsLineStyleSelectionChanged(int index);
        void onSparseComponentsLineStyleSelectionChanged(int index);
        void onStandardComponentsFillingColorsAlphaValueChanged(double value);
        void onSparseComponentsFillingColorsAlphaValueChanged(double value);
        void onPlotTypeSelectionChanged(int index);
        void onMethodSelectionChanged(int index);
        void onSave(bool checked);

    private:
        QSpinBox* m_MaximumNumberOfComponentsSpinBox;
        QLineEdit* m_ComponentsColorsLineEdit;
        QDoubleSpinBox* m_StandardComponentsLineWidthFilledPlotDoubleSpinBox;
        QDoubleSpinBox* m_SparseComponentsLineWidthFilledPlotDoubleSpinBox;
        QDoubleSpinBox* m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox;
        QDoubleSpinBox* m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox;
        QComboBox* m_StandardComponentsLineStyleComboBox;
        QComboBox* m_SparseComponentsLineStyleComboBox;
        QDoubleSpinBox* m_StandardComponentsFillingColorsAlphaDoubleSpinBox;
        QDoubleSpinBox* m_SparseComponentsFillingColorsAlphaDoubleSpinBox;
        QComboBox* m_PlotTypeComboBox;
        QComboBox* m_MethodComboBox;
        QPushButton* m_SavePushButton;
    };
}
#endif //SPARSEPC_PREFERENCES_DIALOG_HPP
