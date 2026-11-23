#ifndef FMODPARAMTORTPCWIDGET_H
#define FMODPARAMTORTPCWIDGET_H

#include <QWidget>
#include <ACETypes.h>

namespace Ui {
class FMODParamToRTPCWidget;
}

class FMODParamToRTPCWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FMODParamToRTPCWidget(AudioControls::TConnectionPtr connection, QWidget *parent = nullptr);
    ~FMODParamToRTPCWidget();

signals:
    void PropertiesChanged();

private slots:
    void UpdateConnectionFromWidgets();

private:
    Ui::FMODParamToRTPCWidget *ui;
    AudioControls::TConnectionPtr m_conn;
    bool m_inUpdateWidgetsFromConnection = false;

    void UpdateWidgetsFromConnection();
};

#endif // FMODPARAMTORTPCWIDGET_H
