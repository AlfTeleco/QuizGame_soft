#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_samples_frequencies.resize( 120 ); // Bar width = 3000ms / 120 = 25ms
    m_samples_frequencies.resize( 120 ); // Bar width = 3000ms / 120 = 25ms

    m_red_pushes.resize( 3000 / m_sample_frequency_window );// Bar width = 3000ms / m_sample_frequency_window ( 3000ms / 25 = 120 steps -> 25ms  )
    m_blue_pushes.resize( 3000 / m_sample_frequency_window );// Bar width = 3000ms / m_sample_frequency_window
    m_green_pushes.resize( 3000 / m_sample_frequency_window );// Bar width = 3000ms / m_sample_frequency_window
    m_yellow_pushes.resize( 3000 / m_sample_frequency_window );// Bar width = 3000ms / m_sample_frequency_window

    m_serial = new QSerialPort(this);
    connect( m_serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &MainWindow::handleError);
    connect( m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    set_game_controller();

    m_red_plot = new QCustomPlot(ui->plot);
    m_blue_plot = new QCustomPlot(ui->plot);
    m_green_plot = new QCustomPlot(ui->plot);
    m_yellow_plot = new QCustomPlot(ui->plot);

    initialize_plots( m_red_plot, QColor( 255, 0, 0  ) );
    initialize_plots( m_blue_plot, QColor( 0, 0, 255  ) );
    initialize_plots( m_green_plot, QColor( 0, 255, 0  ) );
    initialize_plots( m_yellow_plot, QColor( 0, 255, 255  ) );

    QGridLayout *grid_layout = new QGridLayout;

    grid_layout->addWidget(m_red_plot, 0, 0);
    grid_layout->addWidget(m_green_plot, 1, 0);
    grid_layout->addWidget(m_blue_plot, 0, 1);
    grid_layout->addWidget(m_yellow_plot, 1, 1);
    ui->plot->setLayout( grid_layout );


}

MainWindow::~MainWindow()
{
    closeSerialPort();
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
    if( m_serial->open( QIODevice::ReadOnly ) )
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
        m_serial->flush();
        m_serial->clear();
        m_serial->clearError();
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
        classify_sample( m_red_plot );
        ui->red_counter->setText( "Counter: " +  QString::number(sum_up_pushes(m_red_pushes)));
        ui->red_average->setText("Av.: " + QString::number(average_pushing_speed(m_red_times) ) + "ms");
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
        classify_sample( m_blue_plot );
        ui->blue_counter->setText( "Counter: " +  QString::number(sum_up_pushes(m_blue_pushes)));
        ui->blue_average->setText("Av.: " + QString::number(average_pushing_speed(m_blue_times) ) + "ms");
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
        return;
    }
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

void MainWindow::classify_sample( QCustomPlot *plot )
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
//    QStringList log_text = log_contents.split("\r\n");

//    int number_of_pushings = log_contents.split("\r\n")() - 1 ;

//    QString button,time;
//    QList< QPair< QString, int > > button_pushes;

//    for( int l_var0 = 0; l_var0 < number_of_pushings; l_var0++)
//    {
//        button = log_text.at(l_var0);
//        if( !button.split(' ').isEmpty() )
//        {
//            time = button.split(' ').at( button.split(' ').count() - 1 );
//            button = button.split(' ').at( button.split(' ').count() - 2 );
//            QPair< QString, int > new_pair( button, time.split("ms").first().toInt() );
//            button_pushes.append( new_pair );
//        }
//    }

    QStringList log_text = log_contents.split("*");
    QString button,time;
    QPair< QString, int > button_pushes;

    int last_index_full  = log_text.size() - 1;
    while( log_text.at( last_index_full ).isEmpty() )
    {
        last_index_full--;
    }

    time = log_text.at( last_index_full );
    time = time.split("_").at( 1 );
    button = log_text.at( last_index_full ).split("_").last();
    QPair< QString, int > new_pair( button, time.toInt() );

    QString sample_color("R");
    int t_index( new_pair.second / m_sample_frequency_window );

    if( plot == m_red_plot )
    {
        sample_color = "R";
        m_red_pushes[ t_index ] += 1 ;
        m_red_times.append( new_pair.second );
    }
    else if( plot == m_blue_plot )
    {
        sample_color = "B";
        m_blue_pushes[ t_index ] += 1 ;
        m_blue_times.append( new_pair.second );
    }
    else if( plot == m_green_plot )
    {
        sample_color = "G";
        m_green_pushes[ t_index ] += 1 ;
        m_green_times.append( new_pair.second );
    }
    else if( plot == m_yellow_plot )
    {
        sample_color = "Y";
        m_yellow_pushes[ t_index ] += 1 ;
        m_yellow_times.append( new_pair.second );
    }

    update_plot( plot );

}


