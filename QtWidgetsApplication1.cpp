#include "QtWidgetsApplication1.h"

#include "imu.h"
#include "camera.h"
#include <string>
#include <QTime>
#include <cstdio>




QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    settings = new QSettings("config.ini", QSettings::IniFormat);
    setupwin.set_setttings(settings);

    QAction* action_setup = new QAction(tr("Setup"));
    ui.mainToolBar->addAction(action_setup);
    connect(action_setup, SIGNAL(triggered()), this, SLOT(openSetupWindow()));

    camera_timer = new QTimer(this);
    time_timer = new QTimer(this);

    connect(camera_timer, SIGNAL(timeout()), this, SLOT(importFrame()));
    connect(time_timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    stop_camera = false;
    stop_record = true;
    can_release = false;

    QtConcurrent::run(this, &QtWidgetsApplication1::readCamera);
    QtConcurrent::run(this, &QtWidgetsApplication1::readKinect);
    //camera1();
   
    QThread::msleep(1000);
    camera_timer->start(20);
    time_timer->start(1000);

    camera1_show_ready = false;
    camera2_show_ready = false;
}

QtWidgetsApplication1::~QtWidgetsApplication1() {
    stop_camera = true;
}

void QtWidgetsApplication1::importFrame() {
    /*使用opencv的videowrite写入视频数据的结果是，从摄像机采集数据的速度要明显快于videowrite能处理的速度*/
    /*if (camera2_show_ready && camera1_show_ready) {
        qDebug() << QTime::currentTime().toString() << " - frames";
        camera2_show_ready = false;
        camera1_show_ready = false;
    }*/
    
    if (camera1_show_ready && camera2_show_ready) {
        //qDebug() << QTime::currentTime().toString() << " - frames";
        /*
        cv::cvtColor(camera1_color_frame, camera1_color_frame, cv::COLOR_BGR2RGB);
        cv::cvtColor(camera2_color_frame, camera2_color_frame, cv::COLOR_BGRA2RGB);
        QImage srcQImage1 = QImage((uchar*)(camera1_color_frame.data), camera1_color_frame.cols, camera1_color_frame.rows, QImage::Format_RGB888);
        QImage srcQImage2 = QImage((uchar*)(camera2_color_frame.data), camera2_color_frame.cols, camera2_color_frame.rows, QImage::Format_RGB888);
        

        //cv::cvtColor(camera1_color_frame, camera1_color_frame, cv::COLOR_BGR2RGB);
        //QImage srcQImage1 = QImage((uchar*)(camera1_color_frame.data), camera1_color_frame.cols, camera1_color_frame.rows, QImage::Format_RGB888);
        ui.camera1->setPixmap(QPixmap::fromImage(srcQImage1));
        ui.camera1->setScaledContents(true);
        ui.camera1->show();
        
        //cv::cvtColor(camera2_color_frame, camera2_color_frame, cv::COLOR_BGR2RGB);
        //QImage srcQImage2 = QImage((uchar*)(camera2_color_frame.data), camera2_color_frame.cols, camera2_color_frame.rows, QImage::Format_RGB888);
        //QImage srcQImage2 = QImage((uchar*)(k4a_image_get_buffer(camera2_color_frame)),
        //    k4a_image_get_width_pixels(camera2_color_frame), k4a_image_get_height_pixels(camera2_color_frame), QImage::Format_ARGB32);
        ui.camera2->setPixmap(QPixmap::fromImage(srcQImage2));
        ui.camera2->setScaledContents(true);
        ui.camera2->show();


        QImage srcQImage3 = QImage((uchar*)(camera2_depth_frame.data), camera2_depth_frame.cols, camera2_depth_frame.rows, QImage::Format_RGB16);
        ui.camera3->setPixmap(QPixmap::fromImage(srcQImage3));
        ui.camera3->setScaledContents(true);
        ui.camera3->show();
        */
        QTime qtime;
        qtime.start();
        
        QImage srcQImage1(256, 144, QImage::Format_RGB888);
        JPEGtoRGB(&show_pkt, pCodecCtx, pFrame, &srcQImage1);

        ui.camera1->setPixmap(QPixmap::fromImage(srcQImage1));
        ui.camera1->setScaledContents(true);
        ui.camera1->show();
        
        AVPacket _pkt;
        av_new_packet(&_pkt, k4a_image_get_size(show_color) + 1000);
        //av_packet_from_data(&_pkt, k4a_image_get_buffer(show_color), k4a_image_get_size(show_color));
        memcpy(_pkt.data, k4a_image_get_buffer(show_color), k4a_image_get_size(show_color));
        _pkt.size = k4a_image_get_size(show_color);
        
        QImage srcQImage2(256, 144, QImage::Format_RGB888);
        JPEGtoRGB(&_pkt, pCodecCtx, pFrame, &srcQImage2);

        ui.camera2->setPixmap(QPixmap::fromImage(srcQImage2));
        ui.camera2->setScaledContents(true);
        ui.camera2->show();

        av_packet_unref(&_pkt);
        av_free_packet(&_pkt);
        
        
        if (!stop_record) {
            QString filename1 = dirname1 + "\\" + QString().sprintf("%06d.jpeg", record_count);
            QFile file1(filename1);
            file1.open(QIODevice::WriteOnly);
            file1.write((char*)show_pkt.data, show_pkt.size);
            file1.close();
            
            QString filename2 = dirname2 + "\\" + QString().sprintf("%06d.jpeg", record_count);
            QFile file2(filename2);
            file2.open(QIODevice::WriteOnly);
            file2.write((char*)k4a_image_get_buffer(show_color), k4a_image_get_size(show_color));
            file2.close();

            QString filename3 = dirname3 + "\\" + QString().sprintf("%06d.dep", record_count);
            QFile file3(filename3);
            file3.open(QIODevice::WriteOnly);
            file3.write((char*)k4a_image_get_buffer(show_depth), k4a_image_get_size(show_depth));
            file3.close();

            record_count++;
        }
        if (!stop_record && can_release) {

        }

        av_packet_unref(&show_pkt);
        k4a_image_release(show_color);
        k4a_image_release(show_depth);
        camera1_show_ready = false;
        camera2_show_ready = false;

        qDebug() << qtime.elapsed();
    }
    
    
    /*
    QImage srcQImage3 = QImage((uchar*)(k4a_image_get_buffer(camera2_depth_frame)),
        k4a_image_get_width_pixels(camera2_depth_frame), k4a_image_get_height_pixels(camera2_depth_frame), QImage::Format_ARGB4444_Premultiplied);
    ui.camera3->setPixmap(QPixmap::fromImage(srcQImage3));
    ui.camera3->setScaledContents(true);
    ui.camera3->show();

    QImage srcQImage4 = QImage((uchar*)(k4a_image_get_buffer(camera2_ir_frame)),
        k4a_image_get_width_pixels(camera2_ir_frame), k4a_image_get_height_pixels(camera2_ir_frame), QImage::Format_ARGB4444_Premultiplied);
    ui.camera4->setPixmap(QPixmap::fromImage(srcQImage4));
    ui.camera4->setScaledContents(true);
    ui.camera4->show();*/
}

