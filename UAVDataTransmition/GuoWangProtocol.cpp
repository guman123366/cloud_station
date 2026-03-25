#include "GuoWangProtocol.h"
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include "TD550TelemetryData.h"
GuoWangProtocol::GuoWangProtocol(QObject *parent)
	: ConstructionProtocolInterface(parent)
{
}

GuoWangProtocol::~GuoWangProtocol()
{
}

int GuoWangProtocol::ConstructionData(unsigned char* sendBuf)
{
	if (!ReceiveUAVData)
		return 0;
	TD550TelemetryData* ProtocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!ProtocolData)
		return 0;

	QJsonObject RootJasonObject;
	RootJasonObject.insert("uavSN", "24001001");
	RootJasonObject.insert("lng", ProtocolData->m_FirstSubTwoFrame.lon);
	RootJasonObject.insert("lat", ProtocolData->m_FirstSubTwoFrame.lat);
	RootJasonObject.insert("height", ProtocolData->m_FirstSubThreeFrame.AbsolutelyHeight - ProtocolData->m_SecondSubFourFrame.takeoffHeight);//绝对高度-场高;
	RootJasonObject.insert("alt", ProtocolData->m_FirstSubThreeFrame.AbsolutelyHeight);
	RootJasonObject.insert("status", "2");//实时状态(1：待命2：飞行中)
	RootJasonObject.insert("rollData", ProtocolData->m_FirstSubThreeFrame.RollAngVelocity);//滚转角;
	RootJasonObject.insert("pitchData", ProtocolData->m_FirstSubThreeFrame.PitchAngVelocity);//俯仰角;
	RootJasonObject.insert("headingData", ProtocolData->m_FirstSubThreeFrame.YawAngVelocity);//偏航角;
	RootJasonObject.insert("uavHSpeed", ProtocolData->m_FirstSubThreeFrame.XSpeed);//水平速度;
	RootJasonObject.insert("uavVSpeed", ProtocolData->m_FirstSubThreeFrame.ZSpeed);//垂直速度;
	RootJasonObject.insert("flyDistance"			,0);//当前飞行里程
	RootJasonObject.insert("attitude", "1"); //飞行姿态1:俯仰2 : 横
	RootJasonObject.insert("batteryPerce"	,0);//电池电量;
	RootJasonObject.insert("oilPerce", ProtocolData->m_ThirdSubTwoFrame.Oil);//剩余油量;
	RootJasonObject.insert("upQuality"			,0);//无人机与平台的通信质量(unit:%)
	RootJasonObject.insert("downQuality"		,0);//平台与无人机的通信质量(unit:%)
	RootJasonObject.insert("rainFall", "0"); //无人机所在地天气情况0：无雨1：小雨2：中雨3：大雨
	RootJasonObject.insert("windSpeed", 0); //无人机当前接触到的风速（米 / 秒）
	RootJasonObject.insert("temperature", 0); //无人机目前舱内温度
	RootJasonObject.insert("environmentTemperature"		,0);//无人机所在地的环境温度
	RootJasonObject.insert("time"		,QDateTime::currentDateTime().toMSecsSinceEpoch());
	RootJasonObject.insert("flyTime", 220);

	QJsonArray bro_arry;
	QJsonObject LoadInfoList;
	LoadInfoList.insert("loadNo", "CDHF-000001");
	LoadInfoList.insert("loadSN", "ZHD0086425");
	LoadInfoList.insert("loadName", QStringLiteral("载荷设备1"));
	LoadInfoList.insert("status", 0);
	LoadInfoList.insert("workStatus",1);
	QJsonObject loadResultList;
	LoadInfoList.insert("loadResult", loadResultList);
	QJsonArray bro_arry1;
	QJsonObject TaskInfoList;
	TaskInfoList.insert("taskId", "3122");
	TaskInfoList.insert("taskNo", "RW1831815630");
	TaskInfoList.insert("taskType", "10");
	TaskInfoList.insert("type", "10");
	
	bro_arry.push_back(LoadInfoList);
	bro_arry1.push_back(TaskInfoList);
	//QJsonObject bizContentData;
	RootJasonObject.insert("loadInfo", bro_arry);//插入数组;
	RootJasonObject.insert("taskInfo", bro_arry1);//插入数组;
	//RootJasonObject.insert("bizContent",bizContentData);
	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	
	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());
	
	QString outMessage = QString(byte_array);
	qDebug() << outMessage << "\n";
	return byte_array.size();
	
	
	//printf("---------\n");
	
	
}
//#include <QDateTime>
//QString GuoWangProtocol::QDateTime::currentDateTime().toMSecsSinceEpoch()
//{
//	QDateTime currentDateTime = QDateTime::currentDateTime();
//	return currentDateTime.toString("yyyyMMddhhmmss");
//}
