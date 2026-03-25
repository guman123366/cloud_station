#include "UAVDataTransmition.h"
#include "DataCenterManager.h"


#include <QCoreApplication>
#include <QCryptographicHash>
#include <QString>
#include <QByteArray>
#include <QDebug>

UAVDataTransmition::UAVDataTransmition(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	initData();
	ui.pushButton_testUOMSend->setHidden(true);


	ui.VedioRTMP->setHidden(true);
	
}

UAVDataTransmition::~UAVDataTransmition()
{
	if (DataMgr)
	{
		delete DataMgr;

	}
}

void UAVDataTransmition::sl_StartCommLink()
{
	//创建通信连接
	DataMgr->CreateAllConnect();

	DataStateTimer.start(100);
}
void UAVDataTransmition::sl_VedioRTMP()
{
	b_start = !b_start;

	if (b_start)//执行推流;
	{
		QString appDirPath = QCoreApplication::applicationDirPath();
		appDirPath = appDirPath + "/ffmpeg-7.1.1-full_build-shared/bin/ffmpeg.exe";
		QStringList arguments;
		//arguments << "-f" << "gdigrab" << "-r" << "30" << "-s" << "1920x1080" << "-i" << "desktop" << "-vcodec" << "libx264" << "-preset" << "ultrafast" << "-crf" << "23" << "-f" << "flv" << "rtmp://112.29.133.197/uav/lianhe_TD550";
		
		//带屏幕偏移的命令：ffmpeg.exe - f gdigrab - offset_x - 1920 - offset_y 0 - video_size 1920x1080 - i desktop - vcodec libx264 - framerate 5 - pix_fmt yuvj420p - colorspace bt470bg - color_primaries bt470bg - color_trc bt709 - f rtsp rtsp ://127.0.0.1:8554/uav/lianhe_TD550
		arguments.append("-f");
		arguments.append("gdigrab");
		arguments.append("-r");
		arguments.append("30");
		arguments.append("-s");
		arguments.append("1920x1080");
		arguments.append("-i");
		if (qs_vedioHd == "desktop")
			arguments.append("desktop");
		else
			arguments.append("title="+qs_vedioHd);
		arguments.append("-vcodec");
		arguments.append("libx264");
		arguments.append("-preset");
		arguments.append("ultrafast");
		arguments.append("-crf");
		arguments.append("23");
		arguments.append("-f");
		arguments.append("flv");
		arguments.append(qs_vedioStreamIp/*"rtmp://112.29.133.197/uav/lianhe_TD550"*/);
		
		m_process = new QProcess();
		m_process->setProcessChannelMode(QProcess::MergedChannels);
		connect(m_process, &QProcess::readyReadStandardOutput, this, &UAVDataTransmition::onOut);
		m_process->start(appDirPath, arguments);

		if (!m_process->waitForStarted()) {
			qDebug() << "start failed:" << m_process->errorString();
		}
		else {
			qDebug() << "start success:";
			ui.VedioRTMP->setText(QStringLiteral("停止推流"));
		}
	}
	else//停止推流;
	{
		disconnect(m_process, &QProcess::readyReadStandardOutput, this, &UAVDataTransmition::onOut);
		m_process->close();
		delete m_process;
		m_process = NULL;
		ui.VedioRTMP->setText(QStringLiteral("开始推流"));
	}
	
}

void UAVDataTransmition::onOut()
{
	//实时回显数据
	qDebug() << m_process->readAllStandardOutput().data();
}

void UAVDataTransmition::sl_CloseCommLink()
{
	DataMgr->DisAllConnect();
}
void UAVDataTransmition::sl_testUOMSend()
{
	DataMgr->test();
}
void UAVDataTransmition::sl_updateDataState()
{
	bool UAVDataState = DataMgr->getUAVLinkState();
	if (UAVDataState)
	{
		ui.UAVLight->setPixmap(QPixmap(":/images/Resources/images/normal.png"));
		ui.CommLink->setEnabled(false);
		ui.CloseCommLink->setEnabled(true);
	}
	else
	{
		ui.UAVLight->setPixmap(QPixmap(":/images/Resources/images/noready"));
		ui.CommLink->setEnabled(true);
		ui.CloseCommLink->setEnabled(false);
	}
}

void UAVDataTransmition::initData()
{
	connect(ui.CommLink, SIGNAL(clicked()), this, SLOT(sl_StartCommLink()));
	connect(ui.CloseCommLink, SIGNAL(clicked()), this, SLOT(sl_CloseCommLink()));
	connect(ui.pushButton_testUOMSend, SIGNAL(clicked()), this, SLOT(sl_testUOMSend()));
	connect(ui.VedioRTMP, SIGNAL(clicked()), this, SLOT(sl_VedioRTMP()));
	ui.UAVLight->setPixmap(QPixmap(":/images/Resources/images/normal.png"));
	connect(&DataStateTimer, SIGNAL(timeout()), this, SLOT(sl_updateDataState()));

	ui.CommLink->setEnabled(true);
	ui.CloseCommLink->setEnabled(false);

	DataMgr = new DataCenterManager;

	qs_vedioStreamIp = DataMgr->getVideoStreamIP();
	qs_vedioHd = DataMgr->getVideoHd();
}
