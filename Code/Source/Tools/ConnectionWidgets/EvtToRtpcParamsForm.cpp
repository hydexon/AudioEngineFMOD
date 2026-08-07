#include "EvtToRtpcParamsForm.h"
#include "ui_EvtToRtpcParamsForm.h"

#include "../AudioConnections.h"
#include "../AudioFMODDataLoader.h"

namespace AudioEngineFMOD {


EvtToRtpcParamsForm::EvtToRtpcParamsForm(AudioControls::TConnectionPtr connection, CAudioFMODDataLoader *dataLoader, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EvtToRtpcParamsForm),
    m_connection(connection),
    m_loader(dataLoader)
{
    ui->setupUi(this);

    connect(ui->m_paramsCb, &QComboBox::currentIndexChanged, this, &EvtToRtpcParamsForm::UpdateConnectionFromWidgets);

    UpdateWidgetsFromConnection();
}

EvtToRtpcParamsForm::~EvtToRtpcParamsForm()
{
    delete ui;
}

void EvtToRtpcParamsForm::UpdateConnectionFromWidgets()
{
    if(m_inUpdateWidgetsFromConnection)
        return;

    auto conn = static_cast<CFMODEventParamRTPC*>(m_connection.get());
    conn->m_paramName = ui->m_paramsCb->currentText().toStdString().c_str();

    emit PropertiesChanged();
}

void EvtToRtpcParamsForm::UpdateWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;
    for(const auto& param : m_loader->GetEventParameters())
    {
        ui->m_paramsCb->addItem(param.first.c_str());
    }

    auto conn = static_cast<CFMODEventParamRTPC*>(m_connection.get());
    const int cbIdx = ui->m_paramsCb->findText(conn->m_paramName.c_str());
    ui->m_paramsCb->setCurrentIndex(cbIdx);

    m_inUpdateWidgetsFromConnection = false;
}
}