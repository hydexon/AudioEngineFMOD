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
        explicit EventPropertiesForm(AudioControls::TConnectionPtr connection, QWidget *parent = nullptr);
        ~EventPropertiesForm();

    signals:
        void PropertiesChanged();


    private:
        Ui::EventPropertiesForm *ui;
        AudioControls::TConnectionPtr m_connection;
    };
}
#endif // EVENTPROPERTIESFORM_H
