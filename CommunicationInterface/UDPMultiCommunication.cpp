#include "UDPMultiCommunication.h"
#include <QAbstractSocket>
#include <algorithm>
#include<QDateTime>
#include<QCoreApplication>
#include<QDir>
#include <QSet>
#include <QMutex>
#include <QMutexLocker>
#include"CurlUploader.h"
#include<QDateTime>
UDPMultiCommunication* g_CommunicationLink = NULL;
void CALLBACK MultiTimerProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (g_CommunicationLink != NULL)
	{
		g_CommunicationLink->sl_send450Control();
	}
}
QString extractSortieFromFilename(const QString& filename)
{
	// 文件名格式：TD550_TD550A2025P1_1234567890123_STATION_CONTROL_1766126334834_1.dat
	//                 设备型号_设备序列号_sortie_数据类型_时间戳_序列号.dat

	QStringList parts = filename.split('_');

	// 检查是否有足够的部分
	if (parts.size() >= 4) {
		// m_sortie 应该是第三个部分（索引2，从0开始）
		QString sortieStr = parts[2];

		// 验证是否为数字
		bool ok;
		qint64 sortie = sortieStr.toLongLong(&ok);
		if (ok) {
			return sortieStr;
		}
	}

	return "";
}

UDPMultiCommunication::UDPMultiCommunication(QObject* parent)
	: CommunicationInterface(parent), m_bSendControl(false), m_pRxFile(nullptr)
	, m_pTxFile(nullptr)
	, m_maxFileSize(1048576) // 默认1MB
	, m_enableFileSplit(true)
	, m_filePrefix("fileData")
	, m_currentRxFileSize(0)
	, m_currentTxFileSize(0)
	, m_uploadCheckTimer(nullptr)
	, m_isUploading(false)
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
	//日志上传相关
	m_strExePath = QCoreApplication::applicationDirPath();
	initConfig();
	initialize();
}
UDPMultiCommunication::~UDPMultiCommunication()
{
	if (m_uploadCheckTimer) {
		m_uploadCheckTimer->stop();
		delete m_uploadCheckTimer;
	}
	// 上传所有剩余文件
	uploadAllRemainingFiles();

	// 关闭文件
	closeFiles();
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
		writeTxData(aryData);
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
	m_fccVsersion = version;
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
		writeRxData(aryData);
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

QSettings* UDPMultiCommunication::getConfigInfo()
{

	QString	m_configPath = m_strExePath + "/init/Config.ini";
	QSettings* m_configSettings = new QSettings(m_configPath, QSettings::IniFormat);
	// 设置编码
	m_configSettings->setIniCodec("UTF-8");
	return m_configSettings;
}

void UDPMultiCommunication::initConfig()
{
	QString configPath = m_strExePath + "/init/Config.ini";
	QSettings settings(configPath, QSettings::IniFormat);

	// 文件上传配置
	m_uploadUrl = settings.value("FileUpload/UploadUrl", "https://cloud-test.uatair.com/lfy-api").toString().toStdString();
	m_apiPath = settings.value("FileUpload/ApiPath", "/manage/logFile/getUploadUrl").toString().toStdString();
	m_apiSucessPath = settings.value("FileUpload/SucessApiPath", "/manage/logFile/record").toString().toStdString();
	m_successReportUrl = m_uploadUrl + m_apiSucessPath;
	m_logFileConfigUrl = settings.value("FileUpload/logFileConfig", "/manage/logFileConfig/list").toString().toStdString();
	m_logFileConfigUrl = m_uploadUrl + m_logFileConfigUrl;
	// 数据存储配置
	m_dataFolder = settings.value("DataStorage/DataFolder", "data").toString();
	m_maxFileSize = settings.value("DataStorage/MaxFileSize", 5120000).toLongLong();
	m_enableFileSplit = settings.value("DataStorage/EnableFileSplit", true).toBool();
	m_filePrefix = settings.value("DataStorage/FilePrefix", "fileData").toString();
	m_rxFileFolder = m_strExePath + "/" + settings.value("DataStorage/RxFileFolder", "data/RxFile").toString();
	m_txFileFolder = m_strExePath + "/" + settings.value("DataStorage/TxFileFolder", "data/TxFile").toString();
	// 设备配置
	m_deviceModel = settings.value("Device/Model", "TD550").toString().toStdString();
	m_deviceSn = settings.value("Device/sn", "6722390TD550").toString().toStdString();
	m_appID = settings.value("Device/AppId", "f0fo5to874q6ciwanzfbsrk05v9p1vcm").toString().toStdString();
	m_appSecret = settings.value("Device/AppKey", "u63kg92fsoyo273zlv39gbnoqk7usa2n").toString().toStdString();
	// 日志设置
	m_defaultLogType = settings.value("LogSettings/DefaultLogType", "STATION_PARMA").toString();
	m_appVersion = settings.value("Device/AppVersion", "V1.00.00").toString();
	m_sortiePath = settings.value("FileUpload/SortiePath", "/log/logFileConfig/getSortie").toString().toStdString();
	m_sortieUrl = m_uploadUrl + m_sortiePath;
	m_sortie = getSortieWithRetry(10, 1000); // 最多重试10次，每次间隔500ms
	//m_sortie = QDateTime::currentDateTime().toMSecsSinceEpoch();
	qDebug() << "m_uploadUrl" << m_uploadUrl.c_str();
	m_td550FileSizeMB = 1;      // 默认1MB
	m_t1400FileSizeMB = 1;      // 默认1MBko
	m_R6000FileSizeMB = 1;
}
qint64 UDPMultiCommunication::getSortieWithRetry(int maxRetries, int retryDelayMs)
{
	CurlUploader curlUploader;
	curlUploader.setLogRequestUrl(m_sortieUrl);
	curlUploader.setAppIdSecret(m_appID, m_appSecret);

	std::string sortieStr;
	int retryCount = 0;

	while (retryCount < maxRetries) {
		if (retryCount > 0) {
			qDebug() << "Retry attempt" << retryCount + 1 << "of" << maxRetries << "to get sortie";
			QThread::msleep(retryDelayMs);
		}

		sortieStr = curlUploader.getSortie(m_uploadUrl + m_sortiePath, m_deviceSn);

		if ((!sortieStr.empty())&& (sortieStr=="0")) {
			qDebug() << "Successfully obtained sortie on attempt" << retryCount + 1;
			return QString::fromStdString(sortieStr).toLongLong();
		}

		retryCount++;
	}

	// 所有重试都失败，返回当前时间戳
	qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
	qDebug() << "Failed to get sortie after" << maxRetries << "attempts, using timestamp:" << timestamp;
	return timestamp;
}

bool UDPMultiCommunication::createDataDirectories()
{
	QDir dir;

	// 创建主数据目录
	QString dataDir = m_strExePath + "/" + m_dataFolder;
	if (!dir.exists(dataDir)) {
		if (!dir.mkpath(dataDir)) {
			qWarning() << "Failed to create data directory:" << dataDir;
			return false;
		}
	}

	// 创建接收文件目录
	if (!dir.exists(m_rxFileFolder)) {
		if (!dir.mkpath(m_rxFileFolder)) {
			qWarning() << "Failed to create RxFile directory:" << m_rxFileFolder;
			return false;
		}
	}

	// 创建发送文件目录
	if (!dir.exists(m_txFileFolder)) {
		if (!dir.mkpath(m_txFileFolder)) {
			qWarning() << "Failed to create TxFile directory:" << m_txFileFolder;
			return false;
		}
	}

	qDebug() << "Data directories created successfully";
	return true;
}
bool UDPMultiCommunication::getLogFileConfig()
{
	// 使用CurlUploader获取配置
	CurlUploader curlUploader;
	curlUploader.setLogConfigUrl(m_logFileConfigUrl);
	curlUploader.setAppIdSecret(m_appID, m_appSecret); // 从配置中获取
	// 获取配置
	if (!curlUploader.getLogFileConfig(1, 1)) {
		qWarning() << "Failed to get log file config";
		return false;
	}

	// 更新本地配置
	m_td550FileSizeMB = curlUploader.getTd550FileSize();
	m_t1400FileSizeMB = curlUploader.getT1400FileSize();

	// 打印配置信息
	qInfo() << "=== Log File Configuration ===";
	qInfo() << "TD550 File Size:" << m_td550FileSizeMB << "MB";
	qInfo() << "T1400 File Size:" << m_t1400FileSizeMB << "MB";
	qInfo() << "R6000 File Size:" << m_R6000FileSizeMB << "MB";
	qInfo() << "File Merge Time:" << m_fileMergeTimeMinutes << "minutes";
	qInfo() << "===============================";

	// 根据设备型号更新最大文件大小
	if (m_deviceModel == "TD550") {
		m_maxFileSize = m_td550FileSizeMB * 1024 * 1024; // 转换为字节
		qInfo() << "Updated max file size for TD550:" << m_maxFileSize / 1024 / 1024 << "MB";
	}
	else if (m_deviceModel == "T1400") {
		m_maxFileSize = m_t1400FileSizeMB * 1024 * 1024; // 转换为字节
		qInfo() << "Updated max file size for T1400:" << m_maxFileSize / 1024 / 1024 << "MB";
	}
	else if (m_deviceModel == "R6000") {
		m_maxFileSize = m_R6000FileSizeMB * 1024 * 1024; // 转换为字节
		qInfo() << "Updated max file size for T1400:" << m_maxFileSize / 1024 / 1024 << "MB";
	}

	return true;
}
void UDPMultiCommunication::loadLogFileConfig()
{
	qDebug() << "Loading log file configuration...";

	// 在后台线程中加载配置
	QtConcurrent::run([this]() {
		bool success = this->getLogFileConfig();

		if (success) {
			qInfo() << "Log file configuration loaded successfully";
			m_maxFileSize = m_t1400FileSizeMB * 1024 * 1024;
		}
		else {
			qWarning() << "Failed to load log file configuration, using defaults";
		}
		});
}
void UDPMultiCommunication::getLogConfigFromCurl()
{
}
// 修改初始化方法，添加上传服务
bool UDPMultiCommunication::initialize()
{
	// 创建数据目录
	if (!createDataDirectories()) {
		qWarning() << "Failed to create data directories";
		return false;
	}

	// 初始化文件
	initFiles();
	loadLogFileConfig();
	// 启动上传检查定时器
	startUploadCheckTimer();
	// 启动后立即补扫历史文件，避免等待首个定时器周期
	uploadAllEligibleFiles();

	qDebug() << "UDPMultiCommunication initialized successfully";
	return true;
}
void UDPMultiCommunication::writeRxData(const QByteArray& data)
{
	if (!m_pRxFile || !m_pRxFile->isOpen()) {
		qWarning() << "Rx file is not available for writing";
		return;
	}
	// 记录旧文件名，用于文件切换时上传
	QString oldFileName = m_currentRxFileName;
	bool fileSwitched = false;
	// 写入数据
	qint64 bytesWritten = m_pRxFile->write(data);
	if (bytesWritten != data.size()) {
		qWarning() << "Failed to write all Rx data, written:" << bytesWritten << "of" << data.size();
	}
	// 更新文件大小
	m_currentRxFileSize += bytesWritten;
	// 检查文件大小并切换
	if (m_enableFileSplit && m_currentRxFileSize >= m_maxFileSize) {
		qDebug() << "Rx file reached size limit (" << m_currentRxFileSize << "bytes), creating new file";
		fileSwitched = true;

		// 文件切换时，上传旧文件
		if (!oldFileName.isEmpty() && QFile::exists(oldFileName)) {
			uploadFileWhenFull(oldFileName);
		}
		m_currentRxFileName = generateFileName("rx");
		if (openNewFile(m_pRxFile, m_currentRxFileName)) {
			// 写入数据到新文件
			m_pRxFile->write(data);
			m_currentRxFileSize = data.size();
			qDebug() << "New Rx file created:" << m_currentRxFileName;
		}
	}
	// 确保数据写入磁盘
	m_pRxFile->flush();

	// 如果文件接近限制大小，记录日志
	if (!fileSwitched && m_currentRxFileSize >= m_maxFileSize * 0.9) {
		qDebug() << "Rx file approaching size limit:" << m_currentRxFileSize << "/" << m_maxFileSize << "bytes";
	}
}

void UDPMultiCommunication::writeTxData(const QByteArray& data)
{
	if (!m_pTxFile || !m_pTxFile->isOpen()) {
		qWarning() << "Tx file is not available for writing";
		return;
	}
	// 记录旧文件名，用于文件切换时上传
	QString oldFileName = m_currentTxFileName;
	bool fileSwitched = false;

	// 写入数据
	qint64 bytesWritten = m_pTxFile->write(data);
	if (bytesWritten != data.size()) {
		qWarning() << "Failed to write all Tx data, written:" << bytesWritten << "of" << data.size();
	}

	// 更新文件大小
	m_currentTxFileSize += bytesWritten;

	// 检查文件大小并切换
	if (m_enableFileSplit && m_currentTxFileSize >= m_maxFileSize) {
		qDebug() << "Tx file reached size limit (" << m_currentTxFileSize << "bytes), creating new file";
		fileSwitched = true;

		// 文件切换时，上传旧文件
		if (!oldFileName.isEmpty() && QFile::exists(oldFileName)) {
			uploadFileWhenFull(oldFileName);
		}

		m_currentTxFileName = generateFileName("tx");
		if (openNewFile(m_pTxFile, m_currentTxFileName)) {
			// 写入数据到新文件
			m_pTxFile->write(data);
			m_currentTxFileSize = data.size();
			qDebug() << "New Tx file created:" << m_currentTxFileName;
		}
	}

	// 确保数据写入磁盘
	m_pTxFile->flush();

	// 如果文件接近限制大小，记录日志
	if (!fileSwitched && m_currentTxFileSize >= m_maxFileSize * 0.9) {
		qDebug() << "Tx file approaching size limit:" << m_currentTxFileSize << "/" << m_maxFileSize << "bytes";
	}
}

void UDPMultiCommunication::closeFiles()
{
	if (m_pRxFile) {
		m_pRxFile->close();
		delete m_pRxFile;
		m_pRxFile = nullptr;
	}

	if (m_pTxFile) {
		m_pTxFile->close();
		delete m_pTxFile;
		m_pTxFile = nullptr;
	}

	qDebug() << "All data files closed";
}

QString UDPMultiCommunication::getCurrentRxFileName() const
{
	return m_currentRxFileName;
}

QString UDPMultiCommunication::getCurrentTxFileName() const
{
	return m_currentTxFileName;
}


void UDPMultiCommunication::initFiles()
{
	// 生成初始文件名
	m_currentRxFileName = generateFileName("rx");
	m_currentTxFileName = generateFileName("tx");

	// 打开接收文件
	if (!openNewFile(m_pRxFile, m_currentRxFileName)) {
		qWarning() << "Failed to open Rx file:" << m_currentRxFileName;
		return;
	}

	// 打开发送文件
	if (!openNewFile(m_pTxFile, m_currentTxFileName)) {
		qWarning() << "Failed to open Tx file:" << m_currentTxFileName;
		return;
	}

	qDebug() << "Data files initialized:";
	qDebug() << "  Rx file:" << m_currentRxFileName;
	qDebug() << "  Tx file:" << m_currentTxFileName;
}



QString UDPMultiCommunication::generateFileName(const QString& type)
{
	QDateTime dateTime = QDateTime::currentDateTime();
	quint64 timestamp = dateTime.currentMSecsSinceEpoch();

	// 为不同类型生成不同的序列号
	static QMap<QString, int> sequenceMap;
	int sequence = sequenceMap[type] + 1;
	sequenceMap[type] = sequence;

	// 生成文件名：fileData_时间戳_类型_序号.dat
	QString filename;
	if (type == "rx")
	{
		filename = m_rxFileFolder + "/" + QString::fromStdString(m_deviceModel) + "_" + QString::fromStdString(m_deviceSn) + "_" + QString::number(m_sortie) +
			"_" + "STATION_PARMA" + "_" + QString::number(timestamp) + "_" + QString::number(sequence) + ".dat";
	}
	else {
		/*filename = m_txFileFolder + "/" + m_filePrefix + "_" +
			QString::number(timestamp) + "_" +
			type + "_" +
			QString::number(sequence) + ".dat";
	}*/
		filename = m_txFileFolder + "/" + QString::fromStdString(m_deviceModel) + "_" + QString::fromStdString(m_deviceSn) + "_" + QString::number(m_sortie) + +"_" + "STATION_CONTROL"
			+ "_" + QString::number(timestamp) + "_" + QString::number(sequence) + ".dat";
	}

	qDebug() << "Generated filename:" << filename;
	return filename;
}

bool UDPMultiCommunication::openNewFile(QFile*& file, const QString& fileName)
{
	// 关闭旧文件
	if (file) {
		file->close();
		delete file;
		file = nullptr;
	}

	// 创建新文件
	file = new QFile(fileName);
	if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
		qWarning() << "Failed to open file:" << fileName;
		delete file;
		file = nullptr;
		return false;
	}

	// 重置文件大小计数器
// 重置文件大小计数器
	if (fileName.contains("STATION_PARMA")) {
		m_currentRxFileSize = 0;
	}
	else if (fileName.contains("STATION_CONTROL")) {
		m_currentTxFileSize = 0;
	}
	else
	{
		qDebug() << "No file :" << fileName;
	}

	qDebug() << "New file opened:" << fileName;
	return true;
}
bool UDPMultiCommunication::uploadFile(const std::string& filepath, const std::string& data_type)
{
	return uploadFileImpl(QString::fromStdString(filepath), QString::fromStdString(data_type));
}

