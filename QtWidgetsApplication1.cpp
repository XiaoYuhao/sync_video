#include "QtWidgetsApplication1.h"

#include "camera.h"
#include <string>
#include <QTime>
#include <QMessageBox>
#include <QFileDialog>
#include <cstdio>
#include <windows.h>


using namespace zen;


QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    settings = new QSettings("config.ini", QSettings::IniFormat);
    setupwin.set_setttings(settings);
    setWindowTitle("DataCollector");

    qRegisterMetaType<std::vector<std::string>*>();
    //qRegisterMetaType<std::vector<ZenImuData>*>();
    qRegisterMetaType<ZenImuData>();

    QAction* action_setup = new QAction(tr("Setup"));
    ui.mainToolBar->addAction(action_setup);
    connect(action_setup, SIGNAL(triggered()), this, SLOT(openSetupWindow()));

    ui.progressBar->setVisible(false);
    ui.progressLabel->setVisible(false);
    //ui.sensortable->setVisible(false);

    show_timer = new QTimer(this);
    write_timer = new QTimer(this);
    time_timer = new QTimer(this);

    //connect(show_timer, SIGNAL(timeout()), this, SLOT(importFrame()));
    //connect(write_timer, SIGNAL(timeout()), this, SLOT(writeFrame()));
    connect(time_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    connect(this, SIGNAL(ready_to_show()), this, SLOT(importFrame()));
    connect(this, SIGNAL(ready_to_initimu(std::vector<std::string>*)), this, SLOT(initSensorTable(std::vector<std::string>*)));
    //connect(this, SIGNAL(ready_to_updateimu(std::vector<ZenImuData>*)), this, SLOT(updateSensorTable(std::vector<ZenImuData>*)));
    connect(this, SIGNAL(ready_to_updateimu(ZenImuData, int)), this, SLOT(updateSensorTable(ZenImuData, int)));
    connect(this, SIGNAL(ready_to_appendlog(QString)), this, SLOT(appendLog(QString)));

    connect(this, SIGNAL(start_record_signal()), this, SLOT(start_botton_clicked()));
    connect(this, SIGNAL(stop_record_signal()), this, SLOT(stop_botton_clicked()));

    pFrame = av_frame_alloc();

    save_path = settings->value("save_dir").toString();
    if (save_path == "") emit ready_to_appendlog(QString().fromLocal8Bit("警告：没有选择保存的目录，请前往左上方设置中选择保存数据的目录."));
    stop_camera = false;
    stop_record = true;

    QtConcurrent::run(this, &QtWidgetsApplication1::readCamera);
    QtConcurrent::run(this, &QtWidgetsApplication1::readKinect);
    QtConcurrent::run(this, &QtWidgetsApplication1::listenPipe);
    //QtConcurrent::run(this, &QtWidgetsApplication1::initImuSensor);
    //camera1();
    //imu_test();

    QThread::msleep(1000);
    write_timer->start(10);
    show_timer->start(33);
    time_timer->start(1000);

    camera1_show_ready = false;
    camera2_show_ready = false;

}

QtWidgetsApplication1::~QtWidgetsApplication1() {
    stop_camera = true;
    av_frame_free(&pFrame);
}

void QtWidgetsApplication1::importFrame() {
    /*使用opencv的videowrite写入视频数据的结果是，从摄像机采集数据的速度要明显快于videowrite能处理的速度*/
    /*if (camera2_show_ready && camera1_show_ready) {
        qDebug() << QTime::currentTime().toString() << " - frames";
        camera2_show_ready = false;
        camera1_show_ready = false;
    }*/
    if (camera1_show_ready) {
        QImage srcQImage1(256, 144, QImage::Format_RGB888);
        JPEGtoRGB(&camera1_show_pkt, camera1_pCodecCtx, pFrame, &srcQImage1);

        ui.camera1->setPixmap(QPixmap::fromImage(srcQImage1));
        ui.camera1->setScaledContents(true);
        ui.camera1->show();


        av_packet_unref(&camera1_show_pkt);
        camera1_show_ready = false;
    }
    if (camera2_show_ready) {
        av_new_packet(&camera2_show_pkt, k4a_image_get_size(show_color));
        memcpy(camera2_show_pkt.data, k4a_image_get_buffer(show_color), k4a_image_get_size(show_color));
        camera2_show_pkt.size = k4a_image_get_size(show_color);

        QImage srcQImage2(256, 144, QImage::Format_RGB888);
        JPEGtoRGB(&camera2_show_pkt, camera2_pCodecCtx, pFrame, &srcQImage2);

        ui.camera2->setPixmap(QPixmap::fromImage(srcQImage2));
        ui.camera2->setScaledContents(true);
        ui.camera2->show();

        av_packet_unref(&camera2_show_pkt);
        av_free_packet(&camera2_show_pkt);
        k4a_image_release(show_color);
        camera2_show_ready = false;
    }
}

