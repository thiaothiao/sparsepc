#include "preferencesdialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QFile>
#include <QDir>
#include <QVariantList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "preferences.h"

namespace sparsely
{
    PreferencesDialog::PreferencesDialog(QWidget *parent) :
        QDialog(parent)
    {
        // must load preferense json file
        Preferences prefs = Preferences::load(
            QDir(QDir::currentPath() + "/configurations").absoluteFilePath("sparsely.json"));

        QWidget *container = new QWidget(this);
        QVBoxLayout* containerLayout = new QVBoxLayout(container);

        auto* maximumNumberOfComponentsGroupBox = new QGroupBox("Maximum number of pcs", this);
        auto* maximumNumberOfComponentsGroupBoxLayout =
            new QVBoxLayout(maximumNumberOfComponentsGroupBox);

        m_MaximumNumberOfComponentsSpinBox = new QSpinBox(this);
        maximumNumberOfComponentsGroupBoxLayout->addWidget(m_MaximumNumberOfComponentsSpinBox);
        m_MaximumNumberOfComponentsSpinBox->setRange(2, 6);
        m_MaximumNumberOfComponentsSpinBox->setValue(prefs.maximumNumberOfComponents);

        containerLayout->addWidget(maximumNumberOfComponentsGroupBox);

        QObject::connect(m_MaximumNumberOfComponentsSpinBox, qOverload<int>(&QSpinBox::valueChanged),
                this, &PreferencesDialog::onMaximumNumberOfComponentsValueChanged);

        auto* componentsColorsGroupBox = new QGroupBox("Components colors", this);
        auto* componentsColorsGroupBoxLayout = new QVBoxLayout(componentsColorsGroupBox);

        m_ComponentsColorsLineEdit = new QLineEdit(this);
        componentsColorsGroupBoxLayout->addWidget(m_ComponentsColorsLineEdit);

        containerLayout->addWidget(componentsColorsGroupBox);

        m_ComponentsColorsLineEdit->setText(prefs.componentsColors.join(u','));

        QObject::connect(m_ComponentsColorsLineEdit, &QLineEdit::textChanged,
                this, &PreferencesDialog::onComponentsColorsChanged);

        auto* standardComponentsLineWidthFilledPlotGroupBox =
            new QGroupBox("Standard pcs filled plot line width", this);
        auto* standardComponentsLineWidthFilledPlotGroupBoxLayout =
            new QVBoxLayout(standardComponentsLineWidthFilledPlotGroupBox);

        m_StandardComponentsLineWidthFilledPlotDoubleSpinBox = new QDoubleSpinBox(this);
        standardComponentsLineWidthFilledPlotGroupBoxLayout->addWidget(
            m_StandardComponentsLineWidthFilledPlotDoubleSpinBox);
        m_StandardComponentsLineWidthFilledPlotDoubleSpinBox->setRange(1.0, 10.0);
        m_StandardComponentsLineWidthFilledPlotDoubleSpinBox->setValue(
            prefs.standardComponentsLineWidthFilledPlot);

        containerLayout->addWidget(standardComponentsLineWidthFilledPlotGroupBox);

        QObject::connect(m_StandardComponentsLineWidthFilledPlotDoubleSpinBox,
                qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                &PreferencesDialog::onStandardComponentsLineWidthFilledPlotValueChanged);

        auto* sparseComponentsLineWidthFilledPlotGroupBox =
            new QGroupBox("Sparse pcs filled plot line width", this);
        auto* sparseComponentsLineWidthFilledPlotGroupBoxLayout =
            new QVBoxLayout(sparseComponentsLineWidthFilledPlotGroupBox);

        m_SparseComponentsLineWidthFilledPlotDoubleSpinBox = new QDoubleSpinBox(this);
        sparseComponentsLineWidthFilledPlotGroupBoxLayout->addWidget(
            m_SparseComponentsLineWidthFilledPlotDoubleSpinBox);
        m_SparseComponentsLineWidthFilledPlotDoubleSpinBox->setRange(1.0, 10.0);
        m_SparseComponentsLineWidthFilledPlotDoubleSpinBox->setValue(
            prefs.sparseComponentsLineWidthFilledPlot);

        containerLayout->addWidget(sparseComponentsLineWidthFilledPlotGroupBox);

        QObject::connect(m_SparseComponentsLineWidthFilledPlotDoubleSpinBox,
                qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                &PreferencesDialog::onSparseComponentsLineWidthFilledPlotValueChanged);

        auto* standardComponentsLineWidthImpulsesPlotGroupBox =
            new QGroupBox("Standard pcs impulses plot line width", this);
        auto* standardComponentsLineWidthImpulsesPlotGroupBoxLayout =
            new QVBoxLayout(standardComponentsLineWidthImpulsesPlotGroupBox);

        m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox = new QDoubleSpinBox(this);
        standardComponentsLineWidthImpulsesPlotGroupBoxLayout->addWidget(
            m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox);
        m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox->setRange(1.0, 10.0);
        m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox->setValue(
            prefs.standardComponentsLineWidthImpulsesPlot);

        containerLayout->addWidget(standardComponentsLineWidthImpulsesPlotGroupBox);

        QObject::connect(m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox,
                         qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                         &PreferencesDialog::onStandardComponentsLineWidthImpulsesPlotValueChanged);

        auto* sparseComponentsLineWidthImpulsesPlotGroupBox =
            new QGroupBox("Sparse pcs impulses plot line width", this);
        auto* sparseComponentsLineWidthImpulsesPlotGroupBoxLayout =
            new QVBoxLayout(sparseComponentsLineWidthImpulsesPlotGroupBox);

        m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox = new QDoubleSpinBox(this);
        sparseComponentsLineWidthImpulsesPlotGroupBoxLayout->addWidget(
            m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox);

        m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox->setRange(1.0, 10.0);
        m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox->setValue(
            prefs.sparseComponentsLineWidthImpulsesPlot);

        containerLayout->addWidget(sparseComponentsLineWidthImpulsesPlotGroupBox);

        QObject::connect(m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox,
                         qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                         &PreferencesDialog::onSparseComponentsLineWidthImpulsesPlotValueChanged);

        auto* standardComponentsLineStyleGroupBox =
            new QGroupBox("Standard pcs line style", this);
        auto* standardComponentsLineStyleGroupBoxLayout =
            new QVBoxLayout(standardComponentsLineStyleGroupBox);

        m_StandardComponentsLineStyleComboBox = new QComboBox(this) ;
        m_StandardComponentsLineStyleComboBox->addItem("Solid", QVariant::fromValue(Qt::SolidLine));
        m_StandardComponentsLineStyleComboBox->addItem("Dot", QVariant::fromValue(Qt::DotLine));
        m_StandardComponentsLineStyleComboBox->setCurrentIndex(
            (prefs.standardComponentsLineStyle == Qt::SolidLine) ? 0 : 1);

        standardComponentsLineStyleGroupBoxLayout->addWidget(
            m_StandardComponentsLineStyleComboBox);

        containerLayout->addWidget(standardComponentsLineStyleGroupBox);

        QObject::connect(m_StandardComponentsLineStyleComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                         this, &PreferencesDialog::onStandardComponentsLineStyleSelectionChanged);

        auto* sparseComponentsLineStyleGroupBox =
            new QGroupBox("Sparse pcs line style", this);
        auto* sparseComponentsLineStyleGroupBoxLayout =
            new QVBoxLayout(sparseComponentsLineStyleGroupBox);

        m_SparseComponentsLineStyleComboBox = new QComboBox(this) ;
        m_SparseComponentsLineStyleComboBox->addItem("Solid", QVariant::fromValue(Qt::SolidLine));
        m_SparseComponentsLineStyleComboBox->addItem("Dot", QVariant::fromValue(Qt::DotLine));

        m_SparseComponentsLineStyleComboBox->setCurrentIndex(
            (prefs.sparseComponentsLineStyle == Qt::SolidLine) ? 0 : 1);

        sparseComponentsLineStyleGroupBoxLayout->addWidget(
            m_SparseComponentsLineStyleComboBox);

        containerLayout->addWidget(sparseComponentsLineStyleGroupBox);

        QObject::connect(m_SparseComponentsLineStyleComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                         this, &PreferencesDialog::onSparseComponentsLineStyleSelectionChanged);

        auto* standardComponentsFillingColorsAlphaGroupBox =
            new QGroupBox("Standard pcs filling colors alpha", this);
        auto* standardComponentsFillingColorsAlphaGroupBoxLayout =
            new QVBoxLayout(standardComponentsFillingColorsAlphaGroupBox);

        m_StandardComponentsFillingColorsAlphaDoubleSpinBox = new QDoubleSpinBox(this);
        m_StandardComponentsFillingColorsAlphaDoubleSpinBox->setDecimals(3);
        m_StandardComponentsFillingColorsAlphaDoubleSpinBox->setRange(0.0, 1.0);
        m_StandardComponentsFillingColorsAlphaDoubleSpinBox->setValue(
            prefs.standardComponentsFillingColorsAlpha);

        standardComponentsFillingColorsAlphaGroupBoxLayout->addWidget(
            m_StandardComponentsFillingColorsAlphaDoubleSpinBox);

        containerLayout->addWidget(standardComponentsFillingColorsAlphaGroupBox);

        QObject::connect(m_StandardComponentsFillingColorsAlphaDoubleSpinBox,
             qOverload<double>(&QDoubleSpinBox::valueChanged), this,
             &PreferencesDialog::onStandardComponentsFillingColorsAlphaValueChanged);

        auto* sparseComponentsFillingColorsAlphaGroupBox =
            new QGroupBox("Sparse pcs filling colors alpha", this);
        auto* sparseComponentsFillingColorsAlphaGroupBoxLayout =
            new QVBoxLayout(sparseComponentsFillingColorsAlphaGroupBox);

        m_SparseComponentsFillingColorsAlphaDoubleSpinBox = new QDoubleSpinBox(this);
        m_SparseComponentsFillingColorsAlphaDoubleSpinBox->setDecimals(3);
        m_SparseComponentsFillingColorsAlphaDoubleSpinBox->setRange(0.0, 1.0);
        m_SparseComponentsFillingColorsAlphaDoubleSpinBox->setValue(
            prefs.sparseComponentsFillingColorsAlpha);

        sparseComponentsFillingColorsAlphaGroupBoxLayout->addWidget(
            m_SparseComponentsFillingColorsAlphaDoubleSpinBox);

        containerLayout->addWidget(sparseComponentsFillingColorsAlphaGroupBox);

        QObject::connect(m_SparseComponentsFillingColorsAlphaDoubleSpinBox,
             qOverload<double>(&QDoubleSpinBox::valueChanged), this,
             &PreferencesDialog::onSparseComponentsFillingColorsAlphaValueChanged);

        auto* plotTypeGroupBox = new QGroupBox("Plot", this);
        auto* plotTypeGroupBoxLayout = new QVBoxLayout(plotTypeGroupBox);

        m_PlotTypeComboBox = new QComboBox(this) ;
        m_PlotTypeComboBox->addItem("Filled", QVariant::fromValue(Enums::PlotType::FILLED));
        m_PlotTypeComboBox->addItem("Impulses", QVariant::fromValue(Enums::PlotType::IMPULSES));
        m_PlotTypeComboBox->setCurrentIndex(prefs.plotType == Enums::PlotType::FILLED ? 0 : 1);

        plotTypeGroupBoxLayout->addWidget(m_PlotTypeComboBox);

        containerLayout->addWidget(plotTypeGroupBox);

        QObject::connect(m_PlotTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &PreferencesDialog::onPlotTypeSelectionChanged);

        auto* methodGroupBox = new QGroupBox("Method", this);
        auto* methodGroupBoxLayout = new QVBoxLayout(methodGroupBox);

        m_MethodComboBox = new QComboBox(this) ;
        m_MethodComboBox->addItem("Dca", QVariant::fromValue(Enums::Method::DCA));
        m_MethodComboBox->addItem("Forward Gspca", QVariant::fromValue(Enums::Method::FGSPCA));
        m_MethodComboBox->addItem("Backward Gspca", QVariant::fromValue(Enums::Method::BGSPCA));
        m_MethodComboBox->addItem("Custom", QVariant::fromValue(Enums::Method::CUSTOM));
        m_MethodComboBox->addItem("User Dynamic Lib", QVariant::fromValue(Enums::Method::USERDYNAMICLIB));

        switch (prefs.method)
        {// should use a map type ontainer!
        case Enums::Method::DCA:
        {
            m_MethodComboBox->setCurrentIndex(0);
            break;
        }
        case Enums::Method::FGSPCA:
        {
            m_MethodComboBox->setCurrentIndex(1);
            break;
        }
        case Enums::Method::BGSPCA:
        {
            m_MethodComboBox->setCurrentIndex(2);
            break;
        }
        case Enums::Method::CUSTOM:
        {
            m_MethodComboBox->setCurrentIndex(3);
            break;
        }
        case Enums::Method::USERDYNAMICLIB:
        {
            m_MethodComboBox->setCurrentIndex(4);
            break;
        }
        default:
        {
            m_MethodComboBox->setCurrentIndex(0);
            break;
        }
        }

        methodGroupBoxLayout->addWidget(m_MethodComboBox);

        containerLayout->addWidget(methodGroupBox);

        QObject::connect(m_MethodComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                         this, &PreferencesDialog::onMethodSelectionChanged);

        m_SavePushButton = new QPushButton("Save", this);
        containerLayout->addWidget(m_SavePushButton);
        m_SavePushButton->setDisabled(true);
        QObject::connect(m_SavePushButton, &QPushButton::clicked, this, &PreferencesDialog::onSave);

        auto* layout = new QVBoxLayout(this);

        auto* scrollArea = new QScrollArea(this);

        scrollArea->setWidget(container);

        scrollArea->setWidgetResizable(true);

        layout->addWidget(scrollArea);
    }

