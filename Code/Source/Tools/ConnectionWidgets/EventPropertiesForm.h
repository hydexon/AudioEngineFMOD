#ifndef EVENTPROPERTIESFORM_H
#define EVENTPROPERTIESFORM_H

#include <QWidget>
#include <ACETypes.h>

namespace Ui {
class EventPropertiesForm;
}

namespace AudioEngineFMOD
{
    class EventPropertiesForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit EventPropertiesForm(AudioControls::TConnectionPtr connection, AudioControls::TImplControlType ctrlType, QWidget *parent = nullptr);
        ~EventPropertiesForm();

    signals:
        void PropertiesChanged();

    private slots:
        void UpdateConnectionFromWidgets();

    private:
        Ui::EventPropertiesForm *ui;
        AudioControls::TConnectionPtr m_connection;
        AudioControls::TImplControlType m_controlType;

        bool m_inUpdateWidgetsFromConnection = false;
        void UpdateWidgetsFromConnection();
    };
}
#endif // EVENTPROPERTIESFORM_H
