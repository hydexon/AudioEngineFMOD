#ifndef EVTTORTPCPARAMSFORM_H
#define EVTTORTPCPARAMSFORM_H

#include <QWidget>
#include <ACETypes.h>

namespace Ui {
class EvtToRtpcParamsForm;
}

namespace AudioEngineFMOD
{

class CAudioFMODDataLoader;

class EvtToRtpcParamsForm : public QWidget
{
    Q_OBJECT

public:
    explicit EvtToRtpcParamsForm(AudioControls::TConnectionPtr connection, CAudioFMODDataLoader* dataLoader, QWidget *parent = nullptr);
    ~EvtToRtpcParamsForm();

signals:
    void PropertiesChanged();

private:
    Ui::EvtToRtpcParamsForm *ui;
    CAudioFMODDataLoader* m_loader;
    AudioControls::TConnectionPtr m_connection;

    void UpdateWidgetsFromConnection();
};
}
#endif // EVTTORTPCPARAMSFORM_H
