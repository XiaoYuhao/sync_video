#pragma once

#include <QString>
#include <QTextStream>
#include "openzen/ZenTypes.h"
#include <vector>

void clearFiles(const QString& dirname);
int subFilescount(const QString& dirname);
void writeImuData(QString &filename, std::vector<ZenImuData>& imudata, std::vector<std::string>& sensors_name);
void writeImuData2(QTextStream& out, const ZenImuData& imudata, int record);
void writeCsvHeader(QTextStream& out);
void writeCsvData(QTextStream& out, const ZenImuData& imudata, int record);