bool UDPMultiCommunication::uploadFile(const QString& filepath, const QString& data_type)
{
	return uploadFileImpl(filepath, data_type);
}

void UDPMultiCommunication::startUploadCheckTimer()
{
	//m_uploadCheckTimer = new QTimer(this);
	//m_uploadCheckTimer->setInterval(3000); // 30秒检查一次
	//m_uploadCheckTimer->setSingleShot(false);

	//connect(m_uploadCheckTimer, &QTimer::timeout,
	//	this, &UDPMultiCommunication::checkAndUploadCompletedFiles);
	//m_uploadCheckTimer->start();
	//m_isUploading = false;

	//qDebug() << "Upload check timer started, interval: 3 seconds";
	m_uploadCheckTimer = new QTimer(this);
	m_uploadCheckTimer->setInterval(3000); // 3秒检查一次
	m_uploadCheckTimer->setSingleShot(false);

	connect(m_uploadCheckTimer, &QTimer::timeout,
		this, &UDPMultiCommunication::checkAndUploadCompletedFiles);
	m_uploadCheckTimer->start();

	// 新增：状态检查定时器，防止状态卡死
	QTimer* statusCheckTimer = new QTimer(this);
	statusCheckTimer->setInterval(60000); // 60秒检查一次状态
	statusCheckTimer->setSingleShot(false);
	connect(statusCheckTimer, &QTimer::timeout, [this]() {
		QMutexLocker locker(&m_uploadMutex);
		if (m_isUploading) {
			qWarning() << "Upload status has been 'uploading' for more than 60 seconds, resetting...";
			m_isUploading = false;
			// 重新触发上传处理
			QTimer::singleShot(0, this, &UDPMultiCommunication::processUploadQueue);
		}
		});
	statusCheckTimer->start();

	qDebug() << "Upload check timer started, interval: 3 seconds";
}
// 新增：检查上传状态的方法
bool UDPMultiCommunication::isUploading()
{
	QMutexLocker locker(&m_uploadMutex);
	return m_isUploading;
}

