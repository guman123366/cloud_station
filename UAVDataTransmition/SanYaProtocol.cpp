#include "SanYaProtocol.h"
#include "SanYaData.h"
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

SanYaProtocol::SanYaProtocol(QObject *parent /*= NULL*/)
	:ConstructionProtocolInterface(parent)
{

}

SanYaProtocol::~SanYaProtocol()
{

}

int SanYaProtocol::ConstructionData(unsigned char* sendBuf)
{
	if (!ReceiveUAVData)
		return 0;

	SanYaData* sanyaData = (SanYaData*)ReceiveUAVData;
	if (!sanyaData)
		return 0;


	QJsonObject objectData;
	QString strTemp = "";
	double dTemp = 0.0;
	objectData.insert("equiptype", "2");
	objectData.insert("id", "ZHZ770001");
	QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz");
	qint64 nCurrentTimeCount = QDateTime::currentDateTime().msecsTo(QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0))) + 8 * 60 * 60 * 1000;		//加上时区的八个小时
	strTemp = QString("%1").arg(-nCurrentTimeCount);
	objectData.insert("timestamp", strTemp);
	strTemp = QString::number(sanyaData->m_dLon, 'f', 7);
	objectData.insert("lng", strTemp);
	strTemp = QString::number(sanyaData->m_dLat, 'f', 7);
	objectData.insert("lat", strTemp);
	strTemp = QString::number(sanyaData->m_dAlt, 'f', 3);
	objectData.insert("alt", strTemp);
	strTemp = QString::number(dTemp, 'f', 2);
	objectData.insert("fuel", strTemp);
	strTemp = QString::number(dTemp, 'f', 2);
	objectData.insert("speed", strTemp);
	objectData.insert("yaw", strTemp);
	objectData.insert("pitch", strTemp);
	objectData.insert("roll", strTemp);
	QByteArray byte_array = QJsonDocument(objectData).toJson();

	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());

	return byte_array.size();

	/*QJsonObject RootJasonObject;
	RootJasonObject.insert("username", "admin");
	RootJasonObject.insert("password", "Y3l5Y29udHJvbDIy");
	RootJasonObject.insert("type", 2);
	RootJasonObject.insert("deviceHardId", "22334345");

	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());
	return byte_array.size();*/
}
