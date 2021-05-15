#pragma once

#include <QWidget>
#include "ui_upload.h"
#include <QAbstractSocket>
#include <QTcpSocket>

class upload : public QWidget
{
	Q_OBJECT

public:
	upload(QWidget *parent = Q_NULLPTR);
	~upload();

private:
	Ui::upload ui;



private slots:
	void compressFiles();
	void uploadFile();
	void openFile();
	//void startTransfer();
	//void updateClientProgress();

};
