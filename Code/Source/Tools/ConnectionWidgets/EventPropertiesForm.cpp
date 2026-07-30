#include "EventPropertiesForm.h"
#include "ui_EventPropertiesForm.h"

#include <AzQtComponents/Components/Widgets/CheckBox.h>
#include <AzQtComponents/Components/Widgets/ComboBox.h>
#include <AzQtComponents/Components/Widgets/SpinBox.h>

namespace AudioEngineFMOD {
EventPropertiesForm::EventPropertiesForm(AudioControls::TConnectionPtr connection, AudioControls::TImplControlType ctrlType, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventPropertiesForm),
    m_connection(connection),
    m_controlType(ctrlType)
{
    ui->setupUi(this);

    AzQtComponents::CheckBox::applyToggleSwitchStyle(ui->checkBox);
    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->comboBox);
    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->comboBox_2);
    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->comboBox_3);
}

EventPropertiesForm::~EventPropertiesForm()
{
    delete ui;
}

}