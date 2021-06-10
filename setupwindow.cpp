#include "setupwindow.h"

#include <iostream>
#include <QDir>
#include <QFileDialog>

setupwindow::setupwindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
}

setupwindow::~setupwindow()
{

}

void setupwindow::show_current_dir() {
	ui.directoryComboBox->addItem(settings->value("save_dir").toString());
	ui.directoryComboBox->setCurrentIndex(0);
}

void setupwindow::on_toolbotton_clicked() {
	QString directory = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Save Path"), QDir::currentPath()));
	if (!directory.isEmpty()) {
		if (ui.directoryComboBox->findText(directory) == -1) {
			ui.directoryComboBox->addItem(directory);
		}
		ui.directoryComboBox->setCurrentIndex(ui.directoryComboBox->findText(directory));
	}
}


void setupwindow::set_setttings(QSettings* _set) {
	settings = _set;
}

void setupwindow::on_applybotton_clicked() {
	QString dir = ui.directoryComboBox->currentText();
	settings->setValue("save_dir", dir);
	qDebug() << dir << endl;
}

void setupwindow::on_cancelbotton_clicked() {
	this->close();
}