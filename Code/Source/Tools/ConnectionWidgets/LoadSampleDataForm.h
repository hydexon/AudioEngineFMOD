#ifndef LOADSAMPLEDATAFORM_H
#define LOADSAMPLEDATAFORM_H

#include <QWidget>
#include <ACETypes.h>

namespace Ui {
class LoadSampleDataForm;
}
namespace AudioEngineFMOD {

class LoadSampleDataForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoadSampleDataForm(AudioControls::TConnectionPtr connection, QWidget *parent = nullptr);
    ~LoadSampleDataForm();

signals:
    void PropertiesChanged();

private slots:
    void ToggleConnectionFromWidget(bool checked);

private:
    Ui::LoadSampleDataForm *ui;
    AudioControls::TConnectionPtr m_connection;
    bool m_inUpdateWidgetsFromConnection = false;

    void ToggleWidgetsFromConnection();
};


} // namespace AudioEngineFMOD
#endif // LOADSAMPLEDATAFORM_H
