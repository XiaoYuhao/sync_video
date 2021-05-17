
#include "utils.h"
#include <QDir>
#include <QFile>

void clearFiles(const QString& dirname) {
	QDir dir(dirname);
	dir.setFilter(QDir::Files);
	int filecount = dir.count();
	for (int i = 0; i < filecount; i++) {
		dir.remove(dir[i]);
	}
}