#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
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
    void                on_reset_clicked();

private:
    Ui::MainWindow      *ui;
    QSerialPort         *m_serial;
    Settings            m_currentSettings;
    QVector< double >   m_samples_frequencies; // Divide time in slids of 25ms. This will cast 40 stages for a whole second. 120stages for 3 seconds.
    int                 m_sample_frequency_window = 25;

    QCustomPlot         *m_red_plot;
    QCustomPlot         *m_blue_plot;
    QCustomPlot         *m_green_plot;
    QCustomPlot         *m_yellow_plot;

    QVector<double>     m_red_pushes;
    QVector<double>     m_red_times;
    double              m_red_average = 0.0;

    QVector<double>     m_blue_pushes;
    QVector<double>     m_blue_times;
    double              m_blue_average = 0.0;

    QVector<double>     m_green_pushes;
    QVector<double>     m_green_times;
    double              m_green_average = 0.0;

    QVector<double>     m_yellow_pushes;
    QVector<double>     m_yellow_times;
    double              m_yellow_average = 0.0;


    void                setSerialPortSetting( Settings p_settings );
    void                openSerialPort();
    void                closeSerialPort();
    void                writeData(const QByteArray &data);
    void                readData();
    void                handleError(QSerialPort::SerialPortError error);
    void                about();
    void                light_up_button(const QString &data);
    void                update_plot(QCustomPlot *plot);
    void                classify_sample(QCustomPlot *plot );
    void                set_game_controller();
    void                initialize_plots( QCustomPlot *p_plot, const QColor &color );
    void                update_pushes_counters();
    int                 sum_up_pushes( const QVector< double > &vector );
    double              average_pushing_speed(const QVector<double> &vector);
};

#endif // MAINWINDOW_H
