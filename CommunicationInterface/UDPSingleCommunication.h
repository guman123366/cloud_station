/*
	UDP单播通信
*/

#pragma once

#include <QObject>
#include "CommunicationInterface.h"
#include <QUdpSocket>
#include "communicationinterface_global.h"

class COMMUNICATIONINTERFACE_EXPORT UDPSingleCommunication : public CommunicationInterface
{
	Q_OBJECT

public:
	UDPSingleCommunication(QObject *parent=NULL);
	~UDPSingleCommunication();

	bool openPort() override;
	bool closePort() override;
	bool sendData(QByteArray ary, int nLength) override;
private slots:
	void sl_receiveData();			//接收数据槽函数
private:
	QUdpSocket* m_pSocket;			//UDP对象
};
