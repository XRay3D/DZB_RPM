#pragma once

#include <QMainWindow>

class QModbusRtuSerialMaster;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked(bool checked);
    void on_spinBox_valueChanged(int arg1);
    void on_spinBox_editingFinished();

    void on_comboBox_currentIndexChanged(const QString& arg1);

private:
    Ui::MainWindow* ui;
    uint calcCrc(const QByteArray& data);

    void saveSettings();
    void loadSettings();

    void start();
    void stop();
    void setRpm(int rpm);

    QModbusRtuSerialMaster* m_device = nullptr;
};
