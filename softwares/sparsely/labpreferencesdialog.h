#ifndef SPARSEPC_PREFERENCES_DIALOG_HPP
#define SPARSEPC_PREFERENCES_DIALOG_HPP

#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QJsonObject>
#include <QLineEdit>
#include <QMetaEnum>
#include <QPushButton>
#include <QSpinBox>

namespace sparsely
{
    class PreferencesDialog : public QDialog
    {
        Q_OBJECT
      public:
        PreferencesDialog(QWidget *parent = nullptr);

      private slots:
        void onMaximumNumberOfComponentsValueChanged(int value);
        void onComponentsColorsChanged(const QString &text);
        void onStandardComponentsLineWidthFilledPlotValueChanged(double value);
        void onSparseComponentsLineWidthFilledPlotValueChanged(double value);
        void
        onStandardComponentsLineWidthImpulsesPlotValueChanged(double value);
        void onSparseComponentsLineWidthImpulsesPlotValueChanged(double value);
        void onStandardComponentsLineStyleSelectionChanged(int index);
        void onSparseComponentsLineStyleSelectionChanged(int index);
        void onStandardComponentsFillingColorsAlphaValueChanged(double value);
        void onSparseComponentsFillingColorsAlphaValueChanged(double value);
        void onPlotTypeSelectionChanged(int index);
        void onMethodSelectionChanged(int index);
        void onSave(bool checked);

      private:
        QSpinBox *m_MaximumNumberOfComponentsSpinBox;
        QLineEdit *m_ComponentsColorsLineEdit;
        QDoubleSpinBox *m_StandardComponentsLineWidthFilledPlotDoubleSpinBox;
        QDoubleSpinBox *m_SparseComponentsLineWidthFilledPlotDoubleSpinBox;
        QDoubleSpinBox *m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox;
        QDoubleSpinBox *m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox;
        QComboBox *m_StandardComponentsLineStyleComboBox;
        QComboBox *m_SparseComponentsLineStyleComboBox;
        QDoubleSpinBox *m_StandardComponentsFillingColorsAlphaDoubleSpinBox;
        QDoubleSpinBox *m_SparseComponentsFillingColorsAlphaDoubleSpinBox;
        QComboBox *m_PlotTypeComboBox;
        QComboBox *m_MethodComboBox;
        QPushButton *m_SavePushButton;
    };
} // namespace sparsely
#endif // SPARSEPC_PREFERENCES_DIALOG_HPP
