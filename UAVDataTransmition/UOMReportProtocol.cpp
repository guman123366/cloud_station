#include "UOMReportProtocol.h"
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "TD550TelemetryData.h"
UOMReportProtocol::UOMReportProtocol(QObject *parent)
	: ConstructionProtocolInterface(parent)
{
}

UOMReportProtocol::~UOMReportProtocol()
{
}

int UOMReportProtocol::ConstructionData(unsigned char* sendBuf)
{
	if (!ReceiveUAVData)
		return 0;
	TD550TelemetryData* ProtocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!ProtocolData)
		return 0;

	QJsonObject RootJasonObject;
	RootJasonObject.insert("appId", "87878");
	RootJasonObject.insert("charset", "utf-8");
	RootJasonObject.insert("format", "json");
	RootJasonObject.insert("sign", "json");
	RootJasonObject.insert("signType", "md5");
	RootJasonObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	RootJasonObject.insert("version", "1.0");
	
	QJsonArray bro_arry;
	QJsonObject uavDynamicPointList;
	uavDynamicPointList.insert("pubIp"			,"a");
	uavDynamicPointList.insert("bootVersion"	,"a");
	uavDynamicPointList.insert("hwVersion"		,"a");
	uavDynamicPointList.insert("swVersion"		,"a");
	uavDynamicPointList.insert("imsi"			,"a");
	uavDynamicPointList.insert("imei"			,"a");
	uavDynamicPointList.insert("iccid"			,"a");
	uavDynamicPointList.insert("rssi"			,"a");
	uavDynamicPointList.insert("mcc"			,"a");
	uavDynamicPointList.insert("mnc"			,"a");
	uavDynamicPointList.insert("lac"			,"a");
	uavDynamicPointList.insert("cid"			,"a");
	uavDynamicPointList.insert("onlinetime"		,"a");
	uavDynamicPointList.insert("offlinetime"	,"a");
	uavDynamicPointList.insert("coordinate"		,"a");
	uavDynamicPointList.insert("course"			,"a");
	uavDynamicPointList.insert("flightTime"		,"a");
	uavDynamicPointList.insert("gs"				,"a");
	uavDynamicPointList.insert("posiAccuracy"	,"a");
	uavDynamicPointList.insert("height"			,"a");
	uavDynamicPointList.insert("pressure"		,"a");
	uavDynamicPointList.insert("latitude"		,"a");
	uavDynamicPointList.insert("longitude"		,"a");
	uavDynamicPointList.insert("orderId"		,"a");
	uavDynamicPointList.insert("rtTime"			,"a");
	uavDynamicPointList.insert("memo"			,"a");
	uavDynamicPointList.insert("videoCode"		,"a");
	bro_arry.push_back(uavDynamicPointList);

	QJsonObject bizContentData;
	bizContentData.insert("uavDynamicPointList", bro_arry);//插入数组，该数组只有一个元素;
	RootJasonObject.insert("bizContent",bizContentData);
	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	printf("---------\n");

	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());

	return byte_array.size();
}
//#include <QDateTime>
//QString UOMReportProtocol::QDateTime::currentDateTime().toMSecsSinceEpoch()
//{
//	QDateTime currentDateTime = QDateTime::currentDateTime();
//	return currentDateTime.toString("yyyyMMddhhmmss");
//}
