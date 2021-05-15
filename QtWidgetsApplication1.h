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

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = Q_NULLPTR);

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


 
private slots:
    void importFrame();
    void start_botton_clicked();
    void stop_botton_clicked();
    void close_botton_clicked();
    void readCamera();
    void updateTime();
    void openSetupWindow();

    void readKinect();

    void camera1_write();
    void camera2_write();
};
