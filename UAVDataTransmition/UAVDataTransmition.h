#pragma once

#include <QtWidgets/QWidget>
#include "ui_UAVDataTransmition.h"
#include <QTimer>
#include <QProcess>
class DataCenterManager;

class UAVDataTransmition : public QWidget
{
	Q_OBJECT

public:
	UAVDataTransmition(QWidget *parent = Q_NULLPTR);
	~UAVDataTransmition();
private slots:
	void sl_StartCommLink();
	void sl_CloseCommLink();
	void sl_testUOMSend();
	void sl_updateDataState();
	void sl_VedioRTMP();
	void onOut();

private:
	Ui::UAVDataTransmitionClass ui;

	void initData();
	QTimer DataStateTimer;

	DataCenterManager* DataMgr;

	QProcess *m_process = nullptr;

	bool b_start = false;

	QString qs_vedioStreamIp = "";
	QString qs_vedioHd = "";
};
