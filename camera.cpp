#include "camera.h"

using namespace cv;

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

#include <QTime>
#include <QDebug>

int listDevices(std::vector<std::string>& list) {

	//COM Library Initialization
	//comInit();

	ICreateDevEnum* pDevEnum = NULL;
	IEnumMoniker* pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));


	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {

			printf("SETUP: Looking For Capture Devices\n");
			IMoniker* pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

				IPropertyBag* pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}

				// Find the description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"Description", &varName, 0);

				if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

				if (SUCCEEDED(hr))
				{

					hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					int count = 0;
					char tmp[255] = { 0 };
					//int maxLen = sizeof(deviceNames[0]) / sizeof(deviceNames[0][0]) - 2;
					while (varName.bstrVal[count] != 0x00 && count < 255)
					{
						tmp[count] = (char)varName.bstrVal[count];
						count++;
					}
					list.push_back(tmp);
					//deviceNames[deviceCounter][count] = 0;

					//if (!silent) DebugPrintOut("SETUP: %i) %s\n", deviceCounter, deviceNames[deviceCounter]);
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		//if (!silent) DebugPrintOut("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	//comUnInit();

	return deviceCounter;
}

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


int camera1()
{
	AVFormatContext* ifmtCtx = NULL;
	AVFormatContext* ofmtCtx = NULL;
	AVPacket pkt;
	AVFrame* pFrame, * pFrameYUV;
	SwsContext* pImgConvertCtx;
	AVDictionary* params = NULL;
	AVCodec* pCodec;
	AVCodecContext* pCodecCtx;
	unsigned char* outBuffer;
	AVCodecContext* pH264CodecCtx;
	AVCodec* pH264Codec;
	AVDictionary* options = NULL;

	int ret = 0;
	unsigned int i = 0;
	int videoIndex = -1;
	int frameIndex = 0;

	const char* inFilename = "video=USB CAMERA";//输入URL
	const char* outFilename = "output.mjpg"; //输出URL
	const char* ofmtName = NULL;

	avdevice_register_all();
	avformat_network_init();

	AVInputFormat* ifmt = av_find_input_format("dshow");
	if (!ifmt)
	{
		printf("can't find input device\n");
		goto end;
	}

	// 1. 打开输入
	// 1.1 打开输入文件，获取封装格式相关信息
	ifmtCtx = avformat_alloc_context();
	if (!ifmtCtx)
	{
		printf("can't alloc AVFormatContext\n");
		goto end;
	}
	av_dict_set_int(&options, "rtbufsize", 18432000, 0);
	if ((ret = avformat_open_input(&ifmtCtx, inFilename, ifmt, &options)) < 0)
	{
		printf("can't open input file: %s\n", inFilename);
		goto end;
	}

	// 1.2 解码一段数据，获取流相关信息
	if ((ret = avformat_find_stream_info(ifmtCtx, 0)) < 0)
	{
		printf("failed to retrieve input stream information\n");
		goto end;
	}

	// 1.3 获取输入ctx
	for (i = 0; i < ifmtCtx->nb_streams; ++i)
	{
		if (ifmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			break;
		}
	}

	printf("%s:%d, videoIndex = %d\n", __FUNCTION__, __LINE__, videoIndex);

	av_dump_format(ifmtCtx, 0, inFilename, 0);

	// 1.4 查找输入解码器
	pCodec = avcodec_find_decoder(ifmtCtx->streams[videoIndex]->codecpar->codec_id);
	if (!pCodec)
	{
		printf("can't find codec\n");
		goto end;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx)
	{
		printf("can't alloc codec context\n");
		goto end;
	}

	avcodec_parameters_to_context(pCodecCtx, ifmtCtx->streams[videoIndex]->codecpar);

	//  1.5 打开输入解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("can't open codec\n");
		goto end;
	}


	// 1.6 查找H264编码器
	pH264Codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	if (!pH264Codec)
	{
		printf("can't find h264 codec.\n");
		goto end;
	}

	// 1.6.1 设置参数
	pH264CodecCtx = avcodec_alloc_context3(pH264Codec);
	pH264CodecCtx->codec_id = AV_CODEC_ID_MJPEG;
	pH264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
	pH264CodecCtx->width = pCodecCtx->width;
	pH264CodecCtx->height = pCodecCtx->height;
	pH264CodecCtx->time_base.num = 1;
	pH264CodecCtx->time_base.den = 30;	//帧率（即一秒钟多少张图片）
	pH264CodecCtx->bit_rate = 400000;	//比特率（调节这个大小可以改变编码后视频的质量）
	pH264CodecCtx->gop_size = 250;
	pH264CodecCtx->qmin = 10;
	pH264CodecCtx->qmax = 51;
	//some formats want stream headers to be separate
