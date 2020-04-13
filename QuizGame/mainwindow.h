#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QtSerialPort/QSerialPort>
#include <qcustomplot.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };

private slots:
    void on_pushButton_3_clicked();

private:
    Ui::MainWindow      *ui;
    QSerialPort         *m_serial;
    Settings            m_currentSettings;

    void                setSerialPortSetting( Settings p_settings );
    void                openSerialPort();
    void                closeSerialPort();
    void                writeData(const QByteArray &data);
    void                readData();
    void                handleError(QSerialPort::SerialPortError error);
    void                about();
    void                light_up_button(const QString &data);
    void                update_plot();
    QCustomPlot         *get_custom_plot();
};

#endif // MAINWINDOW_H
