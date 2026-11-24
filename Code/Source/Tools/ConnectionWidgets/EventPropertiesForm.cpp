#include "EventPropertiesForm.h"
#include "ui_EventPropertiesForm.h"

#include <AzQtComponents/Components/Widgets/CheckBox.h>
#include <AzQtComponents/Components/Widgets/ComboBox.h>
#include <AzQtComponents/Components/Widgets/SpinBox.h>

#include "../AudioConnections.h"
#include "../../Clients/Engine/Common_FMOD.h"

namespace AudioEngineFMOD {

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
    conn->m_eventType   = static_cast<FMODEventType>(ui->m_cbAction->currentIndex());

    emit PropertiesChanged();
}

void EventPropertiesForm::UpdateWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;

    auto* conn = static_cast<FMODEventConnection*>(m_connection.get());
    ui->m_cbAction->setCurrentIndex(static_cast<int>(conn->m_eventType));
    m_inUpdateWidgetsFromConnection = false;
}

}