//	if (pH264CodecCtx->flags & AVFMT_GLOBALHEADER)
	{
		pH264CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}


	// 1.7 打开H.264编码器
	av_dict_set(&params, "preset", "superfast", 0);
	av_dict_set(&params, "tune", "zerolatency", 0);	//实现实时编码
	if (avcodec_open2(pH264CodecCtx, pH264Codec, &params) < 0)
	{
		printf("can't open video encoder.\n");
		goto end;
	}

	// 2. 打开输出
	// 2.1 分配输出ctx
	if (strstr(outFilename, "rtmp://"))
	{
		ofmtName = "flv";
	}
	else if (strstr(outFilename, "udp://"))
	{
		ofmtName = "mpegts";
	}
	else
	{
		ofmtName = NULL;
	}

	avformat_alloc_output_context2(&ofmtCtx, NULL, ofmtName, outFilename);
	if (!ofmtCtx)
	{
		printf("can't create output context\n");
		goto end;
	}

	// 2.2 创建输出流
	for (i = 0; i < ifmtCtx->nb_streams; ++i)
	{
		AVStream* outStream = avformat_new_stream(ofmtCtx, NULL);
		if (!outStream)
		{
			printf("failed to allocate output stream\n");
			goto end;
		}

		avcodec_parameters_from_context(outStream->codecpar, pH264CodecCtx);
	}

	av_dump_format(ofmtCtx, 0, outFilename, 1);

	if (!(ofmtCtx->oformat->flags & AVFMT_NOFILE))
	{
		// 2.3 创建并初始化一个AVIOContext, 用以访问URL（outFilename）指定的资源
		ret = avio_open(&ofmtCtx->pb, outFilename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("can't open output URL: %s\n", outFilename);
			goto end;
		}
	}

	// 3. 数据处理
	// 3.1 写输出文件

	AVDictionary* opt = NULL;
	av_dict_set_int(&opt, "video_track_timescale", 30, 0);
	ret = avformat_write_header(ofmtCtx, NULL);
	if (ret < 0)
	{
		printf("Error accourred when opening output file\n");
		goto end;
	}


	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	outBuffer = (unsigned char*)av_malloc(
		av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
			pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBuffer,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	pImgConvertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
		AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	/*
	char filename[32];
	while (frameIndex < 1000)
	{
		// 3.2 从输入流读取一个packet
		ret = av_read_frame(ifmtCtx, &pkt);

		if (ret < 0)
		{
			break;
		}
		sprintf(filename, "out\\camera2_%05d.jpeg", frameIndex);
		FILE* file = fopen(filename, "wb");
		fwrite(pkt.data, 1, pkt.size, file);
		fflush(file);
		fclose(file);
		frameIndex++;
		continue;
		if (pkt.stream_index == videoIndex)
		{
			ret = avcodec_send_packet(pCodecCtx, &pkt);
			if (ret < 0)
			{
				printf("Decode error.\n");
				goto end;
			}

			if (avcodec_receive_frame(pCodecCtx, pFrame) >= 0)
			{
				sws_scale(pImgConvertCtx,
					(const unsigned char* const*)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
					pFrameYUV->linesize);


				pFrameYUV->format = pCodecCtx->pix_fmt;
				pFrameYUV->width = pCodecCtx->width;
				pFrameYUV->height = pCodecCtx->height;

				ret = avcodec_send_frame(pH264CodecCtx, pFrame);
				if (ret < 0)
				{
					printf("failed to encode.\n");
					goto end;
				}

				if (avcodec_receive_packet(pH264CodecCtx, &pkt) >= 0)
				{
					// 设置输出DTS,PTS
					pkt.pts = pkt.dts = frameIndex * (ofmtCtx->streams[0]->time_base.den) / ofmtCtx->streams[0]->time_base.num / 30;
					frameIndex++;

					ret = av_interleaved_write_frame(ofmtCtx, &pkt);
					if (ret < 0)
					{
						printf("send packet failed: %d\n", ret);
					}
					else
					{
						printf("send %5d packet successfully!\n", frameIndex);
					}
				}
			}
		}

		av_packet_unref(&pkt);
	}
	*/

	while (frameIndex < 1000)
	{
		// 3.2 从输入流读取一个packet
		ret = av_read_frame(ifmtCtx, &pkt);

		if (ret < 0)
		{
			break;
		}

		pkt.pts = pkt.dts = frameIndex * (ofmtCtx->streams[0]->time_base.den) / ofmtCtx->streams[0]->time_base.num / 30;
		frameIndex++;

		ret = av_interleaved_write_frame(ofmtCtx, &pkt);
		if (ret < 0){
			printf("send packet failed: %d\n", ret);
		}
		else{
			printf("send %5d packet successfully!\n", frameIndex);
		}
		av_packet_unref(&pkt);
	}

	av_write_trailer(ofmtCtx);

end:
	avformat_close_input(&ifmtCtx);

	/* close output */
	if (ofmtCtx && !(ofmtCtx->oformat->flags & AVFMT_NOFILE)) {
		avio_closep(&ofmtCtx->pb);
	}
	avformat_free_context(ofmtCtx);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred\n");
		return -1;
	}

	return 0;
}