void QtWidgetsApplication1::start_botton_clicked() {
    if (!stop_record) return;

    QString name = ui.lineEdit->text();
    name += QTime::currentTime().toString();
    save_path = settings->value("save_dir").toString();
    //save_path += "\\out.avi";
    save_path += "out";

    QDir dir;
    if (!dir.exists(save_path)) {
        dir.mkdir(save_path);
    }
    qDebug() << save_path << endl;

    dirname1 = save_path + "\\camera1";
    dirname2 = save_path + "\\kinect_color";
    dirname3 = save_path + "\\kinect_depth";
    if (!dir.exists(dirname1)) {
        dir.mkdir(dirname1);
    }
    if (!dir.exists(dirname2)) {
        dir.mkdir(dirname2);
    }
    if (!dir.exists(dirname3)) {
        dir.mkdir(dirname3);
    }
    clearFiles(dirname1);
    clearFiles(dirname2);
    clearFiles(dirname3);
    

    stop_record = false;
    can_release = false;
    record_count = 0;

    start_time = QTime::currentTime();
    
}

void QtWidgetsApplication1::stop_botton_clicked() {
    stop_record = true;
    can_release = true;
}

void QtWidgetsApplication1::close_botton_clicked() {
    stop_camera = true;
    camera_timer->stop();
}



