#include "DataProcessFactoryInterface.h"
#include "../CommunicationInterface/CommunicationInterface.h"
#include "../DataAnalysisInterface/DataAnalysisInterface.h"
#include"../DataAnalysisInterface/TD550DataAnalysis.h"
#include "ZHZ_Common.h"
#include "DataDefineInterface.h"
#include "ConstructionProtocolInterface.h"
#include"MQTTProtocol.h"
#include<QThread>
#include "TD550TelemetryData.h"

DataProcessFactoryInterface::DataProcessFactoryInterface(CommunicationInterface* Communication, DataAnalysisInterface* DataAnalysis, ConstructionProtocolInterface* ConstructionProtocol, QObject *parent /*= NULL*/)
	:QObject(parent), ReceiveDataCommunication(Communication), ReceiveDataAnalysis(DataAnalysis), CommunicationName(""), CommunicationListenAddress("")
	, CommunicationListenPort(0), CommunicationSendAddress(""), CommunicaitonSendPort(0), ConstructionSendData(ConstructionProtocol), ReceiveDataCounts(0)
	, AnaylsisDataDefine(NULL), ThirdDataUsed(NULL)
{
	printf("------\n");
}

DataProcessFactoryInterface::~DataProcessFactoryInterface()
{
}

bool DataProcessFactoryInterface::CreateConnect()
{
	//printf("---wds---DataProcessFactoryInterface::CreateConnect---ListenAddress--start\n");
	if (!ReceiveDataCommunication)
	{
		//printf("---wds---DataProcessFactoryInterface::CreateConnect---ListenAddress--!ReceiveDataCommunication,return false\n");
		return false;
	}
		

	ReceiveDataCommunication->setCommunicationParam(CommunicationListenAddress, CommunicationListenPort, CommunicationSendAddress, CommunicaitonSendPort);
	bool ConnectSuccess = ReceiveDataCommunication->openPort();
	//if (!ConnectSuccess)
	//	QMessageBox::warning(NULL, "警告", CommunicationName +"通信创建失败");


	connect(ReceiveDataCommunication, SIGNAL(si_ReceiveData(QByteArray, int)), this, SLOT(sl_ProcessData(QByteArray, int)));
	
   connect(ReceiveDataCommunication, SIGNAL(si_updateLinkState(int)), this, SLOT(sl_gettype(int)));
   connect(this, SIGNAL(si_sendFCCVersion(unsigned short)), ReceiveDataCommunication, SLOT(sl_getFCCVersion(unsigned short)));
	ReceiveDataTimer.start(100);
	ReceiveDataCounts = 1000;
	connect(&ReceiveDataTimer, SIGNAL(timeout()), this, SLOT(sl_ReceiveDataCounts()));
	return ConnectSuccess;
}

bool DataProcessFactoryInterface::DisConnect()
{
	if (!ReceiveDataCommunication)
		return false;

	if (ReceiveDataCommunication->getConnectSuccess())
	{
		return ReceiveDataCommunication->closePort();
	}

	return false;
}

void DataProcessFactoryInterface::setReceiveCommunicationParam(QString listenAddress, int listenPort, QString sendAddress, int sendPort)
{
	CommunicationListenAddress = listenAddress;
	CommunicationListenPort = listenPort;
	CommunicationSendAddress = sendAddress;
	CommunicaitonSendPort = sendPort;
}

void DataProcessFactoryInterface::startSendTimer(int SendTimes)
{
	SendDataTimer.start(SendTimes);
	connect(&SendDataTimer, SIGNAL(timeout()), this, SLOT(sl_SendThirdData()));
}

void DataProcessFactoryInterface::stopSendTimer()
{
	SendDataTimer.stop();
	disconnect(&SendDataTimer, SIGNAL(timeout()), this, SLOT(sl_SendThirdData()));
}

bool DataProcessFactoryInterface::getLinkState()
{
	if (ReceiveDataCounts > 500)
		return false;
	else
		return true;
}

void DataProcessFactoryInterface::sl_ProcessData(QByteArray receiveBuf, int nBufSize)
{
	if (!ReceiveDataAnalysis)
	{
		printf("----->ReceiveDataAnalysis is NULL!\n");
		return;
	}
	AnaylsisDataDefine=ReceiveDataAnalysis->AnalyseData(receiveBuf, nBufSize);
	ReceiveDataCounts = 0;
	TD550DataAnalysis* pAnalysis = dynamic_cast<TD550DataAnalysis*>(ReceiveDataAnalysis);
	m_FCCVersion = pAnalysis->getSendFCCVersion();
	emit si_sendFCCVersion(m_FCCVersion);
}

void DataProcessFactoryInterface::sl_SendThirdData()
{
	if (!ConstructionSendData)
		return;
	if (!ReceiveDataCommunication)
		return;

	ConstructionSendData->setUAVData(ThirdDataUsed);
	// 如果是MQTTProtocol，获取所有发送函数
	MQTTProtocol* mqttProtocol = dynamic_cast<MQTTProtocol*>(ConstructionSendData);
	if (!mqttProtocol) {
		// 如果不是MQTTProtocol，只发送基础数据
		unsigned char temp[2048] = { 0 };
		int len = ConstructionSendData->ConstructionData(temp);
		if (len > 0) {
			ReceiveDataCommunication->sendData(QByteArray((char*)temp, len), len);
		}
		return;
	}
	mqttProtocol->setLinkState(m_linktype);
	// 定义所有发送函数的指针数组
	typedef int (MQTTProtocol::* SendFunc)(unsigned char*);
	SendFunc sendFunctions[] = {
		&MQTTProtocol::ConstructionData,
		&MQTTProtocol::constructionMemsData,
		&MQTTProtocol::constructionEngine_t1400,
		&MQTTProtocol::constructionRemotecontrol,
		&MQTTProtocol::constructionPower,
		&MQTTProtocol::constructionDiscrete,
		&MQTTProtocol::constructionDrivetrain,
		&MQTTProtocol::constructionFMS,
		&MQTTProtocol::constructionGuidanceLaw,
		&MQTTProtocol::constructionControlLaw,
		&MQTTProtocol::constructionOther_t1400
	};
	// 循环调用所有发送函数
	unsigned char temp[2048] = { 0 };
	for (auto func : sendFunctions) {
		int len = (mqttProtocol->*func)(temp);
		if (len > 0) {
			ReceiveDataCommunication->sendData(QByteArray((char*)temp, len), len);
			QThread::msleep(1); // 小延迟防止网络拥塞
		}
	}
	
}

void DataProcessFactoryInterface::sl_ReceiveDataCounts()
{
	ReceiveDataCounts += 100;
}

void DataProcessFactoryInterface::sl_gettype(int type)
{
	m_linktype = type;
	/*if (ConstructionSendData && ReceiveDataCommunication)
	{
		MQTTProtocol* mqttProtocol = dynamic_cast<MQTTProtocol*>(ConstructionSendData);
		if (mqttProtocol) {
			mqttProtocol->setLinkState(m_linktype);
		}
	}*/

}
