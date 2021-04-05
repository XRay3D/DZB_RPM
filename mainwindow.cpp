#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QModbusRtuSerialMaster>
#include <QtSerialPort>
#include <ranges>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_device = new QModbusRtuSerialMaster(this);
    auto availablePorts { QSerialPortInfo::availablePorts().toVector() };
    std::ranges::sort(availablePorts, {}, [](QSerialPortInfo& pi) { return pi.portName().midRef(3).toUInt(); });
    for (const QSerialPortInfo& info : qAsConst(availablePorts)) {
        //        if (info.manufacturer().contains("FTDI"))
        ui->comboBox->addItem(info.portName());
    }
    loadSettings();
}

MainWindow::~MainWindow()
{
    stop();
    saveSettings();
    delete ui;
}

void MainWindow::on_pushButton_clicked(bool checked)
{
    checked ? setRpm(ui->spinBox->value()), start() : stop();
    ui->pushButton->setText(checked ? "Stop" : "Start");
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    setRpm(arg1);
}

uint MainWindow::calcCrc(const QByteArray& data)
{
    uint crc = 0xFFFF;
    uchar* d = (uchar*)data.data();
    for (int i = 0; i < data.size(); ++i) {
        crc ^= d[i];
        for (int k = 0; k < 8; ++k) {
            crc = (crc & 0x0001) ? (crc >> 1) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("State", saveState());
    settings.setValue("comboBox", ui->comboBox->currentText());
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("State").toByteArray());
    ui->comboBox->setCurrentText(settings.value("comboBox").toString());
}

enum RD {
    Read = 0x03,
    Write = 0x06,
};

void MainWindow::start()
{
    uint16_t addr = 0x1000;
    uint16_t data = 0x0001;
    m_device->sendRawRequest(QModbusRequest(QModbusRequest::WriteSingleRegister, addr, data), 1);
}

void MainWindow::stop()
{
    uint16_t addr = 0x1000;
    uint16_t data = 0x0005;
    m_device->sendRawRequest(QModbusRequest(QModbusRequest::WriteSingleRegister, addr, data), 1);
    //    QByteArray data;
    //    data.append(char(0x01));
    //    data.append(Write);
    //    //addr
    //    data.append(char(0x10));
    //    data.append(char(0x00));
    //    //data
    //    data.append(char(0x00));
    //    data.append(char(0x05));

    //    uint crc = calcCrc(data);
    //    char* d = (char*)&crc;

    //    data.append(d[0]);
    //    data.append(d[1]);

    //    write(data);
}

void MainWindow::setRpm(int rpm)
{
    uint16_t addr = 0x2000;
    uint16_t data = rpm * 0.416666667;
    m_device->sendRawRequest(QModbusRequest(QModbusRequest::WriteSingleRegister, addr, data), 1);
    //    rpm *= 0.416666667; // 25;

    //    QByteArray data;
    //    data.append(char(0x01));
    //    data.append(Write);
    //    //addr
    //    data.append(char(0x20));
    //    data.append(char(0x00));
    //    //data
    //    char* r = (char*)&rpm;
    //    data.append(r[1]);
    //    data.append(r[0]);

    //    uint crc = calcCrc(data);
    //    char* d = (char*)&crc;

    //    data.append(d[0]);
    //    data.append(d[1]);

    //    write(data);
}

void MainWindow::on_spinBox_editingFinished()
{
    //setRpm(ui->spinBox->value());
}

void MainWindow::on_comboBox_currentIndexChanged(const QString& arg1)
{
    qDebug() << arg1;
    m_device->disconnectDevice();
    m_device = new QModbusRtuSerialMaster;
    m_device->setConnectionParameter(QModbusDevice::SerialPortNameParameter, arg1);
    m_device->setConnectionParameter(QModbusDevice::SerialParityParameter, {});
    m_device->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    m_device->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
    m_device->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    m_device->setTimeout(100);
    m_device->setNumberOfRetries(0);
    m_device->connectDevice();
}
