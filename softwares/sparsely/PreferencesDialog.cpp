#include "PreferencesDialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent)
{
    QWidget *container = new QWidget(this);
    QVBoxLayout* containerLayout = new QVBoxLayout(container);

    auto* maximumNumberOfComponentsGroupBox = new QGroupBox("Maximum number of pcs", this);
    auto* maximumNumberOfComponentsGroupBoxLayout =
        new QVBoxLayout(maximumNumberOfComponentsGroupBox);

    m_MaximumNumberOfComponentsSpinBox = new QSpinBox(this);
    maximumNumberOfComponentsGroupBoxLayout->addWidget(m_MaximumNumberOfComponentsSpinBox);
    m_MaximumNumberOfComponentsSpinBox->setRange(2, 6);
    m_MaximumNumberOfComponentsSpinBox->setValue(4);

    containerLayout->addWidget(maximumNumberOfComponentsGroupBox);

    QObject::connect(m_MaximumNumberOfComponentsSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            this, &PreferencesDialog::onMaximumNumberOfComponentsValueChanged);

    auto* componentsColorsGroupBox = new QGroupBox("Components colors", this);
    auto* componentsColorsGroupBoxLayout = new QVBoxLayout(componentsColorsGroupBox);

    m_ComponentsColorsLineEdit = new QLineEdit(this);
    componentsColorsGroupBoxLayout->addWidget(m_ComponentsColorsLineEdit);

    containerLayout->addWidget(componentsColorsGroupBox);

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
    m_StandardComponentsLineWidthFilledPlotDoubleSpinBox->setValue(1.0);

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
    m_SparseComponentsLineWidthFilledPlotDoubleSpinBox->setValue(2.0);

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
    m_StandardComponentsLineWidthImpulsesPlotDoubleSpinBox->setValue(2.0);

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
    m_SparseComponentsLineWidthImpulsesPlotDoubleSpinBox->setValue(4.0);

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
    m_StandardComponentsFillingColorsAlphaDoubleSpinBox->setValue(0.125);

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
    m_SparseComponentsFillingColorsAlphaDoubleSpinBox->setValue(0.250);

    sparseComponentsFillingColorsAlphaGroupBoxLayout->addWidget(
        m_SparseComponentsFillingColorsAlphaDoubleSpinBox);

    containerLayout->addWidget(sparseComponentsFillingColorsAlphaGroupBox);

    QObject::connect(m_SparseComponentsFillingColorsAlphaDoubleSpinBox,
         qOverload<double>(&QDoubleSpinBox::valueChanged), this,
         &PreferencesDialog::onSparseComponentsFillingColorsAlphaValueChanged);

    auto* plotTypeGroupBox = new QGroupBox("Plot", this);
    auto* plotTypeGroupBoxLayout = new QVBoxLayout(plotTypeGroupBox);

    m_PlotTypeComboBox = new QComboBox(this) ;
    m_PlotTypeComboBox->addItem("Filled", QVariant::fromValue(PlotType::FILLED));
    m_PlotTypeComboBox->addItem("Impulses", QVariant::fromValue(PlotType::IMPULSES));

    plotTypeGroupBoxLayout->addWidget(m_PlotTypeComboBox);

    containerLayout->addWidget(plotTypeGroupBox);

    QObject::connect(m_PlotTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &PreferencesDialog::onPlotTypeSelectionChanged);

    auto* spcaMethodGroupBox = new QGroupBox("Method", this);
    auto* spcaMethodGroupBoxLayout = new QVBoxLayout(spcaMethodGroupBox);

    m_SpcaMethodComboBox = new QComboBox(this) ;
    m_SpcaMethodComboBox->addItem("Dca", QVariant::fromValue(SpcaMethod::DCA));
    m_SpcaMethodComboBox->addItem("Forward Gspca", QVariant::fromValue(SpcaMethod::FGSPCA));
    m_SpcaMethodComboBox->addItem("Backward Gspca", QVariant::fromValue(SpcaMethod::BGSPCA));
    m_SpcaMethodComboBox->addItem("Custom", QVariant::fromValue(SpcaMethod::CUSTOM));
    m_SpcaMethodComboBox->addItem("User Dynamic Lib", QVariant::fromValue(SpcaMethod::USERDYNAMICLIB));

    spcaMethodGroupBoxLayout->addWidget(m_SpcaMethodComboBox);

    containerLayout->addWidget(spcaMethodGroupBox);

    QObject::connect(m_SpcaMethodComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                     this, &PreferencesDialog::onSpcaMethodSelectionChanged);

    QPushButton *btnOK = new QPushButton("OK", this);
    containerLayout->addWidget(btnOK);
    QObject::connect(btnOK, &QPushButton::clicked, this, &QDialog::accept);

    auto* layout = new QVBoxLayout(this);

    auto* scrollArea = new QScrollArea(this);

    scrollArea->setWidget(container);

    scrollArea->setWidgetResizable(true);

    layout->addWidget(scrollArea);
}

void PreferencesDialog::onMaximumNumberOfComponentsValueChanged(int value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onComponentsColorsChanged(const QString &text)
{
   qDebug() << "Current text:" << text;
}

void PreferencesDialog::onStandardComponentsLineWidthFilledPlotValueChanged(double value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onSparseComponentsLineWidthFilledPlotValueChanged(double value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onStandardComponentsLineWidthImpulsesPlotValueChanged(double value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onSparseComponentsLineWidthImpulsesPlotValueChanged(double value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onStandardComponentsLineStyleSelectionChanged(int index)
{
    qDebug() << "Current index:" << QString::number(index);
}

void PreferencesDialog::onSparseComponentsLineStyleSelectionChanged(int index)
{
    qDebug() << "Current index:" << QString::number(index);
}

void PreferencesDialog::onStandardComponentsFillingColorsAlphaValueChanged(double value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onSparseComponentsFillingColorsAlphaValueChanged(double value)
{
    qDebug() << "Current value:" << QString::number(value);
}

void PreferencesDialog::onPlotTypeSelectionChanged(int index)
{
    qDebug() << "Current index:" << QString::number(index);
}

void PreferencesDialog::onSpcaMethodSelectionChanged(int index)
{
    qDebug() << "Current index:" << QString::number(index);
}
