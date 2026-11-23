#include "EventPropertiesForm.h"
#include "ui_EventPropertiesForm.h"

#include <AzQtComponents/Components/Widgets/CheckBox.h>
#include <AzQtComponents/Components/Widgets/ComboBox.h>
#include <AzQtComponents/Components/Widgets/SpinBox.h>

#include "../AudioConnections.h"
#include "../../Clients/Engine/Common_FMOD.h"

namespace AudioEngineFMOD {

void SetComboboxIndexFromAction(FMODEventType action, QComboBox* cb)
{
    switch(action) {
    case FMODEventType::Play:
        cb->setCurrentIndex(0);
        break;
    case FMODEventType::Stop:
        cb->setCurrentIndex(1);
        break;
    default:
        break;
    }
}

FMODEventType GetEventActionFromComboBox(QComboBox* cb)
{
    return FMODEventType::Play;
}

EventPropertiesForm::EventPropertiesForm(AudioControls::TConnectionPtr connection, AudioControls::TImplControlType ctrlType, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventPropertiesForm),
    m_connection(connection),
    m_controlType(ctrlType)
{
    ui->setupUi(this);

    AzQtComponents::ComboBox::addCustomCheckStateStyle(ui->m_cbAction);

    connect(ui->m_cbAction, &QComboBox::currentIndexChanged, this, &EventPropertiesForm::UpdateConnectionFromWidgets);

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

    auto* conn = static_cast<FMODEventConnection*>(m_connection.get());
    conn->m_eventType   = GetEventActionFromComboBox(ui->m_cbAction);

    emit PropertiesChanged();
}

void EventPropertiesForm::UpdateWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;

    auto* conn = static_cast<FMODEventConnection*>(m_connection.get());
    SetComboboxIndexFromAction(conn->m_eventType, ui->m_cbAction);
    m_inUpdateWidgetsFromConnection = false;
}

}