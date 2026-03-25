#include "UDPSingleCommunication.h"

UDPSingleCommunication::UDPSingleCommunication(QObject *parent)
	: CommunicationInterface(parent)
{
	m_pSocket = new QUdpSocket(this);
}

UDPSingleCommunication::~UDPSingleCommunication()
{
}

bool UDPSingleCommunication::openPort()
{
	m_pSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
	if (m_pSocket->state() == QAbstractSocket::BoundState)
		m_pSocket->abort();		//关闭套接字，并丢弃写缓存中的所有待处理的数据

	bool bSucceed = m_pSocket->bind(QHostAddress::AnyIPv4, ListenPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	if (bSucceed)
	{
		connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(sl_receiveData()));
		setConnectSuccess(true);
		return true;
	}
	else
	{
		return false;
	}
}

bool UDPSingleCommunication::closePort()
{
	bool bClose = disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(sl_receiveData()));
	m_pSocket->close();
	setConnectSuccess(false);
	return bClose;
}

bool UDPSingleCommunication::sendData(QByteArray ary, int nLength)
{
	int nSendSize = m_pSocket->writeDatagram(ary.data(), ary.size(), QHostAddress(SendAddress), SendPort);

	if (nSendSize == nLength)
	{
		return true;
	}
	else
	{
		return false;
	}
}


void UDPSingleCommunication::sl_receiveData()
{
	while (m_pSocket->hasPendingDatagrams())
	{
		QByteArray aryData;
		aryData.resize(m_pSocket->pendingDatagramSize());
		m_pSocket->readDatagram(aryData.data(), aryData.size());

		emit si_ReceiveData(aryData, aryData.size());
		//qDebug() << aryData.size();
	}
}
