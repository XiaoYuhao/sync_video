#pragma once

#include <QWidget>
#include <QSettings>
#include <QDebug>
#include "ui_setupwindow.h"

class setupwindow : public QWidget
{
	Q_OBJECT

public:
	setupwindow(QWidget *parent = Q_NULLPTR);
	~setupwindow();
	void set_setttings(QSettings* _set);

private:
	Ui::setupwindow ui;

	QSettings* settings;
	

private slots:
	void on_toolbotton_clicked();
	void on_applybotton_clicked();
	void on_cancelbotton_clicked();
};