void QtWidgetsApplication1::start_botton_clicked() {
    if (!stop_record) return;
    if (save_path == "") {
        QMessageBox::warning(NULL, QString().fromLocal8Bit("提示"), QString().fromLocal8Bit("未选择保存文件夹！"));
        return;
    }
    //save_path = settings->value("save_dir").toString();
    //save_path += "\\out.avi";
    //save_path += "out";

    QDir dir;
    if (!dir.exists(save_path)) {
        dir.mkdir(save_path);
    }
    qDebug() << save_path << endl;

    dirname1 = save_path + "\\camera1";
    dirname2 = save_path + "\\kinect_color";
    dirname3 = save_path + "\\kinect_depth";
    //dirname4 = save_path + "\\imu_data";
    if (!dir.exists(dirname1)) {
        dir.mkdir(dirname1);
    }
    if (!dir.exists(dirname2)) {
        dir.mkdir(dirname2);
    }
    if (!dir.exists(dirname3)) {
        dir.mkdir(dirname3);
    }
    /*if (!dir.exists(dirname4)) {
        dir.mkdir(dirname4);
    }*/
    clearFiles(dirname1);
    clearFiles(dirname2);
    clearFiles(dirname3);
    //clearFiles(dirname4);

    /*
    imu_files.clear();
    imu_outstreams.clear();
    for (int i = 0; i < sensors_name.size(); i++) {
        QFile *file = new QFile(save_path + "\\" + QString().fromStdString(sensors_name[i]) + ".csv");
        file->open(QIODevice::WriteOnly);
        QTextStream *out = new QTextStream(file);
        writeCsvHeader(*out);
        imu_files.push_back(file);
        imu_outstreams.push_back(out);
    }*/

    stop_record = false;
    can_write_imudata = false;
    record_calibration = false;
    record1_count = 0;
    record2_count = 0;

    //QString cmd1 = "ffmpeg -f dshow -i audio=\"Microphone Array (Azure Kinect Microphone Array)\" " + save_path + "\\audio.mp3";
    //QString cmd2 = QString().fromLocal8Bit("ffmpeg -f dshow -i audio=\"麦克风阵列 (Azure Kinect Microphone Array)\" ") + save_path + "\\audio.mp3";
    QString cmd3 = QString().fromLocal8Bit("ffmpeg -f dshow -i audio=\"麦克风阵列 (2- Azure Kinect Microphone Array)\" ") + save_path + "\\audio.mp3";
    qDebug() << cmd3 << endl;
    //cmd.toLocal8Bit().data();
    if ((fp3 = _popen(cmd3.toLocal8Bit().data(), "w")) == NULL) {
        qDebug() << "Open ffmpeg recored audio pipe failed." << endl;
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：录制音频失败."));
    }
    //if (((fp1 = _popen(cmd1.toLocal8Bit().data(), "w")) == NULL)&&((fp2 = _popen(cmd2.toLocal8Bit().data(), "w")) == NULL)&& ((fp3 = _popen(cmd3.toLocal8Bit().data(), "w")) == NULL)) {
    //    qDebug() << "Open ffmpeg recored audio pipe failed." << endl;
    //    emit ready_to_appendlog(QString().fromLocal8Bit("错误：录制音频失败."));
    //}

    start_time = QTime::currentTime();

}

void QtWidgetsApplication1::stop_botton_clicked() {
    stop_record = true;
    //fwrite("q", sizeof(char), 1, fp1);
    //fwrite("q", sizeof(char), 1, fp2);
    fwrite("q", sizeof(char), 1, fp3);
    //_pclose(fp1);
    //_pclose(fp2);
    _pclose(fp3);
    /*
    for (auto& file : imu_files) {
        file->close();
        delete file;
    }
    for (auto& stream : imu_outstreams) {
        delete stream;
    }
    imu_files.clear();
    imu_outstreams.clear();*/

    /*
    QPixmap pixmap;
    pixmap.load("resource\\tongji.jpeg");
    ui.camera1->setPixmap(pixmap);
    ui.camera1->setScaledContents(true);
    ui.camera1->show();
    ui.camera2->setPixmap(pixmap);
    ui.camera2->setScaledContents(true);
    ui.camera2->show();

    ui.progressBar->setVisible(true);
    ui.progressLabel->setVisible(true);

    createVideo(dirname1, save_path + "\\camera.avi");
    createVideo(dirname2, save_path + "\\kinect_color.avi");

    QThread::msleep(1000);
    ui.progressBar->setVisible(false);
    ui.progressLabel->setVisible(false);
    */
}

void QtWidgetsApplication1::close_botton_clicked() {
    stop_camera = true;
    show_timer->stop();
    write_timer->stop();
    time_timer->stop();
}

/*
void QtWidgetsApplication1::tool_botton_clicked() {
    save_path = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Save Path"), QDir::currentPath()));
    ui.dirlabel->setText(save_path);
}*/