// 新增：获取上传队列大小的方法
int UDPMultiCommunication::getUploadQueueSize()
{
	QMutexLocker locker(&m_uploadMutex);
	return m_pendingUploadFiles.size();
}
void UDPMultiCommunication::uploadAllRemainingFiles()
{
	qInfo() << "Uploading all remaining files before shutdown...";

	// 停止写入当前文件
	if (m_pRxFile && m_pRxFile->isOpen()) {
		m_pRxFile->flush();
		m_pRxFile->close();

		// 上传当前Rx文件
		if (!m_currentRxFileName.isEmpty() && QFile::exists(m_currentRxFileName)) {
			QFileInfo info(m_currentRxFileName);
			if (info.size() >= 1024) { // 至少1KB
				qDebug() << "Uploading final Rx file:" << m_currentRxFileName;
				scheduleFileUpload(m_currentRxFileName);
			}
		}
	}

	if (m_pTxFile && m_pTxFile->isOpen()) {
		m_pTxFile->flush();
		m_pTxFile->close();

		// 上传当前Tx文件
		if (!m_currentTxFileName.isEmpty() && QFile::exists(m_currentTxFileName)) {
			QFileInfo info(m_currentTxFileName);
			if (info.size() >= 1024) { // 至少1KB
				qDebug() << "Uploading final Tx file:" << m_currentTxFileName;
				scheduleFileUpload(m_currentTxFileName);
			}
		}
	}

	// 上传目录中的所有文件
	uploadAllEligibleFiles();

	// 等待上传完成（最大等待10秒）
	QElapsedTimer timer;
	timer.start();

	while (!m_pendingUploadFiles.isEmpty() && timer.elapsed() < 10000) {
		QCoreApplication::processEvents();
		QThread::msleep(100);
	}

	qInfo() << "Remaining files upload completed, pending:" << m_pendingUploadFiles.size();
}
void UDPMultiCommunication::uploadAllEligibleFiles()
{
	scheduleEligibleFilesFromDirectory(m_rxFileFolder, m_currentRxFileName);
	scheduleEligibleFilesFromDirectory(m_txFileFolder, m_currentTxFileName);
}
QJsonObject UDPMultiCommunication::MetadataToJsonObject()
{
	QJsonObject metadataObj;
	metadataObj["appVersion"] = m_appVersion;
	metadataObj["fcVersion"] = getFCCVersion();
	return metadataObj;
}

