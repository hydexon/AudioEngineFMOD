#include "FMODParamToStateWidget.h"
#include "ui_FMODParamToStateWidget.h"

#include <float.h>
#include "../AudioConnections.h"

using namespace AudioEngineFMOD;

FMODParamToStateWidget::FMODParamToStateWidget(AudioControls::TConnectionPtr connection, QWidget *parent) :
    QWidget(parent),
    m_conn(connection),
    ui(new Ui::FMODParamToStateWidget)
{
    ui->setupUi(this);

    ui->dsb_Value->setMinimum(FLT_MIN);
    ui->dsb_Value->setMaximum(FLT_MAX);

    connect(ui->dsb_Value, &QDoubleSpinBox::valueChanged, this, &FMODParamToStateWidget::UpdateConnectionFromWidgets);

    UpdateWidgetsFromConnection();
}

FMODParamToStateWidget::~FMODParamToStateWidget()
{
    delete ui;
}

void FMODParamToStateWidget::UpdateConnectionFromWidgets()
{
    if(m_inUpdateWidgetsFromConnection)
        return;

    auto conn = static_cast<FMODParamToStateConnection*>(m_conn.get());
    conn->m_value = ui->dsb_Value->value();

    emit PropertiesChanged();
}

void FMODParamToStateWidget::UpdateWidgetsFromConnection()
{
    m_inUpdateWidgetsFromConnection = true;

    auto conn = static_cast<FMODParamToStateConnection*>(m_conn.get());
    ui->dsb_Value->setValue(conn->m_value);

    m_inUpdateWidgetsFromConnection = false;
}
