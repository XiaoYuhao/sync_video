#pragma once
#include "opencv2/opencv.hpp"
#include "windows.h"
#include "dshow.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")


int listDevices(std::vector<std::string>& list);