QString UDPMultiCommunication::getFCCVersion()
{
	int nVersion1 = m_fccVsersion % 100;
	QString strVersion1 = "";
	if (nVersion1 >= 10)
	{
		strVersion1 = QString::number(nVersion1);
	}
	else
	{
		strVersion1 = QString("0%1").arg(nVersion1);
	}
	int nVersion3 = m_fccVsersion / 10000;
	int nVersion2 = (m_fccVsersion - (nVersion3 * 10000)) / 100;
	QString strVersion2 = "";
	if (nVersion2 >= 10)
	{
		strVersion2 = QString::number(nVersion2);
	}
	else
	{
		strVersion2 = QString("0%1").arg(nVersion2);
	}
	return QString("%1.%2.%3").arg(nVersion3).arg(strVersion2).arg(strVersion1);
}

void UDPMultiCommunication::uploadFileWhenFull(const QString& fileName)
{
	if (fileName.isEmpty()) {
		qWarning() << "Empty file name provided for upload";
		return;
	}

	QFileInfo fileInfo(fileName);
	if (!fileInfo.exists()) {
		qWarning() << "File does not exist for upload:" << fileName;
		return;
	}

	// 将文件添加到待上传队列
	scheduleFileUpload(fileName);
	qDebug() << "File scheduled for upload due to size limit:" << fileName;
}

