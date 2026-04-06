#ifndef SPARSEPC_PREFERENCES_DIALOG_HPP
#define SPARSEPC_PREFERENCES_DIALOG_HPP

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>

enum class SpcaMethod : std::uint8_t
{
    DCA = 0U,
    FGSPCA,  //ForwardGSPCA
    BGSPCA,   // BackwardGSPA
    CUSTOM,
    USERDYNAMICLIB
};

enum class PlotType : std::uint8_t
{
    FILLED = 0U,
    IMPULSES
};

Q_DECLARE_METATYPE(SpcaMethod)
Q_DECLARE_METATYPE(PlotType)

struct Preferences
{
    int maximumNumberOfComponents = 6;
    QStringList componentsColors = {"red", "green", "blue", "magenta", "yellow", "cyan"};
    double standardComponentsLineWidthFilledPlot = 1.0;
    double sparseComponentsLineWidthFilledPlot = 2.0;
    double standardComponentsLineWidthImpulsesPlot = 2.0;
    double sparseComponentsLineWidthImpulsesPlot = 4.0;
    Qt::PenStyle standardComponentsLineStyle = Qt::SolidLine;
    Qt::PenStyle sparseComponentsLineStyle = Qt::DotLine;
    double standardComponentsFillingColorsAlpha = 0.125;
    double sparseComponentsFillingColorsAlpha = 0.25;
    PlotType plotType = PlotType::FILLED;
    SpcaMethod spcaMethod = SpcaMethod::DCA;
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
    void onSpcaMethodSelectionChanged(int index);

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
    QComboBox* m_SpcaMethodComboBox;
};

#endif //SPARSEPC_PREFERENCES_DIALOG_HPP
