#include "UDPMultiCommunication.h"
#include <QAbstractSocket>
#include <algorithm>
#include <QCoreApplication>
#include <QDateTime>
#include <QMutex>
#include "LogReportCoordinator.h"
UDPMultiCommunication* g_CommunicationLink = NULL;
void CALLBACK MultiTimerProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (g_CommunicationLink != NULL)
	{
		g_CommunicationLink->sl_send450Control();
	}
}

UDPMultiCommunication::UDPMultiCommunication(QObject* parent)
	: CommunicationInterface(parent), m_bSendControl(false), m_deviceOnline(false)
	, m_offlineCheckTimer(nullptr)
	, m_logCoordinator(nullptr)
{
	m_pSocket = new QUdpSocket(this);
	m_controlSocket = new QUdpSocket(this);
	g_CommunicationLink = this;
	m_nTimeOut = 5000;
	connect(&m_time, SIGNAL(timeout()), this, SLOT(sl_timeout()));
	// 新增：离线检测定时器
	m_offlineCheckTimer = new QTimer(this);
	m_offlineCheckTimer->setInterval(30000); // 30秒检测一次
	connect(m_offlineCheckTimer, &QTimer::timeout, this, [this]() {
		if (m_nTimeOut >= 30000) { // 30秒无数据认为离线
			if (m_deviceOnline) {
				m_deviceOnline = false;
				emit deviceOnlineStatusChanged(false);
				qWarning() << "Device offline detected by UDP timeout";
			}
		}});
	m_logCoordinator = new LogReportCoordinator(QCoreApplication::applicationDirPath(), this);
	if (!m_logCoordinator->initialize()) {
		qWarning() << "Failed to initialize log report coordinator";
	}
}
UDPMultiCommunication::~UDPMultiCommunication()
{
	if (m_logCoordinator) {
		m_logCoordinator->shutdown();
	}
}
bool UDPMultiCommunication::openPort()
{
	m_pSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
	if (m_pSocket->state() == QAbstractSocket::BoundState)
		m_pSocket->abort();		//关闭套接字，并丢弃写缓存中的所有待处理的数据
	bool bSucceed = m_pSocket->bind(QHostAddress::AnyIPv4, ListenPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	bool bMulticast = m_pSocket->joinMulticastGroup(QHostAddress(ListenAddress));
	//接收遥控指令
	m_controlSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
	if (m_controlSocket->state() == QAbstractSocket::BoundState)
		m_controlSocket->abort();		//关闭套接字，并丢弃写缓存中的所有待处理的数据
	bool bSucceed_c = m_controlSocket->bind(QHostAddress::AnyIPv4, SendPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	bool bMulticast_c = m_controlSocket->joinMulticastGroup(QHostAddress(ListenAddress));
	if (bSucceed && bMulticast && bSucceed_c && bMulticast_c)
	{
		connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(sl_receiveData()));
		setConnectSuccess(true);
		connect(m_controlSocket, SIGNAL(readyRead()), this, SLOT(sl_recevieControlData()));
		createTimer(m_timerResult, 80, g_MultiTime);
		qDebug() << bSucceed << bMulticast << bSucceed_c << bMulticast_c << u8"创建udp成功";
		m_time.start(500);
		return true;
	}
	else
	{
		qDebug() << bSucceed << bMulticast << bSucceed_c << bMulticast_c << u8"创建udp失败";

		return false;
	}
}
bool UDPMultiCommunication::closePort()
{
	bool bClose = disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(sl_receiveData()));
	m_pSocket->close();
	setConnectSuccess(false);
	return bClose;
}
bool UDPMultiCommunication::sendData(QByteArray ary, int nLength)
{
	int aa = ary.size();
	int nSendSize = m_pSocket->writeDatagram(ary.data(), ary.size(), QHostAddress(ListenAddress), SendPort);

	if (nSendSize == nLength)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool UDPMultiCommunication::sendDat(char* buff, int nLength)
{
	int nSendSize = m_pSocket->writeDatagram(buff, nLength, QHostAddress(ListenAddress), SendPort);

	if (nSendSize == nLength)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void UDPMultiCommunication::sl_recevieControlData()
{
	//qDebug() << u8"遥控指令的槽函数";
	while (m_controlSocket->hasPendingDatagrams())
	{
		//qDebug() << u8"收到遥控指令";
		//printf("---wds---UDPMultiCommunication::sl_receiveData---while--start");
		QByteArray aryData;
		aryData.resize(m_controlSocket->pendingDatagramSize());
		m_controlSocket->readDatagram(aryData.data(), aryData.size());
		if (m_logCoordinator) {
			m_logCoordinator->appendData(LogChannel::Tx, aryData);
		}
	}
}
void UDPMultiCommunication::sl_timeout()
{
	m_Mutex.lock();
	m_nTimeOut += 500;
	m_Mutex.unlock();

	if (m_nTimeOut >= 5000)
	{
		int m_linkState = 0;

		emit si_updateRadioInfo(0);
	}
	else if (m_nTimeOut >= 3000)
	{
		int m_linkState = 1;
		emit si_updateRadioInfo(1);

	}
	else
	{
		int m_linkState = 2;
		emit si_updateRadioInfo(2);
	}
	// 新增：设备离线检测
	if (m_nTimeOut >= 30000 && m_deviceOnline) { // 30秒无数据认为离线
		m_deviceOnline = false;
		emit deviceOnlineStatusChanged(false);
		qWarning() << "Device offline detected by UDP timeout";
	}
}
void UDPMultiCommunication::sl_getFCCVersion(unsigned short version)
{
	if (m_logCoordinator) {
		m_logCoordinator->updateFccVersion(version);
	}
}
int UDPMultiCommunication::getType()
{
	return m_linkState;
}
void UDPMultiCommunication::createTimer(MMRESULT& idEvent, UINT timeSec, UINT timerRec)
{
	TIMECAPS tc;
	//设置多媒体定时器
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		return;
	}
	//获取机器最小定时
	timerRec = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	//定时器开始工作
	timeBeginPeriod(timerRec);

	idEvent = timeSetEvent(timeSec, timerRec, (LPTIMECALLBACK)MultiTimerProc, NULL, TIME_PERIODIC);
}
unsigned short CRC16_CCIT_table[16] = { 0x0, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
	0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
	0xc18c, 0xd1ad, 0xe1ce, 0xf1ef };
unsigned short UDPMultiCommunication::CalCRC16_CCITT(unsigned char* chkbuf, int len)
{
	unsigned char byte_up = 0;
	unsigned char byte_temp = 0;
	unsigned short CRC_temp = 0;
	unsigned short CRC_code = 0xffff;
	unsigned short tempL = 0;

	for (int i = 0; i < len; i++)
	{
		tempL = chkbuf[i];

		byte_up = (unsigned char)(CRC_code >> 12);

		byte_temp = byte_up ^ (tempL >> 4);

		CRC_temp = CRC16_CCIT_table[byte_temp];

		CRC_code = ((CRC_code << 4) ^ CRC_temp) & 0xffff;

		byte_up = (unsigned char)(CRC_code >> 12);

		byte_temp = byte_up ^ (tempL & 0xf);

		CRC_temp = CRC16_CCIT_table[byte_temp];

		CRC_code = ((CRC_code << 4) ^ CRC_temp);
	}
	return CRC_code;
}
int UDPMultiCommunication::sendEmptyControlFrame(unsigned char* buffer)
{
	int nIndex = 0;
	memset(&buffer[nIndex], 0, 3); nIndex += 3;
	memset(&buffer[nIndex], 0x01, 3); nIndex += 3;
	memset(&buffer[nIndex], 0, 31); nIndex += 31;
	unsigned short nCheck = CalCRC16_CCITT(buffer, 37);
	memcpy(&buffer[37], &nCheck, 2);

	return 39;
}

bool UDPMultiCommunication::SendLxCommand(unsigned char* databuff, int len)
{
	m_Mutex.lock();
	memcpy(&g_SendBuff[9], databuff, len);

	unsigned short crc;
	crc = CalCRC16_CCITT(databuff, 6);
	memcpy(&g_SendBuff[15], &crc, 2);
	m_bSendControl = true;
	m_Mutex.unlock();

	return TRUE;
}
void UDPMultiCommunication::sl_receiveData()
{
	while (m_pSocket->hasPendingDatagrams())
	{
		//printf("---wds---UDPMultiCommunication::sl_receiveData---while--start");
		QByteArray aryData;
		aryData.resize(m_pSocket->pendingDatagramSize());
		m_pSocket->readDatagram(aryData.data(), aryData.size());
		if (m_logCoordinator) {
			m_logCoordinator->appendData(LogChannel::Rx, aryData);
		}
		//int aaa = aryData.size();
		//printf("-----aryData.size---->%d\n",aaa);
		emit si_ReceiveData(aryData, aryData.size());
		//qDebug() << aryData.size();
		//printf("---wds---UDPMultiCommunication::sl_receiveData---while--finish");
		m_nTimeOut = 0;
		// 触发数据接收信号
		emit dataReceived();
		// 更新设备在线状态
		if (!m_deviceOnline) {
			m_deviceOnline = true;
			emit deviceOnlineStatusChanged(true);
			qInfo() << "Device online detected by UDP data received";
		}
	}

}
void UDPMultiCommunication::sl_send450Control()
{
	int sendControlType = 0;
	unsigned char txbuf[128], tempBuff[39];
	memset(txbuf, 0, 128);
	memset(tempBuff, 0, 39);
	int i = 0;
	txbuf[0x00] = 0x6F;
	txbuf[0x01] = 0xEB;
	txbuf[0x02] = 0x90;
	if (m_bSendControl)
	{
		memcpy(tempBuff, g_SendBuff, 39);
		memset(&tempBuff[3], 0x01, 3);
		unsigned short nCRCCheck = CalCRC16_CCITT(tempBuff, 37);
		memcpy(&tempBuff[37], &nCRCCheck, 2);
		m_bSendControl = false;
		memset(g_SendBuff, 0, 39);
		sendControlType = 1;
	}
	else
	{
		int nSize = sendEmptyControlFrame(tempBuff);
		sendControlType = 0;
	}
	for (int j = 0x03; j <= 0x12; j++)
	{
		txbuf[j] = tempBuff[i++];
	}
	txbuf[0x1B] = tempBuff[i++];
	txbuf[0x1C] = tempBuff[i++];
	txbuf[0x1D] = 0xAA;
	txbuf[0x1E] = 0x55;
	txbuf[0x1F] = 0xAA;
	txbuf[0x20] = 0x6F;
	for (int k = 0x21; k <= 0x32; k++)
	{
		txbuf[k] = tempBuff[i++];
	}
	txbuf[0x3B] = tempBuff[i++];
	txbuf[0x3C] = tempBuff[i++];
	txbuf[0x3D] = tempBuff[i++];
	txbuf[0x3E] = 0xEB;
	txbuf[0x3F] = 0x90;


	//if (m_pSocket&& m_bSendAck)
	if (m_pSocket)
	{
		if (g_CommunicationLink->getConnectSuccess())
		{
			bool isTrue = g_CommunicationLink->sendDat(reinterpret_cast<char*>(txbuf), 128);
			if (sendControlType)
			{

			}
			else
			{

			}
		}
	}
}

bool UDPMultiCommunication::SendKgCommand(unsigned char num)
{
	m_Mutex.lock();

	memset(&g_SendBuff[0], num & 0xFF, 3);

	m_bSendControl = true;
	m_Mutex.unlock();
	return TRUE;
}

void UDPMultiCommunication::sl_updateKgControl(unsigned char ntype)
{
	SendKgCommand(ntype);
}

void UDPMultiCommunication::sl_sendZuHeCommand(unsigned char cType, QVector<double> vecData)
{
	unsigned char* cZHBuffer;
	m_Mutex.lock();
	ushort nData = 0;
	int iData = 0;
	double dData = 0.0;
	int nWLNum = 0, nWPNum = 0, nLon = 0, nLat = 0, nAlt = 0, nSpeed = 0, nTime = 0;
	switch (cType)
	{
	case 0x10://航点插入
		memset(&g_SendBuff[6], 0x10, 3);
		break;
	case 0x11://航点删除
		memset(&g_SendBuff[6], 0x11, 3);
		break;
	case 0x12://航点修改
		memset(&g_SendBuff[6], 0x12, 3);
		break;
	case 0x13://航点查询
		if (vecData.size() < 2)
		{
			return;
		}
		memset(&g_SendBuff[6], 0x13, 3);
		nWLNum = (int)vecData.at(0);
		nWPNum = (int)vecData.at(1);
		g_SendBuff[18] = nWLNum & 0xFF;
		g_SendBuff[19] = nWPNum & 0xFF;
		break;
	case 0x14://航点任务关闭
		memset(&g_SendBuff[6], 0x14, 3);
		break;
	case 0x15://航线查询
		memset(&g_SendBuff[6], 0x15, 3);
		break;
	case 0x21://点号遥调
		if (vecData.size() < 2)
		{
			return;
		}
		memset(&g_SendBuff[6], 0x21, 3);
		nWLNum = (int)vecData.at(0);
		nWPNum = (int)vecData.at(1);
		g_SendBuff[18] = nWLNum & 0xFF;
		g_SendBuff[19] = nWPNum & 0xFF;
		break;
	case 0x16://航线装订
		if (vecData.size() < 8)
		{
			return;
		}

		memset(&g_SendBuff[6], 0x16, 3);
		g_SendBuff[18] = (int)vecData.at(0);
		g_SendBuff[19] = (int)vecData.at(1);
		g_SendBuff[20] = (int)vecData.at(2);
		nLon = (int)(vecData.at(3) * 1.0e7);
		nLat = (int)(vecData.at(4) * 1.0e7);
		nAlt = (int)(vecData.at(5));
		nSpeed = (int)(vecData.at(6) * 2);
		nTime = (int)(vecData.at(7));
		memcpy(&g_SendBuff[21], &nLon, 4);
		memcpy(&g_SendBuff[25], &nLat, 4);
		memcpy(&g_SendBuff[29], &nAlt, 2);
		memcpy(&g_SendBuff[31], &nSpeed, 2);
		memcpy(&g_SendBuff[33], &nTime, 2);
		break;
	case 0x17://飞机起飞重量装订
		memset(&g_SendBuff[6], 0x17, 3);
		break;
	case 0x22://纵向位置遥调
		memset(&g_SendBuff[6], 0x22, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 10);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x23://横向位置遥调
		memset(&g_SendBuff[6], 0x23, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 10);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x24://高度遥调给定
		memset(&g_SendBuff[6], 0x24, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 2);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x25://航向遥调给定
		memset(&g_SendBuff[6], 0x25, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 10);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x26://纵向速度遥调给定
		memset(&g_SendBuff[6], 0x26, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 10);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x27://垂直速度遥调给定
		memset(&g_SendBuff[6], 0x27, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 10);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x28://侧向速度遥调给定
		memset(&g_SendBuff[6], 0x28, 3);
		cZHBuffer = new unsigned char[2];
		dData = vecData.at(0);
		nData = (ushort)(dData * 10);
		memcpy(cZHBuffer, &nData, 2);
		memcpy(&g_SendBuff[29], cZHBuffer, 2);
		break;
	case 0x31://位置偏差注入（预留）
		memset(&g_SendBuff[6], 0x31, 3);
		break;
	case 0x32://场高注入
		memset(&g_SendBuff[6], 0x32, 3);
		break;
	case 0x33://磁偏角注入（预留）
		memset(&g_SendBuff[6], 0x33, 3);
		break;
	case 0x34://A点坐标装订
		memset(&g_SendBuff[6], 0x34, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = (ushort)(vecData.at(2));
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	case 0x35://B点坐标装订
		memset(&g_SendBuff[6], 0x35, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = (ushort)(vecData.at(2));
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	case 0x36://C点坐标装订
		memset(&g_SendBuff[6], 0x36, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = (ushort)(vecData.at(2));
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	case 0x37://迫降点1坐标装订
		memset(&g_SendBuff[6], 0x37, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = vecData.at(2) * 2;
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	case 0x38://迫降点2坐标装订
		memset(&g_SendBuff[6], 0x38, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = vecData.at(2) * 2;
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	case 0x39://迫降点3坐标装订
		memset(&g_SendBuff[6], 0x39, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = vecData.at(2) * 2;
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	case 0x40://迫降点4坐标装订
		memset(&g_SendBuff[6], 0x40, 3);
		cZHBuffer = new unsigned char[10];
		iData = vecData.at(0) * 1.0e7;
		memcpy(cZHBuffer, &iData, 4);
		iData = vecData.at(1) * 1.0e7;
		memcpy(cZHBuffer + 4, &iData, 4);
		nData = vecData.at(2) * 2;
		memcpy(cZHBuffer + 8, &nData, 2);
		memcpy(&g_SendBuff[21], cZHBuffer, 10);
		break;
	default:
		break;
	}

	m_bSendControl = true;
	m_Mutex.unlock();
}

void UDPMultiCommunication::sl_upfateLxControl(char* cBuff, int nLength)
{
	unsigned char cTempData[6] = { 0 };
	cTempData[0] = (unsigned char)(cBuff[0] / 100.0 * 127);
	cTempData[1] = (unsigned char)(cBuff[1] / 100.0 * 127);
	cTempData[2] = (unsigned char)(cBuff[2] / 100.0 * 127);
	cTempData[3] = (unsigned char)(cBuff[3] / 100.0 * 127);
	cTempData[4] = (unsigned char)(cBuff[4] / 100.0 * 127);
	cTempData[5] = (unsigned char)(cBuff[5] / 100.0 * 127);
	SendLxCommand(cTempData, nLength);
}

bool UDPMultiCommunication::isDeviceOnline() const
{
	return m_deviceOnline;
}