void QtWidgetsApplication1::readCamera() {
    AVFormatContext* pFormatCtx = NULL;
    int i, videoindex;
    AVDictionary* options = NULL;
    //av_dict_set(&options, "fflags", "nobuffer", 0);
    //av_dict_set(&options, "max_delay", "100000", 0);
    av_dict_set(&options, "input_format", "mjpeg", 0);
    av_dict_set(&options, "framerate", "30", 0);
    av_dict_set(&options, "video_size", "1920x1080", 0);
    av_dict_set(&options, "rtbufsize", "1000M", 0);

    avformat_network_init();
    avdevice_register_all();

    std::vector<std::string> list;
    listDevices(list);
    int index = 0;
    for (int i = 0; i < list.size(); i++)
    {
        std::cout << "device lists:" << list[i] << '\t' << "i=" << i << std::endl;
        if (list[i] == "USB CAMERA") {
            index = i;
            qDebug() << "Will open USB CAMERA";
        }
    }

    pFormatCtx = avformat_alloc_context();

    //在windows平台下最好使用dshow方式
    AVInputFormat* ifmt = av_find_input_format("dshow");
    //Set own video device's name
    //摄像头名称
    if (avformat_open_input(&pFormatCtx, "video=USB CAMERA", ifmt, &options) != 0)
    {
        qDebug() << "Couldn't open input stream.";
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug() << "Couldn't find stream information.\n";
        return;
    }

    //AVCodec* pCodec;
    //AVCodecContext* pCodecCtx;

    // 获取输入ctx
    for (i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            index = i;
            break;
        }
    }

    av_dump_format(pFormatCtx, 0, "USB CAMERA", 0);

    // 查找输入解码器
    pCodec = avcodec_find_decoder(pFormatCtx->streams[index]->codecpar->codec_id);
    if (!pCodec)
    {
        qDebug() << "can't find codec";
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        return ;
    }

    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[index]->codecpar);

    //  1.5 打开输入解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("can't open codec\n");
        return;
    }

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    AVPacket pkt;
    int ret = 0;
    while (!stop_camera) {
        ret = av_read_frame(pFormatCtx, &pkt);
        if (ret < 0) {
            break;
        }
        if (!camera1_show_ready) {
            av_packet_move_ref(&show_pkt, &pkt);
            camera1_show_ready = true;
        }
        av_packet_unref(&pkt);
    }

    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avformat_close_input(&pFormatCtx);

}

void QtWidgetsApplication1::updateTime() {
    qint32 sec = 0, min = 0;
    if (!stop_record) {
        sec = start_time.secsTo(QTime::currentTime());
        min = sec / 60;
        sec = sec % 60;
    }
    QString timestr;
    timestr.sprintf("%02d:%02d", min, sec);
    ui.time1->setText(timestr);
    ui.time1->show();
}

void QtWidgetsApplication1::openSetupWindow() {
    setupwin.show();

}

