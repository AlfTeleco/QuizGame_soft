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

    QCustomPlot *plot = new QCustomPlot(ui->plot);
    QVBoxLayout *vertical_layout = new QVBoxLayout;
    vertical_layout->addWidget(plot);
    ui->plot->setLayout( vertical_layout );
    plot->setObjectName( "plot" );
    plot->setInteractions(QCP::iSelectPlottables | QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect );
    plot->xAxis->setLabel("Time(ms)");
    plot->yAxis->setLabel("Nº of hits");
    plot->addGraph();
    plot->addGraph();
    plot->addGraph();
    plot->graph(0)->setName("red");
    plot->graph(0)->setPen(QColor(255, 0, 0, 255));
    plot->graph(0)->setLineStyle(QCPGraph::lsNone);
    plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));
    plot->graph(1)->setName("blue");
    plot->graph(1)->setPen(QColor(0, 0, 255, 255));
    plot->graph(1)->setLineStyle(QCPGraph::lsNone);
    plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));
    plot->graph(2)->setName("yellow");
    plot->graph(2)->setPen(QColor(0, 255, 255, 255));
    plot->graph(2)->setLineStyle(QCPGraph::lsNone);
    plot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));

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

void MainWindow::light_up_button( const QString &data )
{
    if( data.contains("R"))
    {
        QString current_style = ui->red->styleSheet();
        if( current_style.contains("rgb(255,0,0)") )
        {
            ui->red->setStyleSheet("background-color: rgb(0,0,0);");
        }
        else
        {
            ui->red->setStyleSheet("background-color: rgb(255,0,0);");
        }
        ui->go->setStyleSheet(" background-color: rgb(255,255,255); ");
        ui->blue->setStyleSheet(" background-color: rgb(255,255,255); ");
        ui->red->animateClick(100);
    }
    else if( data.contains("B"))
    {
        QString current_style = ui->blue->styleSheet();
        if( current_style.contains("rgb(0,0,255)") )
        {
            ui->blue->setStyleSheet("background-color: rgb(0,0,0);");
        }
        else
        {
            ui->blue->setStyleSheet("background-color: rgb(0,0,255);");
        }
        ui->go->setStyleSheet(" background-color: rgb(255,255,255); ");
        ui->red->setStyleSheet(" background-color: rgb(255,255,255); ");
        ui->blue->animateClick(100);
    }
    else if( data.contains("!"))
    {
        QString current_style = ui->go->styleSheet();
        if( current_style.contains("rgb(0,255,0)") )
        {
            ui->go->setStyleSheet("background-color: rgb(255,255,255);");
        }
        else
        {
            ui->go->setStyleSheet("background-color: rgb(0,255,0);");
        }
        ui->red->setStyleSheet(" background-color: rgb(255,255,255); ");
        ui->blue->setStyleSheet(" background-color: rgb(255,255,255); ");
        ui->go->animateClick(100);
    }
    update_plot();
}

void MainWindow::readData()
{
    QByteArray data = m_serial->readAll();
    qDebug() << "Serial port received " << data;
    ui->log->insertPlainText( QString( data )  );
    light_up_button( data );
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::update_plot()
{
    QString log_contents = ui->log->toPlainText();
    if( log_contents.isEmpty() )
    {
        return;
    }


    if( !log_contents.contains("ms") )
    {
        return;
    }
    QStringList log_text = log_contents.split("ms");

    int number_of_pushings = log_text.size() - 1 ;

    QString button,time;
    QList< QPair< QString, int > > button_pushes;

    for( int l_var0 = 0; l_var0 < number_of_pushings; l_var0++)
    {
        button = log_text.at(l_var0);
        if( !button.split(' ').isEmpty() )
        {
            time = button.split(' ').at( button.split(' ').count() - 1 );
            button = button.split(' ').at( button.split(' ').count() - 2 );
            QPair< QString, int > new_pair( button, time.split("ms").first().toInt() );

            button_pushes.append( new_pair );
        }
    }

    QVector<double> x0(button_pushes.size()), y0(button_pushes.size());
    QVector<double> x1(button_pushes.size()), y1(button_pushes.size());
    QVector<double> x2(button_pushes.size()), y2(button_pushes.size());
    QVector<double> x3(button_pushes.size()), y3(button_pushes.size());

    for( int l_var0 = 0; l_var0 < button_pushes.size(); l_var0++ )
    {
        if( button_pushes.at(l_var0).first.contains('R') )
        {
            x0[l_var0] = button_pushes.at(l_var0).second;
            y0[l_var0] = 1;
        }
        else if( button_pushes.at(l_var0).first.contains('B') )
        {
            x1[l_var0] = button_pushes.at(l_var0).second;
            y1[l_var0] = 1;
        }

    }
    auto plot = get_custom_plot();
    plot->graph(0)->setData(x0, y0);
    plot->graph(1)->setData(x1, y1);
    plot->rescaleAxes();
    plot->yAxis->setRangeUpper( plot->yAxis->range().upper *2 );
    plot->xAxis->setRangeUpper( plot->xAxis->range().upper *2 );
    plot->replot();

}

QCustomPlot *MainWindow::get_custom_plot()
{
    QCustomPlot *plot = dynamic_cast < QCustomPlot*>( ui->plot->children().first() );
    return plot;
}

void MainWindow::on_pushButton_3_clicked()
{
    ui->log->clear();
    update_plot();
}
