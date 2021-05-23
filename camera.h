#pragma once
#include "opencv2/opencv.hpp"
#include "windows.h"
#include "dshow.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
};

#include <QImage>


int listDevices(std::vector<std::string>& list);
int camera1();
int JPEGtoRGB(AVPacket* pkt, AVCodecContext* pCodecCtx, AVFrame* pFrame, QImage *image);
int JPEGtoVideo(const QString& dirname);