void QtWidgetsApplication1::readKinect() {
    uint32_t count = k4a_device_get_installed_count();;
    if (count == 0) {
        qDebug() << "No k4a devices attached!" << endl;
        return;
    }
    k4a_device_t device = NULL;
    if (K4A_FAILED(k4a_device_open(K4A_DEVICE_DEFAULT, &device))) {
        qDebug() << "Failed to open k4a device!" << endl;
        return;
    }
    size_t serial_size = 0;
    k4a_device_get_serialnum(device, NULL, &serial_size);
    char* serial = (char*)(malloc(serial_size));
    k4a_device_get_serialnum(device, serial, &serial_size);
    qDebug() << "Opened device: " << serial << endl;
    free(serial);

    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
    config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
    config.depth_mode = K4A_DEPTH_MODE_WFOV_2X2BINNED;

    if (K4A_FAILED(k4a_device_start_cameras(device, &config))) {
        qDebug()<<"Failed to start cameras!"<<endl;
        k4a_device_close(device);
        return;
    }
    k4a_capture_t capture = NULL;

    int count2 = 0;
    while (!stop_camera) {
        switch (k4a_device_get_capture(device, &capture, 1000)) {
        case K4A_WAIT_RESULT_SUCCEEDED:
            break;
        case K4A_WAIT_RESULT_TIMEOUT:
            qDebug() << "Timed out waiting for a capture." << endl;
            continue;
            break;
        case K4A_WAIT_RESULT_FAILED:
            qDebug() << "Failed to read a capture." << endl;
            k4a_device_stop_cameras(device);
            k4a_device_close(device);
            return;
        }
        k4a_image_t color_frame = k4a_capture_get_color_image(capture);
        k4a_image_t depth_frame = k4a_capture_get_depth_image(capture);
        //k4a_image_t ir_frame = k4a_capture_get_ir_image(capture);
        
        //TODO
        if (!color_frame||!depth_frame) {
            qDebug() << "Get image failed." << endl;
            k4a_capture_release(capture);
            continue;
        }

        if (!camera2_show_ready) {
            show_color = color_frame;
            k4a_image_reference(show_color);
            show_depth = depth_frame;
            k4a_image_reference(show_depth);
            camera2_show_ready = true;
        }

        k4a_image_release(color_frame);
        k4a_image_release(depth_frame);
        k4a_capture_release(capture);
    }

    

    k4a_device_stop_cameras(device);
    k4a_device_close(device);
    return;
}


void QtWidgetsApplication1::camera1_write() {
    int count1 = 0;
    char save_name[32];
    while (true) {
        if (stop_record && camera1_frames_queue.empty()) {
            camera1_writer->release();
            return;
        }
        camera1_frames_mutex.lock();
        while (camera1_frames_queue.empty()) {
            camera1_frames_wait.wait(&camera1_frames_mutex);
            if (stop_record && camera1_frames_queue.empty()) continue;
        }
        cv::Mat mat = camera1_frames_queue.front();
        camera1_frames_queue.pop_front();
        camera1_frames_mutex.unlock();
        //cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        /*QTime qtime;
        qtime.start();
        //camera1_writer->write(mat);
        sprintf(save_name, "out\\tmp%05d", count1);
        FILE* file = fopen(save_name, "wb");
        fwrite(mat.data, 1, mat.total()*mat.elemSize(), file);
        fclose(file);
        qDebug() << qtime.elapsed();*/
        //qDebug() << "Camera1 write frames " << count1++;
    }
    
}


void QtWidgetsApplication1::camera2_write() {
    /*std::string command = "ffmpeg -y -f rawvideo -vcodec mjpeg -s 1920x1080 -framerate 30 -i - -vcodec copy test.avi";
    FILE* pipeout = _popen(command.data(), "w");*/
    int count2 = 0;
    while (true) {
        if (stop_record && camera2_frames_queue.empty()) {
            camera2_writer->release();
            return;
        }
        camera2_frames_mutex.lock();
        while (camera2_frames_queue.empty()) {
            camera2_frames_wait.wait(&camera2_frames_mutex);
            if (stop_record && camera2_frames_queue.empty()) continue;
        }
        cv::Mat mat = camera2_frames_queue.front();
        camera2_frames_queue.pop_front();
        camera2_frames_mutex.unlock();
        
        //cv::cvtColor(mat, mat, cv::COLOR_YUV2BGR_YUY2);
        //camera2_writer->write(mat);
        
        //qDebug() << "Camera2 write frames " << count2++;
    }
}