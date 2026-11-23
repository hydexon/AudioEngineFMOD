#include "FMODParamToRTPCWidget.h"
#include "ui_FMODParamToRTPCWidget.h"

#include <float.h>
#include "../AudioConnections.h"

using namespace AudioEngineFMOD;

FMODParamToRTPCWidget::FMODParamToRTPCWidget(AudioControls::TConnectionPtr connection, QWidget *parent) :
    QWidget(parent),
    m_conn(connection),
    ui(new Ui::FMODParamToRTPCWidget)
{
    ui->setupUi(this);

    ui->dsb_Mult->setMinimum(FLT_MIN);
    ui->dsb_Mult->setMaximum(FLT_MAX);

    ui->dsb_Shift->setMinimum(FLT_MIN);
    ui->dsb_Shift->setMaximum(FLT_MAX);

    connect(ui->dsb_Mult, &QDoubleSpinBox::valueChanged, this, &FMODParamToRTPCWidget::UpdateConnectionFromWidgets);
    connect(ui->dsb_Shift, &QDoubleSpinBox::valueChanged, this, &FMODParamToRTPCWidget::UpdateConnectionFromWidgets);

    UpdateWidgetsFromConnection();
}

FMODParamToRTPCWidget::~FMODParamToRTPCWidget()
{
    delete ui;
}

void FMODParamToRTPCWidget::UpdateConnectionFromWidgets()
{
    if(m_inUpdateWidgetsFromConnection)
        return;

    auto conn = static_cast<FMODParamRtpcConnection*>(m_conn.get());
    conn->m_mult  = ui->dsb_Mult->value();
    conn->m_shift = ui->dsb_Shift->value();

    emit PropertiesChanged();
}

void FMODParamToRTPCWidget::UpdateWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;

    auto conn = static_cast<FMODParamRtpcConnection*>(m_conn.get());
    ui->dsb_Mult->setValue(conn->m_mult);
    ui->dsb_Shift->setValue(conn->m_shift);

    m_inUpdateWidgetsFromConnection = false;
}
