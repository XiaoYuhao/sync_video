#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include "setupwindow.h"
#include "openzen/OpenZen.h"
#include "openzen/ZenTypes.h"
#include "kinect/k4a.h"
#include <QTimer>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QMutex>
#include <QQueue>
#include <QVector>
#include <QAction>
#include <QMenu>
#include <QSettings>
#include <QDebug>
#include <QMetaType>
#include <cstdio>
#include "utils.h"

extern "C" {
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavdevice/avdevice.h"
}

Q_DECLARE_METATYPE(std::vector<std::string>*);
//Q_DECLARE_METATYPE(std::vector<ZenImuData>*);
Q_DECLARE_METATYPE(ZenImuData);

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = Q_NULLPTR);
    ~QtWidgetsApplication1();

private:
    Ui::QtWidgetsApplication1Class ui;

private:
    setupwindow setupwin;

    QTimer* show_timer;
    QTimer* write_timer;
    QTimer* time_timer;

    bool stop_camera, stop_record;
    QTime start_time;

    QSettings* settings;


    bool camera1_show_ready;
    bool camera2_show_ready;

    QString save_path;
    int save_image_count;

    int record1_count, record2_count;
    AVPacket camera1_show_pkt, camera2_show_pkt;
    k4a_image_t show_color;

    QString dirname1, dirname2, dirname3, dirname4;

    AVCodec* camera1_pCodec, *camera2_pCodec;
    AVCodecContext* camera1_pCodecCtx, *camera2_pCodecCtx;
    AVFrame *pFrame;

    //zen::ZenClient *zenclient;
    //std::vector<ZenImuData> imudata;
    std::vector<std::string> sensors_name;
    bool can_write_imudata;

    QVector<QFile*> imu_files;
    QVector<QTextStream*> imu_outstreams;


signals:
    void ready_to_show();
    void ready_to_initimu(std::vector<std::string>*);
    //void ready_to_updateimu(std::vector<ZenImuData>*);
    void ready_to_updateimu(ZenImuData, int);
    void ready_to_appendlog(QString);
 
private slots:
    void start_botton_clicked();
    void stop_botton_clicked();
    void close_botton_clicked();
    void tool_botton_clicked();
    
    void updateTime();
    void openSetupWindow();

    void readCamera();
    void readKinect();

    void importFrame();
    //void writeFrame();

    int createVideo(const QString &dirname, const QString &outfile);

    int initImuSensor();
    void readImuSensor(std::string& io_type, std::string& sensor_name, std::string& identifer, int index);
    void initSensorTable(std::vector<std::string>*);
    //void updateSensorTable(std::vector<ZenImuData>*);
    void updateSensorTable(ZenImuData, int);
    void appendLog(QString);
};