void QtWidgetsApplication1::listenPipe() {
    while (true) {
        HANDLE hpipe = NULL;
        hpipe = CreateNamedPipe(L"\\\\.\\pipe\\signalpipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
        if (hpipe == INVALID_HANDLE_VALUE) {
            std::cout << "Open pipe error" << std::endl;
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：与主程序建立通信失败."));
            return;
        }
        if (!ConnectNamedPipe(hpipe, NULL)) {
            std::cout << "Connect Failed." << std::endl;
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：与主程序建立通信失败."));
            break;
        }
        char szBuffer[1024] = { 0 };
        DWORD dwReturn = 0;

        while (true) {
            if (ReadFile(hpipe, szBuffer, 1024, &dwReturn, NULL)) {
                std::cout << szBuffer << std::endl;
                //std::string str(szBuffer);
                if (szBuffer[0] == '0') {
                    emit ready_to_appendlog(QString().fromLocal8Bit("提示：结束录制视频..."));
                    emit stop_record_signal();
                }
                else if (szBuffer[0] == '1') {
                    QString fname = QString::fromUtf8(szBuffer + 2);
                    save_path = settings->value("save_dir").toString();
                    if (save_path == "") {
                        emit ready_to_appendlog(QString().fromLocal8Bit("警告：没有选择保存的目录，数据将被暂时存放D:\\temp目录下"));
                        save_path = "D:\\temp";
                    }
                    save_path += "\\" + fname; \
                        emit ready_to_appendlog(QString().fromLocal8Bit("提示：开始录制视频..."));
                    emit ready_to_appendlog(QString().fromLocal8Bit("提示：数据保存在目录") + save_path);
                    emit start_record_signal();

                }

            }
            else {
                std::cout << "Read Failed." << std::endl;
                break;
            }
        }
    }
    /*
    HANDLE hpipe = NULL;
    char szBuffer[1024] = { 0 };
    DWORD dwReturn = 0;
    while (!WaitNamedPipe(L"\\\\.\\pipe\\videocollector", NMPWAIT_USE_DEFAULT_WAIT)) {
        qDebug() << "No read pipe accessible." << endl;
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：未找到主程序，请检查主程序是否启动."));
        QThread::sleep(1);
    }
    hpipe = CreateFile(L"\\\\.\\pipe\\videocollector", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hpipe == INVALID_HANDLE_VALUE) {
        qDebug() << "Open read pipe error." << endl;
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：打开消息管道失败，请尝试重启程序."));
        return;
    }
    ui.StartButton->setDisabled(true);
    ui.StopButton->setDisabled(true);
    while (true) {
        if (ReadFile(hpipe, szBuffer, 1024, &dwReturn, NULL)) {
            //szBuffer[dwReturn] = '\0';
            if (szBuffer[0] == '0') {
                qDebug() << "Recv a stop signal from server" << endl;
                emit ready_to_appendlog(QString().fromLocal8Bit("提示：结束录制视频..."));
                //emit stop_record_signal();
            }
            else if (szBuffer[0] == '1') {
                qDebug() << "Recv a start signal from server" << endl;
                emit ready_to_appendlog(QString().fromLocal8Bit("提示：开始录制视频..."));
                std::string name = szBuffer + 3;
                save_path = settings->value("save_dir").toString();
                if(save_path == ""){
                    emit ready_to_appendlog(QString().fromLocal8Bit("警告：没有选择保存的目录，数据将被暂时存放D:\\temp\\目录下"));
                    save_path = "D:\\temp\\";
                }
                save_path += QString::fromStdString(name);
                //emit start_record_signal();
            }
        }
        else {
            qDebug() << "Read pipe error." << endl;
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：读取管道消息出错."));
            break;
        }
    }
    */
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
    av_log_set_level(AV_LOG_ERROR);

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
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：USB相机无法打开，请检查连接是否正常."));
        return;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug() << "Couldn't find stream information.";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：USB相机无法找到流信息."));
        return;
    }
    for (int i = 0; i < pFormatCtx->nb_streams; ++i)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            index = i;
            break;
        }
    }

    av_dump_format(pFormatCtx, 0, "USB CAMERA", 0);

    // 查找输入解码器
    camera1_pCodec = avcodec_find_decoder(pFormatCtx->streams[index]->codecpar->codec_id);
    if (!camera1_pCodec)
    {
        qDebug() << "can't find codec";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：USB相机无法找到解码器."));
        return;
    }

    camera1_pCodecCtx = avcodec_alloc_context3(camera1_pCodec);
    if (!camera1_pCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：USB相机无法为解码器分配空间."));
        return;
    }

    avcodec_parameters_to_context(camera1_pCodecCtx, pFormatCtx->streams[index]->codecpar);

    //  1.5 打开输入解码器
    if (avcodec_open2(camera1_pCodecCtx, camera1_pCodec, NULL) < 0)
    {
        qDebug() << "can't open codec";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：USB相机无法打开解码器."));
        return;
    }
    emit ready_to_appendlog(QString().fromLocal8Bit("提示：USB相机初始化成功."));
    AVPacket pkt;
    int ret = 0;
    while (!stop_camera) {
        ret = av_read_frame(pFormatCtx, &pkt);
        if (ret < 0) {
            break;
        }
        if (!camera1_show_ready) {
            //av_packet_move_ref(&camera1_show_pkt, &pkt);
            av_packet_ref(&camera1_show_pkt, &pkt);
            camera1_show_ready = true;
            emit ready_to_show();
        }
        if (!stop_record) {
            QString filename1 = dirname1 + "\\" + QString().sprintf("%06d.jpeg", record1_count);
            QFile file1(filename1);
            file1.open(QIODevice::WriteOnly);
            file1.write((char*)pkt.data, pkt.size);
            file1.close();
            record1_count++;
        }
        av_packet_unref(&pkt);
    }

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
    setupwin.show_current_dir();
    setupwin.show();

}

