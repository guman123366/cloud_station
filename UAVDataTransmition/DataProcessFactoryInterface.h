#pragma once

#include <QObject>
#include <QTimer>

class CommunicationInterface;
class DataAnalysisInterface;
class DataDefineInterface;
class ConstructionProtocolInterface;
class MQTTProtocol;
class TD550TelemetryData;
#include "../CommunicationInterface/MQTTCommunication.h"
#include"../CommunicationInterface/UDPMultiCommunication.h"
class DataProcessFactoryInterface : public QObject
{
	Q_OBJECT

public:
	DataProcessFactoryInterface(CommunicationInterface* Communication, DataAnalysisInterface* DataAnalysis, ConstructionProtocolInterface* ConstructionProtocol, QObject *parent = NULL);
	~DataProcessFactoryInterface();
	
	bool CreateConnect();
	bool DisConnect();
	void setCommunicationName(QString Name){ CommunicationName = Name; }
	void setReceiveCommunicationParam(QString listenAddress, int listenPort, QString sendAddress, int sendPort);
	DataDefineInterface* getAnalysisData(){ return AnaylsisDataDefine; }
	void startSendTimer(int SendTimes);
	void stopSendTimer();
	void setThirdData(DataDefineInterface* ThirdData){ ThirdDataUsed = ThirdData; }
	bool getLinkState();
	CommunicationInterface* getCommunication(){ return ReceiveDataCommunication; }
private slots:
	void sl_ProcessData(QByteArray, int);
	void sl_SendThirdData();
	void sl_ReceiveDataCounts();			//计算未收到数据计数
	void sl_gettype(int);

signals:
	void si_sendLastTemToMqtt(TD550TelemetryData*);
	void si_sendFCCVersion(unsigned short version);
private:
	CommunicationInterface* ReceiveDataCommunication;			//接收数据通信连接
	DataAnalysisInterface* ReceiveDataAnalysis;					//接收数据解析
	ConstructionProtocolInterface* ConstructionSendData;		//按照协议打包发送数据

	QString CommunicationName;
	QString CommunicationListenAddress;
	int CommunicationListenPort;
	QString CommunicationSendAddress;
	int CommunicaitonSendPort;
	DataDefineInterface* AnaylsisDataDefine;

	QTimer SendDataTimer;				//发送定时器
	QByteArray SendData;				//发送的数据
	DataDefineInterface* ThirdDataUsed;	//打包数据所需的外部数据

	QTimer ReceiveDataTimer;			//计算接收数据通断定时器
	int ReceiveDataCounts;				//未收到数据计数
	int m_linktype;
	unsigned short m_FCCVersion;
};
