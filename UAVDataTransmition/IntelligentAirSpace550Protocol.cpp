#include "IntelligentAirSpace550Protocol.h"
#include "TD550TelemetryData.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "IntelligentAirSpaceData.h"
#include <QDateTime>
#include <qDebug>

IntelligentAirSpace550Protocol::IntelligentAirSpace550Protocol(QObject *parent)
	: ConstructionProtocolInterface(parent)
{
}

IntelligentAirSpace550Protocol::~IntelligentAirSpace550Protocol()
{
}

int IntelligentAirSpace550Protocol::ConstructionData(unsigned char* sendBuf)
{
	if (!ReceiveUAVData)
		return 0;

	TD550TelemetryData* ProtocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!ProtocolData)
		return 0;

	QJsonObject RootJasonObject;
	QString DataTemp = "";
	double ValueTemp = 0.0;
	RootJasonObject.insert("deviceHardId", "ZHZ301R22208032");
	RootJasonObject.insert("appId", 87878);
	QJsonObject DeviceDataObject;
	DeviceDataObject.insert("deviceHardId", "ZHZ301R22208032");
	QJsonObject DataJasonObject;
	DataJasonObject.insert("openid", "13234");
	QJsonObject UavInfoObject;
	UavInfoObject.insert("armd", 1);
	UavInfoObject.insert("climbRate", ProtocolData->m_FirstSubThreeFrame.ZSpeed);//爬升率
	QJsonObject CustomJasonObject;
	CustomJasonObject.insert("maxHeight", 6500);
	UavInfoObject.insert("customData", QJsonValue(CustomJasonObject));//最大飞行高度
	UavInfoObject.insert("latitude", ProtocolData->m_FirstSubTwoFrame.lat);//纬度
	UavInfoObject.insert("height", ProtocolData->m_FirstSubThreeFrame.RelHeight);//高程/相对高度
	UavInfoObject.insert("taskId", 53278);//任务ID
	ValueTemp = ProtocolData->m_FirstSubThreeFrame.XSpeed;
	UavInfoObject.insert("airSpeed", ValueTemp);//飞行速度
	ValueTemp = abs(sqrt(pow(ProtocolData->m_FirstSubThreeFrame.XSpeed, 2) + pow(ProtocolData->m_FirstSubThreeFrame.YSpeed, 2)));
	UavInfoObject.insert("groundSpeed", ValueTemp);//地速
	ValueTemp = ProtocolData->m_SecondSubThreeFrame.DIS_XY;
	UavInfoObject.insert("distanceToHome", ValueTemp);//距离Home点距离
	UavInfoObject.insert("distanceToNext", ProtocolData->m_SecondSubOneFrame.FlushingDis);//到下一航点距离
	UavInfoObject.insert("uid", 0);//用户ID
	ValueTemp = ProtocolData->m_FirstSubThreeFrame.Yaw;
	ValueTemp = ValueTemp >= 0 ? ValueTemp : 360 + ValueTemp;
	UavInfoObject.insert("yaw", ValueTemp);//方位角
	ValueTemp = ProtocolData->m_FirstSubThreeFrame.Roll;
	UavInfoObject.insert("roll", ValueTemp);//滚转角
	ValueTemp = ProtocolData->m_FirstSubThreeFrame.Pitch;
	UavInfoObject.insert("pitch", ValueTemp);//倾斜角
	UavInfoObject.insert("longitude", ProtocolData->m_FirstSubTwoFrame.lon);//经度
	UavInfoObject.insert("platformType", 0);//平台类型
	UavInfoObject.insert("voltage", ProtocolData->m_SecondSubFourFrame.Power24V);//电池电压
	UavInfoObject.insert("flightDistance", 0);//飞行里程
	UavInfoObject.insert("barometerTemp", -(ProtocolData->m_FirstSubOneFrame.StaticTemp));//气压计温度
	QString strTemp = "";
	if (ProtocolData->m_SecondSubTwoFrame.NaviState == 0)
	{
		strTemp = "未定位";
	}
	else if (ProtocolData->m_SecondSubTwoFrame.NaviState == 1)
	{
		strTemp = "单点";
	}
	else if (ProtocolData->m_SecondSubTwoFrame.NaviState ==2)
	{
		strTemp = "伪距差分";
	}
	else if (ProtocolData->m_SecondSubTwoFrame.NaviState == 3)
	{
		strTemp = "载波差分";
	}
	UavInfoObject.insert("isLocation", strTemp);//是否定位成功
	UavInfoObject.insert("current", 0);//电流
	UavInfoObject.insert("flightState", 1);//飞行状态
	UavInfoObject.insert("unmannedId", 1);//无人机类型
	UavInfoObject.insert("battaryRemain", ProtocolData->m_ThirdSubTwoFrame.Oil);//剩余油量
	QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz");
	qint64 nCurrentTimeCount = QDateTime::currentDateTime().msecsTo(QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0))) + 8 * 60 * 60 * 1000;		//加上时区的八个小时
	UavInfoObject.insert("dateTime", -nCurrentTimeCount);//时间戳
	UavInfoObject.insert("imuTemp", 0);//IMU传感器温度
	UavInfoObject.insert("altitude", ProtocolData->m_FirstSubThreeFrame.AbsolutelyHeight);//高程/海拔高度
	UavInfoObject.insert("flightSortie", "zhz20230829");//架次号
	UavInfoObject.insert("flightMode", "速度模式");//飞行模式
	UavInfoObject.insert("flightTime", 100);//飞行时长
	UavInfoObject.insert("satCount", ProtocolData->m_SecondSubOneFrame.MainNavNum);//卫星数
	DataJasonObject.insert("uavInfo", QJsonValue(UavInfoObject));
	QJsonArray mountInfoJasonArray;
	DataJasonObject.insert("mountInfo", QJsonValue(mountInfoJasonArray));
	QJsonArray videoInfoJasonArray;
	QJsonObject videoJsonObject;
	videoJsonObject.insert("vUrl", "webrtc://120.79.35.124:1935/live/livestream");
	videoJsonObject.insert("videoType", "flv");
	videoInfoJasonArray.append(QJsonValue(videoJsonObject));
	DataJasonObject.insert("videoInfo", QJsonValue(videoInfoJasonArray));
	DeviceDataObject.insert("data", QJsonValue(DataJasonObject));
	DeviceDataObject.insert("appId", 87878);
	DeviceDataObject.insert("systemCode", "UATAIR");
	DeviceDataObject.insert("deviceType", 1);
	RootJasonObject.insert("deviceData", QJsonValue(DeviceDataObject));

	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());

	QString outMessage = QString(byte_array);
	//qDebug() << outMessage << "\n";
	return byte_array.size();
}
