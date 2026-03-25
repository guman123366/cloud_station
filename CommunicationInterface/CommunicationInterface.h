/*
	通信接口
*/

#pragma once
#include <QObject>
#include "communicationinterface_global.h"

class COMMUNICATIONINTERFACE_EXPORT CommunicationInterface : public QObject
{
	Q_OBJECT

public:
	CommunicationInterface(QObject *parent);
	virtual ~CommunicationInterface();

	virtual bool openPort() = 0;								//打开断开
	virtual bool sendData(QByteArray ary, int nLength) = 0;		//发送数据
	virtual bool closePort() = 0;								//关闭端口
	void setConnectSuccess(bool bSuccess){ ConnectSuccess = bSuccess; }	
	bool getConnectSuccess(){ return ConnectSuccess; }
	void setCommunicationParam(QString listenAddress,int listenPort,QString sendAddress,int sendPort);
	
	QString ListenAddress;		//本地地址
	int ListenPort;				//本地端口
	QString SendAddress;		//远端地址
	int SendPort;				//远端端口
	QString	CommandTopi;
		QString	TelemetryTopic;
private:
	bool ConnectSuccess;		//是否创建成功

signals:
	void si_ReceiveData(QByteArray ary, int nLength);
};
