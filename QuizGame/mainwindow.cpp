#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_serial = new QSerialPort(this);
    connect( m_serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MainWindow::handleError);
    connect( m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    Settings t_settings;
    t_settings.name = "COM5";
    t_settings.baudRate = 9600;
    t_settings.dataBits = QSerialPort::Data8;
    t_settings.parity = QSerialPort::NoParity;
    t_settings.stopBits = QSerialPort::OneStop;
    t_settings.flowControl = QSerialPort::NoFlowControl;
    setSerialPortSetting( t_settings );
    if( !m_serial->isOpen() )
    {
        openSerialPort();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSerialPortSetting( MainWindow::Settings p_settings)
{
    m_currentSettings.name = p_settings.name;
    m_currentSettings.baudRate = p_settings.baudRate;
    m_currentSettings.dataBits = p_settings.dataBits;
    m_currentSettings.parity = p_settings.parity;
    m_currentSettings.stopBits = p_settings.stopBits;
    m_currentSettings.flowControl = p_settings.flowControl;
}

void MainWindow::openSerialPort()
{
    m_serial->setPortName(m_currentSettings.name);
    m_serial->setBaudRate(m_currentSettings.baudRate);
    m_serial->setDataBits(m_currentSettings.dataBits);
    m_serial->setParity(m_currentSettings.parity);
    m_serial->setStopBits(m_currentSettings.stopBits);
    m_serial->setFlowControl(m_currentSettings.flowControl);
    if( m_serial->open( QIODevice::ReadWrite ) )
    {
        qDebug() << "Connected!!";
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

    }
}

void MainWindow::closeSerialPort()
{
    if( m_serial->isOpen() )
    {
        m_serial->close();
    }
    qDebug() << "Disconnected";
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Sobre Gestor de accesos"),
                       tr("El <b>Gestor de accesos</b> es una pequeña aplicación de "
                          "demostración cuyo único objetivo es el de mostrar un ejemplo "
                          "de la dinámica del sistema y sus posibilidades."));
}

void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
    ui->log->insertPlainText( QTime::currentTime().toString() + ": " + QString( data ) );
}

void MainWindow::readData()
{
    QByteArray data = m_serial->readAll();
    qDebug() << "Serial port received " << data;
    ui->log->insertPlainText( QTime::currentTime().toString() + ": " + QString( data )  );
    if( data.contains("R"))
    {
        QString current_style = ui->pushButton->styleSheet();
        if( current_style.contains("rgb(255,0,0)") )
        {
            ui->pushButton->setStyleSheet("background-color: rgb(255,255,255);");
        }
        else
        {
            ui->pushButton->setStyleSheet("background-color: rgb(255,0,0);");
        }
    }
    if( data.contains("W"))
    {
        ui->pushButton->setStyleSheet("background-color: rgb(255,255,255);");
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}
