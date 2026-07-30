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
    ui->m_cbLoadSampleData->setChecked(conn->loadSampleData);

    connect(ui->m_cbLoadSampleData, &QCheckBox::toggled, this, &LoadSampleDataForm::ToggleConnectionFromWidget);
}

LoadSampleDataForm::~LoadSampleDataForm()
{
    delete ui;
}

void LoadSampleDataForm::ToggleConnectionFromWidget(bool checked)
{
    if(m_connection)
        return;

    CFMODBankConnection* conn = static_cast<CFMODBankConnection*>(m_connection.get());
    conn->loadSampleData = checked;

    emit PropertiesChanged();
}

} // namespace AudioEngineFMOD
