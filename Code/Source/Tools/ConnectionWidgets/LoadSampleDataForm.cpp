#include "LoadSampleDataForm.h"
#include "ui_LoadSampleDataForm.h"

#include <AzQtComponents/Components/Widgets/CheckBox.h>
#include <QCheckBox>

#include "../AudioConnections.h"

namespace AudioEngineFMOD {

LoadSampleDataForm::LoadSampleDataForm(AudioControls::TConnectionPtr connection, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadSampleDataForm),
    m_connection(connection)
{
    ui->setupUi(this);

    AzQtComponents::CheckBox::applyToggleSwitchStyle(ui->m_cbLoadSampleData);

    auto conn = static_cast<CFMODBankConnection*>(m_connection.get());
    ui->m_cbLoadSampleData->setChecked(conn->m_loadSampleData);

    connect(ui->m_cbLoadSampleData, &QCheckBox::toggled, this, &LoadSampleDataForm::ToggleConnectionFromWidget);
    ToggleWidgetsFromConnection();
}

LoadSampleDataForm::~LoadSampleDataForm()
{
    delete ui;
}

void LoadSampleDataForm::ToggleConnectionFromWidget(bool checked)
{
    if(m_inUpdateWidgetsFromConnection)
        return;

    CFMODBankConnection* conn = static_cast<CFMODBankConnection*>(m_connection.get());
    conn->m_loadSampleData = checked;

    emit PropertiesChanged();
}

void LoadSampleDataForm::ToggleWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;

    CFMODBankConnection* conn = static_cast<CFMODBankConnection*>(m_connection.get());
    ui->m_cbLoadSampleData->setChecked(conn->m_loadSampleData);

    m_inUpdateWidgetsFromConnection = false;

}

} // namespace AudioEngineFMOD
