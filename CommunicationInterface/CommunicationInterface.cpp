#include "CommunicationInterface.h"

CommunicationInterface::CommunicationInterface(QObject *parent)
	: QObject(parent), ConnectSuccess(false), ListenAddress(""), ListenPort(0)
	, SendAddress(""), SendPort(0)
{
}

CommunicationInterface::~CommunicationInterface()
{
}

void CommunicationInterface::setCommunicationParam(QString listenAddress, int listenPort, QString sendAddress, int sendPort)
{
	ListenAddress = listenAddress;
	ListenPort = listenPort;
	SendAddress = sendAddress;
	SendPort = sendPort;
}
