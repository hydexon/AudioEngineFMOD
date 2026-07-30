#include "EventPropertiesForm.h"
#include "ui_EventPropertiesForm.h"

namespace AudioEngineFMOD {
EventPropertiesForm::EventPropertiesForm(AudioControls::TConnectionPtr connection, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EventPropertiesForm),
    m_connection(connection)
{
    ui->setupUi(this);
}

EventPropertiesForm::~EventPropertiesForm()
{
    delete ui;
}

}