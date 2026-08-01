#include "EventPropertiesForm.h"
#include "ui_EventPropertiesForm.h"

#include <AzQtComponents/Components/Widgets/CheckBox.h>
#include <AzQtComponents/Components/Widgets/ComboBox.h>
#include <AzQtComponents/Components/Widgets/SpinBox.h>

#include "../AudioConnections.h"
#include "../../Clients/Engine/Common_FMOD.h"

namespace AudioEngineFMOD {

void SetComboboxIndexFromAction(FMODEventAction action, QComboBox* cb)
{
    switch(action) {
    case FMODEventAction::Play:
        cb->setCurrentIndex(0);
        break;
    case FMODEventAction::Pause:
        cb->setCurrentIndex(1);
        break;
    case FMODEventAction::Resume:
        cb->setCurrentIndex(2);
        break;
    case FMODEventAction::Stop:
        cb->setCurrentIndex(3);
        break;
    default:
        break;
    }
}

FMODEventAction GetEventActionFromComboBox(QComboBox* cb)
{
    switch(cb->currentIndex())
    {
    case 0: return FMODEventAction::Play;
    case 1: return FMODEventAction::Pause;
    case 2: return FMODEventAction::Resume;
    case 3: return FMODEventAction::Stop;
    default: return FMODEventAction::Unknown;
    }
}

EventPropertiesForm::EventPropertiesForm(AudioControls::TConnectionPtr connection, AudioControls::TImplControlType ctrlType, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventPropertiesForm),
    m_connection(connection),
    m_controlType(ctrlType)
{
    ui->setupUi(this);

    AzQtComponents::CheckBox::applyToggleSwitchStyle(ui->m_cbLoadSampleData);
    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->m_cbAction);
    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->m_cbStopMode);
    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->comboBox_3);

    connect(ui->m_cbAction, &QComboBox::currentIndexChanged, this, &EventPropertiesForm::UpdateConnectionFromWidgets);
    connect(ui->m_cbStopMode, &QComboBox::currentIndexChanged, this, &EventPropertiesForm::UpdateConnectionFromWidgets);
    connect(ui->m_cbLoadSampleData, &QCheckBox::toggled, this, &EventPropertiesForm::UpdateConnectionFromWidgets);

    UpdateWidgetsFromConnection();
}

EventPropertiesForm::~EventPropertiesForm()
{
    delete ui;
}

void EventPropertiesForm::UpdateConnectionFromWidgets()
{
    if(m_inUpdateWidgetsFromConnection)
        return;

    auto* conn = static_cast<CFMODEventConnection*>(m_connection.get());
    conn->m_action   = GetEventActionFromComboBox(ui->m_cbAction);
    conn->m_stopMode = ui->m_cbStopMode->currentIndex() == 0 ? FMOD_STUDIO_STOP_ALLOWFADEOUT : FMOD_STUDIO_STOP_IMMEDIATE;
    conn->m_loadSampleData = ui->m_cbLoadSampleData->isChecked();

    emit PropertiesChanged();
}

void EventPropertiesForm::UpdateWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;

    auto* conn = static_cast<CFMODEventConnection*>(m_connection.get());
    ui->m_cbStopMode->setCurrentIndex(conn->m_stopMode == FMOD_STUDIO_STOP_ALLOWFADEOUT ? 0 : 1);
    SetComboboxIndexFromAction(conn->m_action, ui->m_cbAction);
    ui->m_cbLoadSampleData->setChecked(conn->m_loadSampleData);

    m_inUpdateWidgetsFromConnection = false;
}

}