#ifndef FMODPARAMTOSTATEWIDGET_H
#define FMODPARAMTOSTATEWIDGET_H

#include <QWidget>
#include <ACETypes.h>

namespace Ui {
class FMODParamToStateWidget;
}

class FMODParamToStateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FMODParamToStateWidget(AudioControls::TConnectionPtr connection, QWidget *parent = nullptr);
    ~FMODParamToStateWidget();

signals:
    void PropertiesChanged();

private slots:
    void UpdateConnectionFromWidgets();

private:
    Ui::FMODParamToStateWidget *ui;
    AudioControls::TConnectionPtr m_conn;
    bool m_inUpdateWidgetsFromConnection = false;

    void UpdateWidgetsFromConnection();

};

#endif // FMODPARAMTOSTATEWIDGET_H
