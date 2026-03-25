#pragma once

#include <QObject>

#define UDPSendPod9XXX() UDPSendPodComm9XXX::getInstance()

class UDPMultiCommunication;

class UDPSendPodComm9XXX : public QObject
{
	Q_OBJECT
public:
	static UDPSendPodComm9XXX* getInstance();
	bool CreateConnect();
	bool DisConnect();
	void sendData(QByteArray ary, int nLength);
private:
	UDPSendPodComm9XXX(QObject *parent=NULL);
	~UDPSendPodComm9XXX();
	static UDPSendPodComm9XXX* SendPodComm9XXXInstance;

	UDPMultiCommunication* UDPPodComm9XXX;
};