void UDPMultiCommunication::checkAndUploadCompletedFiles()
{
	qDebug() << "Checking for completed files to upload...";

	uploadAllEligibleFiles();

	qDebug() << "Completed file check, pending uploads:" << m_pendingUploadFiles.size();
}

bool UDPMultiCommunication::isFileReadyForUpload(const QString& filePath)
{
	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists()) {
		return false;
	}

	// 检查文件是否正在被写入（通过检查最近修改时间）
	QDateTime lastModified = fileInfo.lastModified();
	QDateTime now = QDateTime::currentDateTime();

	// 如果文件在最近2秒内被修改过，认为还在写入中
	if (lastModified.secsTo(now) < 2) {
		return false;
	}

	// 检查文件大小是否合理（大于1KB）
	if (fileInfo.size() < 1024) {
		return false;
	}

	return true;
}

bool UDPMultiCommunication::shouldScheduleFileForUpload(const QString& filePath, const QString& currentFilePath)
{
	if (filePath.isEmpty()) {
		return false;
	}

	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists()) {
		return false;
	}

	if (!currentFilePath.isEmpty() &&
		fileInfo.absoluteFilePath() == QFileInfo(currentFilePath).absoluteFilePath()) {
		return false;
	}

	return isFileReadyForUpload(fileInfo.absoluteFilePath());
}