void QtWidgetsApplication1::readKinect() {
    uint32_t count = k4a_device_get_installed_count();;
    if (count == 0) {
        qDebug() << "No k4a devices attached!" << endl;
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法找到Kinect设备，请检查设备是否正常连接."));
        return;
    }
    k4a_device_t device = NULL;
    if (K4A_FAILED(k4a_device_open(K4A_DEVICE_DEFAULT, &device))) {
        qDebug() << "Failed to open k4a device!" << endl;
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法打开Kinect设备，请检查设备是否正常运行（电源、USB连接）."));
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

    // 查找输入解码器
    camera2_pCodec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    if (!camera2_pCodec)
    {
        qDebug() << "kinect can't find codec";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：Kinect无法找到解码器."));
        return;
    }

    camera2_pCodecCtx = avcodec_alloc_context3(camera2_pCodec);
    if (!camera2_pCodecCtx)
    {
        qDebug() << "kinect can't alloc codec context";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：Kinect无法为解码器分配空间."));
        return;
    }

    camera2_pCodecCtx->codec_id = AV_CODEC_ID_MJPEG;
    camera2_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    camera2_pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ422P;
    camera2_pCodecCtx->width = 1920;
    camera2_pCodecCtx->height = 1080;

    //qDebug() << pCodecCtx->codec_id << " - " << pCodecCtx->codec_type << " - " << pCodecCtx->pix_fmt;

    //  1.5 打开输入解码器
    if (avcodec_open2(camera2_pCodecCtx, camera2_pCodec, NULL) < 0)
    {
        qDebug() << "kinect can't open codec";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：Kinect无法打开解码器."));
        return;
    }

    if (K4A_FAILED(k4a_device_start_cameras(device, &config))) {
        qDebug() << "Failed to start cameras!" << endl;
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：Kinect打开相机失败."));
        k4a_device_close(device);
        return;
    }
    k4a_capture_t capture = NULL;
    emit ready_to_appendlog(QString().fromLocal8Bit("提示：Kinect初始化成功."));

    /* 保存校准值
    size_t data_size = 0;
    k4a_device_get_raw_calibration(device, NULL, &data_size);
    data_size += 10;
    uint8_t* calibration_data = new uint8_t[data_size];
    if (K4A_BUFFER_RESULT_SUCCEEDED == k4a_device_get_raw_calibration(device, calibration_data, &data_size)) {
        QString calibration_file_name = save_path + "\\kinect_calibration";
        QFile calibration_file(calibration_file_name);
        calibration_file.open(QIODevice::WriteOnly);
        calibration_file.write((char*)calibration_data, data_size);
        calibration_file.close();
        QString calibration_cfg_name = save_path + "\\kinect_calibration_cfg";
        QFile calibration_cfg_file(calibration_cfg_name);
        calibration_cfg_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream txtOutput(&calibration_cfg_file);
        txtOutput << "raw calibration data size: " << data_size << endl;
        calibration_cfg_file.close();
        emit ready_to_appendlog(QString().fromLocal8Bit("提示：Kinect相机校准值提取成功."));
    }
    else {
        emit ready_to_appendlog(QString().fromLocal8Bit("警告：Kinect相机校准值提取失败."));
    }
    */

    while (!stop_camera) {
        switch (k4a_device_get_capture(device, &capture, 1000)) {
        case K4A_WAIT_RESULT_SUCCEEDED:
            break;
        case K4A_WAIT_RESULT_TIMEOUT:
            qDebug() << "Timed out waiting for a capture." << endl;
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：Kinect等待图像数据超时."));
            continue;
            break;
        case K4A_WAIT_RESULT_FAILED:
            qDebug() << "Failed to read a capture." << endl;
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：Kinect读取图像数据失败."));
            k4a_device_stop_cameras(device);
            k4a_device_close(device);
            return;
        }
        k4a_image_t color_frame = k4a_capture_get_color_image(capture);
        k4a_image_t depth_frame = k4a_capture_get_depth_image(capture);
        //k4a_image_t ir_frame = k4a_capture_get_ir_image(capture);

        if (record2_count == 1) {
            qDebug() << "depth image stride : " << k4a_image_get_stride_bytes(depth_frame) << endl;
        }

        //TODO
        if (!color_frame || !depth_frame) {
            qDebug() << "Get image failed." << endl;
            k4a_capture_release(capture);
            continue;
        }

        if (!camera2_show_ready) {
            show_color = color_frame;
            k4a_image_reference(show_color);
            camera2_show_ready = true;
            emit ready_to_show();
        }

        if (!record_calibration && !stop_record) {
            record_calibration = true;
            size_t data_size = 0;
            k4a_device_get_raw_calibration(device, NULL, &data_size);
            data_size += 10;
            uint8_t* calibration_data = new uint8_t[data_size];
            if (K4A_BUFFER_RESULT_SUCCEEDED == k4a_device_get_raw_calibration(device, calibration_data, &data_size)) {
                QString calibration_file_name = save_path + "\\kinect_calibration";
                QFile calibration_file(calibration_file_name);
                calibration_file.open(QIODevice::WriteOnly);
                calibration_file.write((char*)calibration_data, data_size);
                calibration_file.close();
                QString calibration_cfg_name = save_path + "\\kinect_calibration_cfg";
                QFile calibration_cfg_file(calibration_cfg_name);
                calibration_cfg_file.open(QIODevice::WriteOnly | QIODevice::Text);
                QTextStream txtOutput(&calibration_cfg_file);
                txtOutput << "raw calibration data size: " << data_size << endl;
                calibration_cfg_file.close();
                emit ready_to_appendlog(QString().fromLocal8Bit("提示：Kinect相机校准值提取成功."));
            }
            else {
                emit ready_to_appendlog(QString().fromLocal8Bit("警告：Kinect相机校准值提取失败."));
            }
        }

        if (!stop_record) {
            can_write_imudata = true;

            QString filename2 = dirname2 + "\\" + QString().sprintf("%06d.jpeg", record2_count);
            QFile file2(filename2);
            file2.open(QIODevice::WriteOnly);
            file2.write((char*)k4a_image_get_buffer(color_frame), k4a_image_get_size(color_frame));
            file2.close();

            QString filename3 = dirname3 + "\\" + QString().sprintf("%06d.dep", record2_count);
            QFile file3(filename3);
            file3.open(QIODevice::WriteOnly);
            file3.write((char*)k4a_image_get_buffer(depth_frame), k4a_image_get_size(depth_frame));
            file3.close();

            record2_count++;
        }

        k4a_image_release(color_frame);
        k4a_image_release(depth_frame);
        k4a_capture_release(capture);
    }



    k4a_device_stop_cameras(device);
    k4a_device_close(device);
    return;
}



int QtWidgetsApplication1::createVideo(const QString& dirname, const QString& outfile) {
    int filescount = subFilescount(dirname);
    if (filescount == 0) return 0;
    qDebug() << "The number of jpeg images is " << filescount;

    emit ready_to_appendlog(QString().fromLocal8Bit("提示：开始编码视频."));

    AVFormatContext* ifmtCtx = NULL;
    AVFormatContext* ofmtCtx = NULL;
    AVPacket pkt;
    AVFrame* imageFrame, * imageFrameYUV;
    SwsContext* pImgConvertCtx;
    AVDictionary* params = NULL;
    AVCodec* imageCodec;
    AVCodecContext* imageCodecCtx;
    unsigned char* outBuffer;
    AVCodecContext* videoCodecCtx;
    AVCodec* videoCodec;
    AVDictionary* options = NULL;
    int index = 0, ret = 0;

    avdevice_register_all();

    ui.progressBar->setRange(0, filescount - 1);
    ui.progressBar->reset();

    QString filename = dirname + "\\000000.jpeg";
    if (avformat_open_input(&ifmtCtx, filename.toStdString().c_str(), NULL, NULL) != 0) {
        qDebug() << "Can't open input file.";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法打开图像文件."));
        goto end;
    }

    if (avformat_find_stream_info(ifmtCtx, NULL) < 0)
    {
        qDebug() << "Couldn't find stream information.\n";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法找到流信息."));
        goto end;
    }
    for (int i = 0; i < ifmtCtx->nb_streams; ++i)
    {
        if (ifmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            index = i;
            break;
        }
    }

    av_dump_format(ifmtCtx, 0, NULL, 0);

    // 查找输入解码器
    imageCodec = avcodec_find_decoder(ifmtCtx->streams[index]->codecpar->codec_id);
    if (!imageCodec)
    {
        qDebug() << "can't find codec";
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法找到解码器."));
        goto end;
    }

    imageCodecCtx = avcodec_alloc_context3(imageCodec);
    if (!imageCodecCtx)
    {
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法分配解码器所需内存."));
        qDebug() << "can't alloc codec context";
        goto end;
    }

    avcodec_parameters_to_context(imageCodecCtx, ifmtCtx->streams[index]->codecpar);
    //qDebug() << imageCodecCtx->codec_id << " - " << imageCodecCtx->codec_type << " - " << imageCodecCtx->pix_fmt;
    //  1.5 打开输入解码器
    if (avcodec_open2(imageCodecCtx, imageCodec, NULL) < 0)
    {
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法打开解码器."));
        qDebug() << "can't open codec";
        goto end;
    }

    imageFrame = av_frame_alloc();
    imageFrameYUV = av_frame_alloc();

    videoCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if (!videoCodec) {
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法找到视频编码信息."));
        qDebug() << "Can't fine  codec.";
        goto end;
    }

    videoCodecCtx = avcodec_alloc_context3(videoCodec);
    videoCodecCtx->codec_id = AV_CODEC_ID_MPEG4;
    videoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    videoCodecCtx->width = imageCodecCtx->width;
    videoCodecCtx->height = imageCodecCtx->height;
    videoCodecCtx->time_base.num = 1;
    videoCodecCtx->time_base.den = 30;
    videoCodecCtx->bit_rate = 5000000;
    videoCodecCtx->gop_size = 20;
    videoCodecCtx->qmin = 10;
    videoCodecCtx->qmax = 51;

    if (videoCodecCtx->flags & AVFMT_GLOBALHEADER)
    {
        videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    av_dict_set(&params, "preset", "superfast", 0);
    av_dict_set(&params, "tune", "zerolatency", 0);	//实现实时编码
    if (avcodec_open2(videoCodecCtx, videoCodec, &params) < 0)
    {
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法打开视频编码器."));
        qDebug() << "can't open video encoder.";
        goto end;
    }

    avformat_alloc_output_context2(&ofmtCtx, NULL, NULL, outfile.toStdString().c_str());
    if (!ofmtCtx) {
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法创建输出文件."));
        qDebug() << "Can't create output context.";
        goto end;
    }

    for (int i = 0; i < ifmtCtx->nb_streams; ++i)
    {
        AVStream* outStream = avformat_new_stream(ofmtCtx, NULL);
        if (!outStream)
        {
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法创建输出文件."));
            qDebug() << "failed to allocate output stream";
            goto end;
        }

        avcodec_parameters_from_context(outStream->codecpar, videoCodecCtx);
    }

    av_dump_format(ofmtCtx, 0, outfile.toStdString().c_str(), 1);

    if (!(ofmtCtx->oformat->flags & AVFMT_NOFILE))
    {
        // 2.3 创建并初始化一个AVIOContext, 用以访问URL（outFilename）指定的资源
        ret = avio_open(&ofmtCtx->pb, outfile.toStdString().c_str(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法创建输出文件."));
            qDebug() << "can't open output URL: " << outfile.toStdString().c_str();
            goto end;
        }
    }

    AVDictionary* opt = NULL;
    av_dict_set_int(&opt, "video_track_timescale", 30, 0);
    ret = avformat_write_header(ofmtCtx, NULL);
    if (ret < 0)
    {
        emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法创建输出文件."));
        qDebug() << "Error accourred when opening output file";
        goto end;
    }

    imageFrame = av_frame_alloc();
    imageFrameYUV = av_frame_alloc();

    AVPixelFormat convertFormat = AV_PIX_FMT_YUV420P;
    /*if (imageCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ420P) {
        convertFormat = AV_PIX_FMT_YUV420P;
    }
    else if (imageCodecCtx->pix_fmt == AV_PIX_FMT_YUVJ422P) {
        convertFormat = AV_PIX_FMT_YUV422P;
    }*/

    outBuffer = (unsigned char*)av_malloc(
        av_image_get_buffer_size(AV_PIX_FMT_YUV420P, imageCodecCtx->width,
            imageCodecCtx->height, 1));
    av_image_fill_arrays(imageFrameYUV->data, imageFrameYUV->linesize, outBuffer,
        convertFormat, imageCodecCtx->width, imageCodecCtx->height, 1);

    pImgConvertCtx = sws_getContext(imageCodecCtx->width, imageCodecCtx->height,
        imageCodecCtx->pix_fmt, imageCodecCtx->width, imageCodecCtx->height,
        convertFormat, SWS_BICUBIC, NULL, NULL, NULL);


    int frameIndex = 0;
    while (frameIndex < filescount) {
        ui.progressLabel->setText(QString().sprintf("Encoding video: %d/%d -> ", frameIndex, filescount - 1) + outfile);
        ui.progressBar->setValue(frameIndex);

        filename = dirname + QString().sprintf("\\%06d.jpeg", frameIndex);
        //qDebug() << filename;
        avformat_free_context(ifmtCtx);
        ifmtCtx = NULL;
        ret = avformat_open_input(&ifmtCtx, filename.toStdString().c_str(), NULL, NULL);
        if (ret != 0) {
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：无法打开图像文件."));
            qDebug() << "Can't open input file.";
            goto end;
        }

        ret = av_read_frame(ifmtCtx, &pkt);

        if (ret < 0) {
            break;
        }


        ret = avcodec_send_packet(imageCodecCtx, &pkt);
        if (ret < 0) {
            emit ready_to_appendlog(QString().fromLocal8Bit("错误：图像解码出错."));
            qDebug() << "Decode error.";
            goto end;
        }

        if (avcodec_receive_frame(imageCodecCtx, imageFrame) >= 0) {
            sws_scale(pImgConvertCtx,
                (const unsigned char* const*)imageFrame->data,
                imageFrame->linesize, 0, imageCodecCtx->height, imageFrameYUV->data,
                imageFrameYUV->linesize);

            imageFrameYUV->format = AV_PIX_FMT_YUV420P;
            imageFrameYUV->width = imageCodecCtx->width;
            imageFrameYUV->height = imageCodecCtx->height;

            ret = avcodec_send_frame(videoCodecCtx, imageFrameYUV);
            if (ret < 0) {
                emit ready_to_appendlog(QString().fromLocal8Bit("错误：视频编码出错."));
                qDebug() << "failed to encode.";
                goto end;
            }

            if (avcodec_receive_packet(videoCodecCtx, &pkt) >= 0) {
                // 设置输出DTS,PTS
                pkt.pts = pkt.dts = frameIndex * (ofmtCtx->streams[0]->time_base.den) / ofmtCtx->streams[0]->time_base.num / 30;
                frameIndex++;

                ret = av_interleaved_write_frame(ofmtCtx, &pkt);
                if (ret < 0) {
                    emit ready_to_appendlog(QString().fromLocal8Bit("错误：写入文件出错."));
                    qDebug() << "send packet failed: " << ret;
                }
            }
        }
        av_packet_unref(&pkt);
        avformat_close_input(&ifmtCtx);
    }
    av_write_trailer(ofmtCtx);

end:
    avformat_close_input(&ifmtCtx);

    /* close output */
    if (ofmtCtx && !(ofmtCtx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmtCtx->pb);
    }
    avformat_free_context(ofmtCtx);
    av_free(outBuffer);
    if (ret < 0 && ret != AVERROR_EOF) {
        qDebug() << "Error occurred";
        ui.progressLabel->setText("Encode Error");
        ui.progressBar->setValue(filescount - 1);
        return -1;
    }
    qDebug() << "Encode Successfully";
    ui.progressLabel->setText("Encode Successfully");
    ui.progressBar->setValue(filescount - 1);

    return 0;
}

/*
int QtWidgetsApplication1::initImuSensor() {
    ZenSetLogLevel(ZenLogLevel_Debug);

    auto clientPair = make_client();
    auto& clientError = clientPair.first;
    auto& zenclient = clientPair.second;
    if (clientError) {
        return -1;
    }

    //zenclient = new ZenClient(std::move(clientPair.second));

    if (auto error = zenclient.listSensorsAsync()) {
        zenclient.close();
        return -1;
    }

    std::vector<ZenEventData_SensorFound> g_discoveredSensors;
    std::vector<ZenSensorComponent> imu_handles;
    std::vector<ZenSensor> sensors;
    //std::vector<std::string> sensors_name;
    std::vector<unsigned int> sensors_idx;
    std::vector<unsigned int> count;
    std::vector<ZenImuData> imudata;
    int imu_number = 0;
    int count2 = 0;
    while (!stop_camera) {
        const auto pair = zenclient.waitForNextEvent();
        const bool success = pair.first;
        auto& event = pair.second;
        if (!success) {
            break;
        }
        if (!event.component.handle) {
            if (event.eventType == ZenEventType_SensorFound) {
                g_discoveredSensors.push_back(event.data.sensorFound);
            }
            else if (event.eventType == ZenEventType_SensorListingProgress) {
                if (event.data.sensorListingProgress.progress == 1.0f) {
                    if (g_discoveredSensors.empty()) {
                        qDebug() << "-- no sensors found -- ";
                        break;
                    }
                    for (int i = 0; i < g_discoveredSensors.size(); i++) {
                        std::string sensor_name = g_discoveredSensors[i].name;
                        if (sensor_name.substr(0, 4) == "LPMS") {
                            qDebug() << i << g_discoveredSensors[i].name << " (" << g_discoveredSensors[i].ioType << ") " << g_discoveredSensors[i].identifier;
                            sensors_idx.push_back(i);
                            sensors_name.push_back(sensor_name);
                        }
                    }
                    emit ready_to_appendlog(QString().fromLocal8Bit("提示：正在尝试初始化IMU传感器，请耐心等待..."));
                    emit ready_to_initimu(&sensors_name);
                    QThread::msleep(100);
                    for (int i = 0; i < sensors_idx.size(); i++) {
                        QtConcurrent::run(this, &QtWidgetsApplication1::readImuSensor, std::string(g_discoveredSensors[sensors_idx[i]].ioType), std::string(g_discoveredSensors[i].name), std::string(g_discoveredSensors[i].identifier), i);
                        imu_number++;
                    }
                    break;
                }
            }
        }
    }

end:
    for (auto& sensor : sensors) {
        sensor.release();
    }
    zenclient.close();
    return 0;
}

void QtWidgetsApplication1::readImuSensor(std::string& io_type, std::string& sensor_name, std::string& identifer, int index) {
    auto clientPair = make_client();
    auto& clientError = clientPair.first;
    auto& zenclient = clientPair.second;
    if (clientError) {
        return;
    }
    int reconnect = 0;
    std::cout << io_type << " - " << sensor_name << std::endl;

repeat:
    auto sensorPair = zenclient.obtainSensorByName(io_type, identifer, 115200);
    auto obtainError = sensorPair.first;
    auto& sensor = sensorPair.second;
    if (obtainError && reconnect++ < 3) {
        goto repeat;
    }
    if (obtainError) {
        qDebug() << "Init Sensor " << index << " Failed.";
        std::string str = "错误：初始化传感器" + sensor_name + "失败.";
        ui.logBrowser->append(QString().fromLocal8Bit(str.c_str()));
        zenclient.close();
        return;
    }
    auto imuPair = sensor.getAnyComponentOfType(g_zenSensorType_Imu);
    auto& hasImu = imuPair.first;
    auto imu = imuPair.second;
    if (!hasImu) {
        zenclient.close();
        return;
    }
    if (auto error = imu.setBoolProperty(ZenImuProperty_StreamData, true)) {
        zenclient.close();
        return;
    }
    if (auto error = imu.setInt32Property(ZenImuProperty_SamplingRate, 50)) {
        zenclient.close();
        return ;
    }
    qDebug() << "Init Sensor " << index << " Successfully.";
    std::string str = "提示：初始化传感器" + sensor_name + "成功.";
    ui.logBrowser->append(QString().fromLocal8Bit(str.c_str()));

    int last_record = -1;
    int count = 0;
    ZenEventData_SensorFound sensorfound;
    while (!stop_camera) {
        const auto pair = zenclient.waitForNextEvent();
        const bool success = pair.first;
        auto& event = pair.second;
        if (!success) {
            break;
        }
        if (event.eventType == ZenEventType_ImuData) {
            if (!stop_record) {
                writeCsvData(*imu_outstreams[index], event.data.imuData, record2_count);
            }
            emit ready_to_updateimu(event.data.imuData, index);
        }
    }
}

void QtWidgetsApplication1::initSensorTable(std::vector<std::string> *sensors_name) {
    int num = (*sensors_name).size();
    qDebug() << num;
    ui.sensortable->setColumnCount(7);
    ui.sensortable->setRowCount(2 + num);
    ui.sensortable->verticalHeader()->setVisible(false);
    ui.sensortable->horizontalHeader()->setVisible(false);
    ui.sensortable->setShowGrid(false);
    ui.sensortable->setFocusPolicy(Qt::NoFocus);
    ui.sensortable->setFrameShape(QFrame::NoFrame);
    ui.sensortable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.sensortable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.sensortable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.sensortable->setStyleSheet("background-color:rgba(0,0,0,0)");
    ui.sensortable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui.sensortable->setColumnWidth(0, 100);
    ui.sensortable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui.sensortable->setVisible(true);

    ui.sensortable->setSpan(0, 0, 2, 1);
    ui.sensortable->setSpan(0, 1, 1, 3);
    ui.sensortable->setSpan(0, 4, 1, 3);

    ui.sensortable->setItem(0, 0, new QTableWidgetItem("Sensor"));
    ui.sensortable->item(0, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(0, 1, new QTableWidgetItem("Accleration"));
    ui.sensortable->item(0, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(0, 4, new QTableWidgetItem("Gyroscope"));
    ui.sensortable->item(0, 4)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(1, 1, new QTableWidgetItem("X"));
    ui.sensortable->item(1, 1)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(1, 2, new QTableWidgetItem("Y"));
    ui.sensortable->item(1, 2)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(1, 3, new QTableWidgetItem("Z"));
    ui.sensortable->item(1, 3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(1, 4, new QTableWidgetItem("X"));
    ui.sensortable->item(1, 4)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(1, 5, new QTableWidgetItem("Y"));
    ui.sensortable->item(1, 5)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui.sensortable->setItem(1, 6, new QTableWidgetItem("Z"));
    ui.sensortable->item(1, 6)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    for (int i = 2; i < ui.sensortable->rowCount(); i++) {
        ui.sensortable->setItem(i, 0, new QTableWidgetItem(QString().fromStdString((*sensors_name)[i - 2])));
        ui.sensortable->item(i, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        for (int j = 1; j < ui.sensortable->columnCount(); j++) {
            ui.sensortable->setItem(i, j, new QTableWidgetItem(" - "));
            ui.sensortable->item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
    //ui.logBrowser->append(QString().fromLocal8Bit("提示：初始化IMU传感器成功."));
}

void QtWidgetsApplication1::updateSensorTable(ZenImuData imudata, int col) {
    if (stop_camera) return;
    for (int j = 0; j < 3; j++) {
        ui.sensortable->item(col + 2, j + 1)->setText(QString().sprintf("%.3f", imudata.a[j]));
        ui.sensortable->item(col + 2, j + 4)->setText(QString().sprintf("%.3f", imudata.g[j]));
    }
}
*/

void QtWidgetsApplication1::appendLog(QString text) {
    ui.logBrowser->append(text);
}