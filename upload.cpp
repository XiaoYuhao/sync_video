#include "upload.h"

#include "quazip/JlCompress.h"
#include <QDir>

upload::upload(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

upload::~upload()
{
}
 

void upload::compressFiles() {

	JlCompress::compressDir("", "");
}

void upload::openFile() {

}

void upload::uploadFile() {

}