void UDPMultiCommunication::scheduleEligibleFilesFromDirectory(const QString& directoryPath, const QString& currentFilePath)
{
	QDir dir(directoryPath);
	const QStringList files = dir.entryList(QStringList() << "*.dat", QDir::Files);

	for (const QString& fileName : files) {
		const QString filePath = dir.absoluteFilePath(fileName);
		if (shouldScheduleFileForUpload(filePath, currentFilePath)) {
			scheduleFileUpload(filePath);
		}
	}
}

void UDPMultiCommunication::scheduleFileUpload(const QString& filePath)
{
	if (filePath.isEmpty()) {
		qWarning() << "Empty file name provided for scheduling";
		return;
	}

	QMutexLocker locker(&m_uploadMutex);

	// 检查文件是否存在
	if (!QFile::exists(filePath)) {
		qWarning() << "File does not exist for scheduling:" << filePath;
		return;
	}

	// 检查是否已在队列中
	if (m_pendingUploadFiles.contains(filePath)) {
		qDebug() << "File already in upload queue:" << filePath;
		return;
	}

	// 添加到待上传队列
	m_pendingUploadFiles.append(filePath);
	// 按时间戳从大到小排序
	// 重新排序队列（按时间戳从大到小）
	sortQueueByTimestampDescending();
	qDebug() << "File scheduled for upload:" << filePath
		<< "Queue size:" << m_pendingUploadFiles.size();

	// 如果当前没有在上传，立即开始处理队列
	if (!m_isUploading) {
		QTimer::singleShot(0, this, &UDPMultiCommunication::processUploadQueue);
	}
}

void UDPMultiCommunication::processUploadQueue()
{
	const int MAX_RETRY = 3;  // 最大重试次数
	QString filePath;

	{
		QMutexLocker locker(&m_uploadMutex);
		if (m_isUploading) {
			return;
		}

		if (m_pendingUploadFiles.isEmpty()) {
			return;
		}

		m_isUploading = true;
		filePath = m_pendingUploadFiles.first();
	}

	qDebug() << "Processing upload for file:" << filePath;

	QString dataType = "STATION_PARMA";
	if (filePath.contains("tx", Qt::CaseInsensitive)) {
		dataType = "STATION_CONTROL";
	}

	bool success = false;
	int retryCount = 0;

	while (retryCount < MAX_RETRY && !success) {
		if (retryCount > 0) {
			qDebug() << "Retry" << retryCount << "for file:" << filePath;
			QThread::msleep(1000);
		}

		try {
			success = uploadFileImpl(filePath, dataType);
		}
		catch (...) {
			qWarning() << "Exception during upload";
			success = false;
		}
		retryCount++;
	}

	bool shouldScheduleNext = false;
	{
		QMutexLocker locker(&m_uploadMutex);
		m_pendingUploadFiles.removeAll(filePath);

		if (success) {
			if (QFile::exists(filePath) && !QFile::remove(filePath)) {
				qWarning() << "Failed to delete uploaded file:" << filePath;
			}
			qInfo() << "File uploaded successfully:" << filePath;
		}
		else if (QFile::exists(filePath)) {
			m_pendingUploadFiles.append(filePath);
			qWarning() << "File failed after" << MAX_RETRY << "attempts, moved to end of queue:" << filePath;
		}
		else {
			qWarning() << "File no longer exists, not re-adding to queue:" << filePath;
		}

		m_isUploading = false;
		shouldScheduleNext = !m_pendingUploadFiles.isEmpty();
	}

	if (shouldScheduleNext) {
		QTimer::singleShot(0, this, &UDPMultiCommunication::processUploadQueue);
	}
}