void MainWindow::update_plot( QCustomPlot *plot )
{
    QVector< double > *bar_y = nullptr;
    if( plot == m_red_plot )
    {
        bar_y = &m_red_pushes;
    }
    else if( plot == m_blue_plot )
    {
        bar_y = &m_blue_pushes;
    }
    else if( plot == m_green_plot )
    {
        bar_y = &m_green_pushes;
    }
    else if( plot == m_yellow_plot )
    {
        bar_y = &m_yellow_pushes;
    }

    QVector< double > bars_x;
    for( int l_var0 = 0; l_var0 < bar_y->size(); l_var0++ )
    {
        bars_x.append(  l_var0 * m_sample_frequency_window );
    }

    if( plot->plottable() )
    {
        QCPBars *bars = dynamic_cast< QCPBars* > ( plot->plottable() );
        bars->setData( bars_x, *bar_y);
    }
    plot->rescaleAxes();
    plot->yAxis->setRangeUpper( 10 );
    plot->replot();

}

void MainWindow::on_reset_clicked()
{
    ui->log->clear();
    m_red_pushes.clear();
    m_red_times.clear();
    m_blue_pushes.clear();
    m_blue_times.clear();
    m_green_pushes.clear();
    m_green_times.clear();
    m_yellow_pushes.clear();
    m_yellow_times.clear();

    update_plot( m_red_plot );
    update_plot( m_blue_plot );
    update_plot( m_green_plot );
    update_plot( m_yellow_plot );
}

void MainWindow::set_game_controller()
{
    Settings t_settings;
    QList<QSerialPortInfo> serial_port_info = QSerialPortInfo::availablePorts();
    QString port_description;
    for( int l_var0 = 0; l_var0 < serial_port_info.size(); l_var0++ )
    {
        port_description = serial_port_info.at(l_var0).description();
        if( port_description.contains("MSP430") )
        {
            t_settings.name = serial_port_info.at(l_var0).portName();
            qDebug() << "Game controller found at " + port_description
                        + " "
                        + t_settings.name;
        }
    }
    t_settings.baudRate = 9600;
    t_settings.dataBits = QSerialPort::Data8;
    t_settings.parity = QSerialPort::NoParity;
    t_settings.stopBits = QSerialPort::OneStop;
    t_settings.flowControl = QSerialPort::NoFlowControl;
    setSerialPortSetting( t_settings );
    m_serial->flush();
    m_serial->clear();
    m_serial->clearError();
    m_serial->close();

    if( !m_serial->isOpen() )
    {
        openSerialPort();
    }
}

void MainWindow::initialize_plots( QCustomPlot *p_plot, const QColor &color )
{

    // set dark background gradient:
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(255, 204, 204));
    gradient.setColorAt(0.38, QColor(255, 178, 102));
    gradient.setColorAt(1, QColor(255, 255, 153));
    p_plot->setBackground( QBrush( gradient ) );

    p_plot->setObjectName( "plot" );
    p_plot->setInteractions(QCP::iSelectPlottables | QCP::iRangeDrag | QCP::iRangeZoom | QCP::iMultiSelect );
    p_plot->xAxis->setLabel("Time(ms)");
//    p_plot->xAxis->setRange(0, 3000);
    p_plot->yAxis->setLabel("Nº of hits");
    p_plot->addGraph();
    p_plot->addGraph();
    p_plot->addGraph();
    p_plot->graph(0)->setName("graph");
    p_plot->graph(0)->setPen(QColor(Qt::black));
    p_plot->graph(0)->setLineStyle(QCPGraph::lsLine);
    //p_plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
    QCPBars *frequency = new QCPBars(p_plot->xAxis, p_plot->yAxis);
    frequency->setAntialiased(false);
    frequency->setName("Sample_frequency");
    frequency->setPen(QPen( color.lighter(170)));
    frequency->setBrush(color);
    frequency->setWidth( 20 );

}

int MainWindow::sum_up_pushes(const QVector<double> &vector)
{
    int ret = 0;
    for( int l_var0 = 0; l_var0 < vector.size(); l_var0++ )
    {
        ret += vector[l_var0];
    }
    return ret;
}

double MainWindow::average_pushing_speed(const QVector<double> &vector)
{
    double ret = 0;
    for( int l_var0 = 0; l_var0 < vector.size(); l_var0++ )
    {
        ret += vector[l_var0];
    }
    ret /= vector.size();
    return ret;
}

