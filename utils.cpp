
#include "utils.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

void clearFiles(const QString& dirname) {
	QDir dir(dirname);
	dir.setFilter(QDir::Files);
	int filecount = dir.count();
	for (int i = 0; i < filecount; i++) {
		dir.remove(dir[i]);
	}
}

int subFilescount(const QString& dirname) {
	QDir dir(dirname);
	dir.setFilter(QDir::Files);
	return dir.count();
}

void writeImuData(QString &filename, std::vector<ZenImuData>& imudata, std::vector<std::string> &sensors_name) {
	QFile file(filename);
	file.open(QIODevice::WriteOnly);
	QTextStream out(&file);
	out.setRealNumberNotation(QTextStream::FixedNotation);
	out.setRealNumberPrecision(3);
	int n = imudata.size();
	for (int i = 0; i < n; i++) {
		out << "                ------------ Imu Sensor " << i + 1 << "------------" << endl;
		out << "Name:                    " << QString().fromStdString(sensors_name[i]) << endl;
		out << "FrameCount:              " << QString::number(imudata[i].frameCount).rightJustified(8, ' ') << endl;
		out << " TimeStamp:              " << QString::number(imudata[i].timestamp).rightJustified(8, ' ') << endl;
		out << "Accelerometer:           " << QString::number(imudata[i].a[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].a[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].a[2]).rightJustified(8, ' ') << endl;
		out << "Gyroscope:               " << QString::number(imudata[i].g[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].g[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].g[2]).rightJustified(8, ' ') << endl;
		out << "Magnetometer:            " << QString::number(imudata[i].b[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].b[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].b[2]).rightJustified(8, ' ') << endl;
		out << "Accelerometer(Raw):      " << QString::number(imudata[i].aRaw[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].aRaw[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].aRaw[2]).rightJustified(8, ' ') << endl;
		out << "Gyroscope(Raw):          " << QString::number(imudata[i].gRaw[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].gRaw[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].gRaw[2]).rightJustified(8, ' ') << endl;
		out << "Magnetometer(Raw):       " << QString::number(imudata[i].bRaw[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].bRaw[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].bRaw[2]).rightJustified(8, ' ') << endl;
		out << "Angular velocity:        " << QString::number(imudata[i].w[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].w[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].w[2]).rightJustified(8, ' ') << endl;
		out << "Euler angle:             " << QString::number(imudata[i].r[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].r[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].r[2]).rightJustified(8, ' ') << endl;
		out << "Quaternion orientation:  " << QString::number(imudata[i].q[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].q[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].q[2]).rightJustified(8, ' ') << endl;
		out << "Rotation Matrix:" << endl;
		out << "   " << QString::number(imudata[i].rotationM[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotationM[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotationM[2]).rightJustified(8, ' ') << endl;
		out << "   " << QString::number(imudata[i].rotationM[3]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotationM[4]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotationM[5]).rightJustified(8, ' ') << endl;
		out << "   " << QString::number(imudata[i].rotationM[6]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotationM[7]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotationM[8]).rightJustified(8, ' ') << endl;
		out << "Rotation Matrix(offset): " << endl;
		out << "   " << QString::number(imudata[i].rotOffsetM[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotOffsetM[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotOffsetM[2]).rightJustified(8, ' ') << endl;
		out << "   " << QString::number(imudata[i].rotOffsetM[3]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotOffsetM[4]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotOffsetM[5]).rightJustified(8, ' ') << endl;
		out << "   " << QString::number(imudata[i].rotOffsetM[6]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotOffsetM[7]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].rotOffsetM[8]).rightJustified(8, ' ') << endl;
		out << "Barometric pressure:     " << QString::number(imudata[i].pressure).rightJustified(8, ' ') << endl;
		out << "Linear acceleration:     " << QString::number(imudata[i].linAcc[0]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].linAcc[1]).rightJustified(8, ' ') << "   " << QString::number(imudata[i].linAcc[2]).rightJustified(8, ' ') << endl;
		out << "Gyroscope temperature:   " << QString::number(imudata[i].gTemp).rightJustified(8, ' ') << endl;
		out << "Altitude:                " << QString::number(imudata[i].altitude).rightJustified(8, ' ') << endl;
		out << "Temperature:             " << QString::number(imudata[i].temperature).rightJustified(8, ' ') << endl;
		out << "Heave Motion:            " << QString::number(imudata[i].heaveMotion).rightJustified(8, ' ') << endl;
		out << "                ---------------- End ----------------" << endl;
		out << endl;
	}
	file.close();
}

void writeImuData2(QTextStream& out, const ZenImuData& imudata, int record) {
	out << "              ---------------- Imu Data Frame "<<record<< "----------------" << endl;
	out << "FrameCount:              " << QString::number(imudata.frameCount).rightJustified(8, ' ') << endl;
	out << " TimeStamp:              " << QString::number(imudata.timestamp).rightJustified(8, ' ') << endl;
	out << "Accelerometer:           " << QString::number(imudata.a[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.a[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.a[2]).rightJustified(8, ' ') << endl;
	out << "Gyroscope:               " << QString::number(imudata.g[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.g[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.g[2]).rightJustified(8, ' ') << endl;
	out << "Magnetometer:            " << QString::number(imudata.b[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.b[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.b[2]).rightJustified(8, ' ') << endl;
	out << "Accelerometer(Raw):      " << QString::number(imudata.aRaw[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.aRaw[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.aRaw[2]).rightJustified(8, ' ') << endl;
	out << "Gyroscope(Raw):          " << QString::number(imudata.gRaw[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.gRaw[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.gRaw[2]).rightJustified(8, ' ') << endl;
	out << "Magnetometer(Raw):       " << QString::number(imudata.bRaw[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.bRaw[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.bRaw[2]).rightJustified(8, ' ') << endl;
	out << "Angular velocity:        " << QString::number(imudata.w[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.w[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.w[2]).rightJustified(8, ' ') << endl;
	out << "Euler angle:             " << QString::number(imudata.r[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.r[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.r[2]).rightJustified(8, ' ') << endl;
	out << "Quaternion orientation:  " << QString::number(imudata.q[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.q[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.q[2]).rightJustified(8, ' ') << endl;
	out << "Rotation Matrix:" << endl;
	out << "   " << QString::number(imudata.rotationM[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotationM[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotationM[2]).rightJustified(8, ' ') << endl;
	out << "   " << QString::number(imudata.rotationM[3]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotationM[4]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotationM[5]).rightJustified(8, ' ') << endl;
	out << "   " << QString::number(imudata.rotationM[6]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotationM[7]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotationM[8]).rightJustified(8, ' ') << endl;
	out << "Rotation Matrix(offset): " << endl;
	out << "   " << QString::number(imudata.rotOffsetM[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotOffsetM[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotOffsetM[2]).rightJustified(8, ' ') << endl;
	out << "   " << QString::number(imudata.rotOffsetM[3]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotOffsetM[4]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotOffsetM[5]).rightJustified(8, ' ') << endl;
	out << "   " << QString::number(imudata.rotOffsetM[6]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotOffsetM[7]).rightJustified(8, ' ') << "   " << QString::number(imudata.rotOffsetM[8]).rightJustified(8, ' ') << endl;
	out << "Barometric pressure:     " << QString::number(imudata.pressure).rightJustified(8, ' ') << endl;
	out << "Linear acceleration:     " << QString::number(imudata.linAcc[0]).rightJustified(8, ' ') << "   " << QString::number(imudata.linAcc[1]).rightJustified(8, ' ') << "   " << QString::number(imudata.linAcc[2]).rightJustified(8, ' ') << endl;
	out << "Gyroscope temperature:   " << QString::number(imudata.gTemp).rightJustified(8, ' ') << endl;
	out << "Altitude:                " << QString::number(imudata.altitude).rightJustified(8, ' ') << endl;
	out << "Temperature:             " << QString::number(imudata.temperature).rightJustified(8, ' ') << endl;
	out << "Heave Motion:            " << QString::number(imudata.heaveMotion).rightJustified(8, ' ') << endl;
	out << "                ---------------- End ----------------" << endl;
	out << endl;
}

void writeCsvHeader(QTextStream& out) {
	out << "ImageIndex, TimeStamp (s), FrameNumber, AccX (g), AccY (g), AccZ (g), GyroX (deg/s), GyroY (deg/s), GyroZ (deg/s), \
		MagX (uT), MagY (uT), MagZ (uT), EulerX (deg), EulerY (deg), EulerZ (deg), QuatW, QuatX, QuatY, QuatZ, \
		LinAccX (g), LinAccY (g), LinAccZ (g), Pressure (kPa), Altitude (m), Temperature (degC), HeaveMotion (m)" << endl;
}

void writeCsvData(QTextStream& out, const ZenImuData& imudata, int record) {
	out << record << ", " << imudata.timestamp << ", " << imudata.frameCount << ", " << imudata.a[0] << ", " << imudata.a[1] << ", " << imudata.a[2]
		<< ", " << imudata.g[0] << ", " << imudata.g[1] << ", " << imudata.g[2] << ", " << imudata.b[0] << ", " << imudata.b[1] << ", " << imudata.b[2]
		<< ", " << imudata.r[0] << ", " << imudata.r[1] << ", " << imudata.r[2] << ", " << imudata.q[0] << ", " << imudata.q[1] << ", " << imudata.q[2]
		<< ", " << imudata.q[3] << ", " << imudata.linAcc[0] << ", " << imudata.linAcc[1] << ", " << imudata.linAcc[2] << ", " << imudata.pressure
		<< ", " << imudata.altitude << ", " << imudata.gTemp << ", " << imudata.heaveMotion << endl;
}