bool UDPMultiCommunication::uploadFileImpl(const QString& filepath, const QString& data_type)
{
	QFileInfo fileInfo(filepath);
	if (!fileInfo.exists()) {
		qWarning() << "[" << data_type << "] File does not exist:" << filepath;
		return false;
	}

	QString filename = fileInfo.fileName();
	qint64 filesize = fileInfo.size();
	QString sortie1 = extractSortieFromFilename(filename);
	// 使用 CurlUploader 获取上传URL
	CurlUploader curlUploader;
	curlUploader.setLogRequestUrl(m_uploadUrl + m_apiPath);
	curlUploader.setAppIdSecret(m_appID, m_appSecret); // 从配置中获取
	//curlUploader.setLogParam(m_deviceModel, m_deviceSn, QString::number(m_sortie).toStdString(), data_type.toStdString(), "GROUND_STATION");
	curlUploader.setLogParam(m_deviceModel, m_deviceSn, sortie1.toStdString(), data_type.toStdString(), "GROUND_STATION");

	std::string upload_url = curlUploader.requestLogUrl(filename.toStdString(), filesize);
	if (upload_url.empty()) {
		qWarning() << "[" << data_type << "] Failed to get upload URL for file:" << filename;
		return false;
	}
	//m_sortie = QString::fromStdString(curlUploader.getSortie()).toLongLong();

	//QRegExp rx("_(\\d+)_STATION_PARMA_"); // 匹配 _数字_STATION_PARMA_ 模式
	//if (rx.indexIn(filename) != -1) {
	//	filename.replace(rx.cap(1), QString::number(m_sortie)); // 将匹配到的数字替换为newSortie
	//}
	qInfo() << "[" << data_type << "] Upload URL:" << QString::fromStdString(upload_url);

	// 读取文件内容
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "[" << data_type << "] Failed to open file:" << filepath;
		return false;
	}

	QByteArray content = file.readAll();
	file.close();

	if (content.size() != filesize) {
		qWarning() << "[" << data_type << "] Failed to read complete file content:" << filepath;
		return false;
	}

	// 设置上传参数并执行上传
	curlUploader.setMethod(CurlUploader::Method::PUT);
	curlUploader.setVerifySSL(false);

	if (!curlUploader.uploadData(upload_url, content.toStdString(), filename.toStdString())) {
		qWarning() << "[" << data_type << "] Upload failed for file:" << filename;
		return false;
	}

	// 计算本地MD5
	std::string calculated_md5 = calculateFileMD5(filepath);
	std::transform(calculated_md5.begin(), calculated_md5.end(),
		calculated_md5.begin(), ::toupper);

	qInfo() << "[" << data_type << "] Local MD5:" << QString::fromStdString(calculated_md5);

	// 获取服务器返回的MD5
	std::string response_md5 = curlUploader.getResponseMD5();
	qInfo() << "[" << data_type << "] Server MD5:" << QString::fromStdString(response_md5);

	// MD5校验
	if (response_md5 != calculated_md5) {
		qWarning() << "[" << data_type << "] MD5 verification failed for file:" << filename;
		qWarning() << "[" << data_type << "] Local:" << QString::fromStdString(calculated_md5)
			<< "Server:" << QString::fromStdString(response_md5);
		return false;
	}

	// 上传成功报告 - 根据协议文档
	// 解析文件名中的时间戳和序列号
	QString timestampStr;
	int upload_seq=0;
	if (!parseFilenameComponents(filename, timestampStr, upload_seq)) {
		qWarning() << "[" << data_type << "] Failed to parse filename components:" << filename;
		// 这里不返回false，因为文件已经上传成功
	}
	else {
		// 将时间戳转换为 ISO 8601 UTC 格式
		QString logTime;
		if (!timestampStr.isEmpty()) {
			bool ok = false;
			qlonglong timestamp = timestampStr.toLongLong(&ok);
			if (ok) {
				QDateTime dateTime;
				if (timestampStr.length() == 13) { // 毫秒时间戳
					dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
				}
				else if (timestampStr.length() == 10) { // 秒时间戳
					dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
				}
				else {
					// 尝试猜测时间戳类型
					if (timestamp > 1000000000000) { // 大于 2001-09-09 的毫秒时间戳
						dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
					}
					else {
						dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
					}
				}

				if (dateTime.isValid()) {
					// 格式化为 ISO 8601 UTC: yyyyMMddTHH:mm:ssZ
					logTime = dateTime.toUTC().toString("yyyyMMddTHH:mm:ssZ");
				}
			}
		}
		QJsonObject metadataObj = MetadataToJsonObject();
		QJsonDocument doc(metadataObj);
		QString metadataString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
		// 然后传递给uploadSuccessReport
		std::string metadata = metadataString.toStdString();
		// 获取响应路径
		std::string response_path = curlUploader.getReturnPath();
		if (!curlUploader.uploadSuccessReport(m_successReportUrl,
			logTime.toStdString(),
			upload_seq, metadata)) {
			qWarning() << "[" << data_type << "] Failed to report upload success for file:" << filename;
			// 注意：上传成功报告失败不应该影响文件删除，因为文件已经成功上传
		}
		else {
			qDebug() << "[" << data_type << "] Upload success report sent successfully";
		}
	}

	// 删除本地文件 - 这是关键步骤！
	if (!QFile::remove(filepath)) {
		qWarning() << "[" << data_type << "] Failed to delete uploaded file:" << filepath;
		return false;  // 如果无法删除文件，认为上传不完整
	}

	qInfo() << "[" << data_type << "] Successfully uploaded and deleted file:" << filename;
	return true;
}






std::string UDPMultiCommunication::calculateFileMD5(const std::string& filename)
{
	return calculateFileMD5(QString::fromStdString(filename));
}
std::string UDPMultiCommunication::calculateFileMD5(const QString& filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "Cannot open file for MD5 calculation:" << filename;
		return "";
	}

	QCryptographicHash hash(QCryptographicHash::Md5);

	// 分块读取大文件，避免内存问题
	const qint64 bufferSize = 1024 * 16; // 16KB
	char buffer[bufferSize];
	qint64 bytesRead;

	while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
		hash.addData(buffer, bytesRead);
	}

	file.close();

	QByteArray result = hash.result();
	return result.toHex().toStdString();
}

