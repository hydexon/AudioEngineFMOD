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

    for(const auto& param : m_loader->GetEventParameters())
    {
        ui->m_paramsCb->addItem(param.first.c_str());
    }
}

EvtToRtpcParamsForm::~EvtToRtpcParamsForm()
{
    delete ui;
}

void EvtToRtpcParamsForm::UpdateWidgetsFromConnection()
{
    auto conn = static_cast<CFMODEventParamRTPC*>(m_connection.get());
    AZ_UNUSED(conn);
}
}