    void PreferencesDialog::onMaximumNumberOfComponentsValueChanged(int value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onComponentsColorsChanged(const QString &text)
    {
       qDebug() << "Current text:" << text;
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onStandardComponentsLineWidthFilledPlotValueChanged(double value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onSparseComponentsLineWidthFilledPlotValueChanged(double value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onStandardComponentsLineWidthImpulsesPlotValueChanged(double value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onSparseComponentsLineWidthImpulsesPlotValueChanged(double value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onStandardComponentsLineStyleSelectionChanged(int index)
    {
        qDebug() << "Current index:" << QString::number(index);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onSparseComponentsLineStyleSelectionChanged(int index)
    {
        qDebug() << "Current index:" << QString::number(index);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onStandardComponentsFillingColorsAlphaValueChanged(double value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onSparseComponentsFillingColorsAlphaValueChanged(double value)
    {
        qDebug() << "Current value:" << QString::number(value);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onPlotTypeSelectionChanged(int index)
    {
        qDebug() << "Current index:" << QString::number(index);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onMethodSelectionChanged(int index)
    {
        qDebug() << "Current index:" << QString::number(index);
        m_SavePushButton->setEnabled(true);
    }

    void PreferencesDialog::onSave(bool checked)
    {
        qDebug() << "On save";
        Preferences prefs;

        prefs.maximumNumberOfComponents =
            m_MaximumNumberOfComponentsSpinBox->value();
        prefs.componentsColors =
            m_ComponentsColorsLineEdit->text().split(u',', Qt::SkipEmptyParts);
        prefs.standardComponentsLineWidthFilledPlot =
            m_StandardComponentsLineWidthFilledPlotDoubleSpinBox->value();
        prefs.sparseComponentsLineWidthFilledPlot =
            m_SparseComponentsLineWidthFilledPlotDoubleSpinBox->value();
        prefs.standardComponentsLineWidthImpulsesPlot =
            m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox->value();
        prefs.sparseComponentsLineWidthImpulsesPlot =
            m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox->value();
        prefs.standardComponentsLineStyle =
            m_StandardComponentsLineStyleComboBox->currentData().value<Qt::PenStyle>();
        prefs.sparseComponentsLineStyle =
            m_SparseComponentsLineStyleComboBox->currentData().value<Qt::PenStyle>();
        prefs.standardComponentsFillingColorsAlpha =
            m_StandardComponentsFillingColorsAlphaDoubleSpinBox->value();
        prefs.sparseComponentsFillingColorsAlpha =
            m_SparseComponentsFillingColorsAlphaDoubleSpinBox->value();
        prefs.plotType = m_PlotTypeComboBox->currentData().value<Enums::PlotType>();
        prefs.method = m_MethodComboBox->currentData().value<Enums::Method>();

        prefs.save(QDir(QDir::currentPath() + "/configurations").absoluteFilePath("sparsely.json"));

        QDialog::accept();
    }
}