// 解析文件名中的时间戳和序列号
bool UDPMultiCommunication::parseFilenameComponents(const QString& filename, QString& timestampStr, int& sequenceNum)
{
	//// 文件名格式示例：fileData_1764669485277.dat
	//// 或：TD550_TD550A2025P1_1764669485277_RX_DATA_1.dat

	//// 方法1：使用Qt的字符串分割
	//QStringList parts = filename.split('_');
	//if (parts.size() < 2) {
	//	qWarning() << "Invalid filename format:" << filename;
	//	return false;
	//}

	//// 寻找时间戳部分（通常是数字部分）
	//QString timestampCandidate;
	//for (int i = 0; i < parts.size(); ++i) {
	//	QString part = parts[i];
	//	bool isNumber = false;
	//	part.toLongLong(&isNumber);
	//	if (isNumber && part.length() >= 10) { // 时间戳至少10位（秒级时间戳）
	//		timestampCandidate = part;
	//		break;
	//	}
	//}

	//if (timestampCandidate.isEmpty()) {
	//	// 尝试从文件名中直接提取时间戳
	//	QRegularExpression timestampRegex("(\\d{10,13})");
	//	QRegularExpressionMatch match = timestampRegex.match(filename);
	//	if (match.hasMatch()) {
	//		timestampCandidate = match.captured(1);
	//	}
	//}

	//// 提取序列号（通常是最后一个数字部分，但可能包含扩展名）
	//int seqNum = 1; // 默认序列号为1
	//QString lastPart = parts.last();
	//QString baseName = lastPart.split('.').first(); // 去掉扩展名

	//bool ok = false;
	//int extractedSeq = baseName.toInt(&ok);
	//if (ok) {
	//	seqNum = extractedSeq;
	//}
	//else {
	//	// 尝试从其他部分找序列号
	//	for (int i = parts.size() - 1; i >= 0; --i) {
	//		QString part = parts[i];
	//		bool isNumber = false;
	//		int num = part.toInt(&isNumber);
	//		if (isNumber && num > 0 && part.length() <= 6) { // 序列号通常是较小的数字
	//			seqNum = num;
	//			break;
	//		}
	//	}
	//}

	//if (timestampCandidate.isEmpty()) {
	//	qWarning() << "Cannot extract timestamp from filename:" << filename;
	//	return false;
	//}

	//timestampStr = timestampCandidate;
	//sequenceNum = seqNum;

	////qDebug() << "Parsed filename:" << filename;
	////qDebug() << "  Timestamp:" << timestampStr;
	////qDebug() << "  Sequence:" << sequenceNum;
	// 备选方案：使用正则表达式
	const QString baseName = QFileInfo(filename).fileName();
	QRegularExpression regex(
		R"(^(.*?)_(.*?)_(\d+)_(STATION_PARMA|STATION_CONTROL)_(\d{10,13})_(\d+)\.dat$)"
	);

	QRegularExpressionMatch match = regex.match(baseName);
	if (match.hasMatch()) {
		QString deviceModel = match.captured(1);
		QString deviceSn = match.captured(2);
		QString sortie = match.captured(3);
		QString station = match.captured(4);
		QString timestamp = match.captured(5);  // 这是您要的第二个时间戳
		QString sequence = match.captured(6);

		timestampStr = timestamp;
		sequenceNum = sequence.toInt();

		qDebug() << "Parsed successfully:";
		qDebug() << "  Device:" << deviceModel << deviceSn;
		qDebug() << "  Sortie:" << sortie;
		qDebug() << "  Timestamp:" << timestampStr;
		qDebug() << "  Sequence:" << sequenceNum;

		return true;
	}
	return false;
}

// 从文件名中提取时间戳
QString UDPMultiCommunication::extractTimestampFromFilename(const QString& filename)
{
	QString timestampStr;
	int sequenceNum;

	if (parseFilenameComponents(filename, timestampStr, sequenceNum)) {
		return timestampStr;
	}

	return "";
}
void UDPMultiCommunication::sortQueueByTimestampDescending()
{
	std::sort(m_pendingUploadFiles.begin(), m_pendingUploadFiles.end(),
		[this](const QString& a, const QString& b) {
			QString timeStrA = extractTimestampFromFilename(a);
			QString timeStrB = extractTimestampFromFilename(b);
			const int seqA = extractSequenceFromFilename(a);
			const int seqB = extractSequenceFromFilename(b);

			bool okA, okB;
			qint64 timeA = timeStrA.toLongLong(&okA);
			qint64 timeB = timeStrB.toLongLong(&okB);

			if (okA && okB) {
				if (timeA != timeB) {
					return timeA > timeB;
				}
				if (seqA != seqB) {
					return seqA > seqB;
				}
				return a < b;
			}
			else if (okA) {
				return true;   // A有效，B无效，A排在前面
			}
			else if (okB) {
				return false;  // B有效，A无效，B排在前面
			}
			else {
				if (seqA != seqB) {
					return seqA > seqB;
				}
				return a < b;
			}
		});
}
// 从文件名中提取序列号
int UDPMultiCommunication::extractSequenceFromFilename(const QString& filename)
{
	QString timestampStr;
	int sequenceNum;

	if (parseFilenameComponents(filename, timestampStr, sequenceNum)) {
		return sequenceNum;
	}

	return 1; // 默认序列号为1
}
