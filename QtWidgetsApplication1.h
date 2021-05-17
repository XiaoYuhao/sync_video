#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication1.h"
#include "setupwindow.h"
#include "opencv2/opencv.hpp"
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
#include <cstdio>
#include "utils.h"

extern "C" {
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavdevice/avdevice.h"
}

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
    cv::VideoCapture capture;
    cv::VideoWriter* camera1_writer;
    cv::VideoWriter* camera2_writer;
    cv::VideoWriter* camera3_writer;

    QTimer* camera_timer;
    QTimer* time_timer;
    cv::Mat camera1_color_frame;
    bool stop_camera, stop_record, can_release;
    QTime start_time;

    QSettings* settings;

    cv::Mat camera2_color_frame;
    cv::Mat camera2_depth_frame;
    cv::Mat camera2_ir_frame;

    bool camera1_show_ready;
    bool camera2_show_ready;

    QString save_path;
    int save_image_count;

    QQueue<cv::Mat> camera1_frames_queue;
    QQueue<cv::Mat> camera2_frames_queue;

    QMutex camera1_frames_mutex;
    QMutex camera2_frames_mutex;
    QWaitCondition camera1_frames_wait;
    QWaitCondition camera2_frames_wait;

    FILE* ffmpeg_pipe1;
    FILE* ffmpeg_pipe2;

    int record_count;
    AVPacket show_pkt;
    k4a_image_t show_color;
    k4a_image_t show_depth;

    QString dirname1, dirname2, dirname3;

    AVCodec* pCodec;
    AVCodecContext* pCodecCtx;
    AVFrame *pFrame, *pFrameRGB;

 
private slots:
    void importFrame();
    void start_botton_clicked();
    void stop_botton_clicked();
    void close_botton_clicked();
    
    void updateTime();
    void openSetupWindow();

    void readCamera();
    void readKinect();

    void camera1_write();
    void camera2_write();
};
