
#include "UomJsonPackage.h"

//#include "rapidjson/document.h"
//#include "rapidjson/rapidjson.h"
//#include "rapidjson/stringbuffer.h"
//#include "rapidjson/writer.h"
#include"../ThridParty/include/rapidjson/document.h"
#include"../ThridParty/include/rapidjson/rapidjson.h"
#include"../ThridParty/include/rapidjson/stringbuffer.h"
#include"../ThridParty/include/rapidjson/writer.h"


#include <QDateTime>
#include <fstream>
#include <random>

#pragma execution_character_set("utf-8")


UomJsonPackage::UomJsonPackage()
{
	m_nAappId = 87878;
	m_strSystemCode ="UATAIR";
	m_nDeviceType = 1;

	m_strDevId = "20230815TD220";
	m_strUavSn = "sn987766";
	m_strManufacturerID = "91110302057391444C";

	
	m_strUavType = "TD-550";

	m_nPlatformType = 1;
	m_nUnmannedId = 2;
}

std::string UomJsonPackage::GenerateJsonString(const WayPoint& point, const UAVPose& pose, const FlightState& state)
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc.SetObject();


	//设备Id
	rapidjson::Value devId(m_strDevId.c_str(), allocator);
	doc.AddMember("deviceHardId", devId, allocator);

	//无人机原始数据
	rapidjson::Value rawData("");
	doc.AddMember("rawData", rawData, allocator);

	//无人机类型
	rapidjson::Value uavType(m_strUavType.c_str(), allocator);
	doc.AddMember("uavType", uavType, allocator);

	//厂商的无人机生产序列号
	rapidjson::Value uavSn(m_strUavSn.c_str(), allocator);
	doc.AddMember("sn", uavSn, allocator);

	//生产厂商信用代码
	rapidjson::Value manufacturerID(m_strManufacturerID.c_str(), allocator);
	doc.AddMember("manufacturerID", manufacturerID, allocator);
	
	//航点类型
	rapidjson::Value flightStatus("TakeOff");
	doc.AddMember("flightStatus", flightStatus, allocator);

	//海拔高度
	rapidjson::Value altitude(0.0);
	doc.AddMember("altitude", altitude, allocator);

	//垂直速度
	rapidjson::Value vs(0.0);
	doc.AddMember("vs", vs, allocator);

	//水平速度
	rapidjson::Value gs(0.0);
	doc.AddMember("gs", gs, allocator);

	//----------------------------------

	rapidjson::Value deviceData(rapidjson::kObjectType);

	//设备Id
	rapidjson::Value devId0(m_strDevId.c_str(), allocator);
	deviceData.AddMember("deviceHardId", devId0, allocator);


	rapidjson::Value data(rapidjson::kObjectType);


	rapidjson::Value uavInfo(rapidjson::kObjectType);

	//任务ID
	rapidjson::Value taskId(53278);
	uavInfo.AddMember("taskId", taskId, allocator);

	//用户ID
	rapidjson::Value uid(0);
	uavInfo.AddMember("uid", uid, allocator);

	//自定义数据
	rapidjson::Value customData(0);
	uavInfo.AddMember("customData", customData, allocator);

	//爬升率
	rapidjson::Value climbRate(0.0);
	uavInfo.AddMember("climbRate", climbRate, allocator);

	//经度
	rapidjson::Value longitude(point.longitude);
	uavInfo.AddMember("longitude", longitude, allocator);

	//纬度
	rapidjson::Value latitude(point.latitude);
	uavInfo.AddMember("latitude", latitude, allocator);

	//高度
	rapidjson::Value height(point.height);
	uavInfo.AddMember("height", height, allocator);

	//空速
	rapidjson::Value airSpeed(0.0);
	uavInfo.AddMember("airSpeed", airSpeed, allocator);

	//地速
	rapidjson::Value groundSpeed(0.0);
	uavInfo.AddMember("groundSpeed", groundSpeed, allocator);

	//方位角yaw
	rapidjson::Value yaw(pose.yaw);
	uavInfo.AddMember("yaw", yaw, allocator);

	//滚转角roll
	rapidjson::Value roll(pose.roll);
	uavInfo.AddMember("roll", roll, allocator);

	//俯仰角度pitch
	rapidjson::Value pitch(pose.pitch);
	uavInfo.AddMember("pitch", pitch, allocator);

	//距home点距离
	rapidjson::Value distanceToHome(state.distanceToHome);
	uavInfo.AddMember("distanceToHome", distanceToHome, allocator);

	//距下一个航点距离
	rapidjson::Value distanceToNext(state.distanceToNext);
	uavInfo.AddMember("distanceToNext", distanceToNext, allocator);

	//飞行里程
	rapidjson::Value flightDistance(state.flightDistance);
	uavInfo.AddMember("flightDistance", flightDistance, allocator);

	//平台类型
	rapidjson::Value platformType(m_nPlatformType);
	uavInfo.AddMember("platformType", platformType, allocator);

	//电池电压
	rapidjson::Value voltage(0.0);
	uavInfo.AddMember("voltage", voltage, allocator);

	//气压计温度
	rapidjson::Value barometerTemp(0.0);
	uavInfo.AddMember("barometerTemp", barometerTemp, allocator);

	//是否定位成功
	rapidjson::Value isLocation(0);
	uavInfo.AddMember("isLocation", isLocation, allocator);

	//电流
	rapidjson::Value current(0);
	uavInfo.AddMember("current", current, allocator);

	//飞行状态
	rapidjson::Value flightState(0);
	uavInfo.AddMember("flightState", flightState, allocator);

	//无人机类型
	rapidjson::Value unmannedId(m_nUnmannedId);
	uavInfo.AddMember("unmannedId", unmannedId, allocator);

	//剩余油量
	rapidjson::Value battaryRemain(0);
	uavInfo.AddMember("battaryRemain", battaryRemain, allocator);

	//时间戳
	rapidjson::Value dataTime("162218469800");
	uavInfo.AddMember("dateTime", dataTime, allocator);

	//IMU传感器温度
	rapidjson::Value imuTemp(0);
	uavInfo.AddMember("imuTemp", imuTemp, allocator);

	//架次号
	rapidjson::Value flightSortie("zhz20230829");
	uavInfo.AddMember("flightSortie", flightSortie, allocator);

	//飞行模式
	std::string desc = FlightModeConvertDesc(state.flightMode);
	rapidjson::Value flightMode(desc.c_str(), allocator);
	uavInfo.AddMember("flightMode", flightMode, allocator);

	//飞行时常
	rapidjson::Value flightTime(state.flightTime.c_str(), allocator);
	uavInfo.AddMember("flightTime", flightTime, allocator);

	//卫星数
	rapidjson::Value satCount(0);
	uavInfo.AddMember("satCount", satCount, allocator);

	//.....
	rapidjson::Value armd(1);
	uavInfo.AddMember("armd", armd, allocator);

	data.AddMember("uavInfo", uavInfo, allocator);  // 添加uavInfo对象

	//---------------------------------------------------------------------------

	//挂载信息
	rapidjson::Value mountInfo(rapidjson::kArrayType);
	data.AddMember("mountInfo", mountInfo, allocator);

	//视频列表
	rapidjson::Value videoInfo(rapidjson::kArrayType);
	data.AddMember("videoInfo", videoInfo, allocator);

	deviceData.AddMember("data", data, allocator);  // 添加data对象

	//-------------------------------------------------------------------------

	//appId
	rapidjson::Value appId(m_nAappId);
	deviceData.AddMember("appId", appId, allocator);

	//systemCode
	rapidjson::Value systemCode(m_strSystemCode.c_str(), allocator);
	deviceData.AddMember("systemCode", systemCode, allocator);

	//deviceType
	rapidjson::Value deviceType(m_nDeviceType);
	deviceData.AddMember("deviceType", deviceType, allocator);

	doc.AddMember("deviceData", deviceData, allocator); // 添加deviceData对象


	//------------------------------------------------------------------------------------

	//序列化
	rapidjson::StringBuffer strBuffer;
	rapidjson::Writer<rapidjson::StringBuffer,rapidjson::UTF8<char>> writer(strBuffer);
	doc.Accept(writer);
	
	/*	
	const char*  buffer = strBuffer.GetString();
	std::string filename = "test.json";
	std::ofstream ofs(filename);
	if (ofs.is_open())
	{
		ofs << strBuffer.GetString();
		ofs.close();
	}
	*/
	/*
	FILE* fp = fopen("test_1.json", "wb");
	if (fp)
	{
		std::string str(strBuffer.GetString());
		fwrite(str.data(), 1, str.length(), fp);
		fclose(fp);
	}
	 */
	return std::string(strBuffer.GetString());
}

std::string  UomJsonPackage::GenerateJsonString(const UomUAVSate& var)
{
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

	doc.SetObject();

	//
	//设备Id
	rapidjson::Value devId(var.stu_deviceHardId.c_str(), allocator);
	doc.AddMember("deviceHardId", devId, allocator);

	//无人机原始数据
	rapidjson::Value rawData("");
	doc.AddMember("rawData", rawData, allocator);

	//无人机类型
	rapidjson::Value uavType(var.stu_uavType.c_str(), allocator);
	doc.AddMember("uavType", uavType, allocator);

	//厂商的无人机生产序列号
	rapidjson::Value uavSn(var.stu_sn.c_str(), allocator);
	doc.AddMember("sn", uavSn, allocator);

	//生产厂商信用代码
	rapidjson::Value manufacturerID(var.stu_manufacturerID.c_str(), allocator);
	doc.AddMember("manufacturerID", manufacturerID, allocator);

	
	//地空状态;
	if ((var.stu_FCCState & 0x4000) == 0x4000)//地面;
	{
		rapidjson::Value groundType(0);
		doc.AddMember("groundAirType", groundType, allocator);

		//判断是起飞还是降落;
		if (m_last_FlightState == "Inflight")
		{
			m_temp_FlightState = "Land";
		}
		else
		{
			m_temp_FlightState = "TakeOff";
		}
	
	}
	else//空中;
	{
		
		rapidjson::Value groundType(1);
		doc.AddMember("groundAirType", groundType, allocator);
		
		m_temp_FlightState = "Inflight";
	}
	m_last_FlightState = m_temp_FlightState;
	
	printf("----%s\n", m_temp_FlightState.data());
	//航点类型
	//rapidjson::Value flightStatus(var.stu_flightStatus.c_str(), allocator);
	//doc.AddMember("flightStatus", flightStatus, allocator);
	
	
	rapidjson::Value flightStatus(m_temp_FlightState.c_str(), allocator);
	doc.AddMember("flightStatus", flightStatus, allocator);

	//海拔高度
	rapidjson::Value altitude(var.stu_altitude.c_str(), allocator);
	doc.AddMember("altitude", altitude, allocator);

	//垂直速度
	rapidjson::Value vs(var.vs);
	doc.AddMember("vs", vs, allocator);

	//水平速度
	rapidjson::Value gs(var.gs);
	doc.AddMember("gs", gs, allocator);

	
	//if (m_temp_FlightState/*.c_str()*/ == "TakeOff" && m_flag)//产生随机值;
	//{
	//	std::random_device rd;
	//	std::mt19937 gen(rd());
	//	const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	//	std::uniform_int_distribution<> dis(0, sizeof(charset) - 2); // sizeof(charset) - 1 is the null terminator
	//	std::string code;
	//	for (int i = 0; i < 8; ++i) {
	//		code += charset[dis(gen)];
	//	}
	//
	//	//random值
	//	rapidjson::Value random(code.c_str(), allocator);
	//	doc.AddMember("random", random, allocator);
	//	m_temp_random = code;
	//	m_flag = false;
	//}
	//else if (m_temp_FlightState == "Land")
	//{
	//	rapidjson::Value random(m_temp_random.c_str(), allocator);
	//	doc.AddMember("random", random, allocator);
	//	m_flag = true;
	//}
	//else//还用之前的;
	//{
	//	rapidjson::Value random(m_temp_random.c_str(), allocator);
	//	doc.AddMember("random", random, allocator);
	//}
	//----------------------------------

	rapidjson::Value deviceData(rapidjson::kObjectType);

	//设备Id
	rapidjson::Value devId0(var.stu_deviceHardId.c_str(), allocator);
	deviceData.AddMember("deviceHardId", devId0, allocator);


	rapidjson::Value data(rapidjson::kObjectType);


	rapidjson::Value uavInfo(rapidjson::kObjectType);

	//任务ID
	rapidjson::Value taskId(var.flightSate.taskId.c_str(), allocator);
	uavInfo.AddMember("taskId", taskId, allocator);

	//用户ID
	rapidjson::Value uid(var.flightSate.uid);
	uavInfo.AddMember("uid", uid, allocator);

	//自定义数据
	rapidjson::Value customData(0);
	uavInfo.AddMember("customData", customData, allocator);

	//爬升率
	rapidjson::Value climbRate(var.flightSate.climbRate);
	uavInfo.AddMember("climbRate", climbRate, allocator);

	//经度
	rapidjson::Value longitude(var.flightSate.longitude);
	uavInfo.AddMember("longitude", longitude, allocator);

	//纬度
	rapidjson::Value latitude(var.flightSate.latitude);
	uavInfo.AddMember("latitude", latitude, allocator);



	//高度(距离起飞点的高度)
	double aaa = atof(var.stu_altitude.data());
	double bbb = atof(var.stu_changgao.data());
	rapidjson::Value height(aaa - bbb);
	uavInfo.AddMember("height", height, allocator);

	//空速
	rapidjson::Value airSpeed(var.flightSate.airSpeed);
	uavInfo.AddMember("airSpeed", airSpeed, allocator);

	//地速
	rapidjson::Value groundSpeed(var.flightSate.groundSpeed);
	uavInfo.AddMember("groundSpeed", groundSpeed, allocator);

	//方位角yaw
	rapidjson::Value yaw(var.flightSate.yaw);
	uavInfo.AddMember("yaw", yaw, allocator);

	//滚转角roll
	rapidjson::Value roll(var.flightSate.roll);
	uavInfo.AddMember("roll", roll, allocator);

	//俯仰角度pitch
	rapidjson::Value pitch(var.flightSate.pitch);
	uavInfo.AddMember("pitch", pitch, allocator);

	//距home点距离
	rapidjson::Value distanceToHome(var.flightSate.distanceToHome);
	uavInfo.AddMember("distanceToHome", distanceToHome, allocator);

	//距下一个航点距离
	rapidjson::Value distanceToNext(var.flightSate.distanceToNext);
	uavInfo.AddMember("distanceToNext", distanceToNext, allocator);

	//飞行里程
	rapidjson::Value flightDistance(var.flightSate.flightDistance);
	uavInfo.AddMember("flightDistance", flightDistance, allocator);

	//平台类型
	rapidjson::Value platformType(var.flightSate.platformType);
	uavInfo.AddMember("platformType", platformType, allocator);

	//电池电压
	rapidjson::Value voltage(var.flightSate.voltage);
	uavInfo.AddMember("voltage", voltage, allocator);

	//气压计温度
	rapidjson::Value barometerTemp(var.flightSate.barometerTemp);
	uavInfo.AddMember("barometerTemp", barometerTemp, allocator);

	//是否定位成功
	rapidjson::Value isLocation(var.flightSate.isLocation.c_str(), allocator);
	//rapidjson::Value isLocation("test", allocator);
	uavInfo.AddMember("isLocation", isLocation, allocator);

	//电流
	rapidjson::Value current(0);
	uavInfo.AddMember("current", current, allocator);

	//飞行状态
	rapidjson::Value flightState(0);
	uavInfo.AddMember("flightState", flightState, allocator);

	//无人机类型
	rapidjson::Value unmannedId(var.flightSate.unmannedId);
	uavInfo.AddMember("unmannedId", unmannedId, allocator);

	//剩余油量
	rapidjson::Value battaryRemain(var.flightSate.battaryRemain);
	uavInfo.AddMember("battaryRemain", battaryRemain, allocator);

	QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz");
	qint64 nCurrentTimeCount = QDateTime::currentDateTime().msecsTo(QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0))) + 8 * 60 * 60 * 1000;		//加上时区的八个小时
	//UavInfoObject.insert("dateTime", -nCurrentTimeCount);//时间戳
	//时间戳
	rapidjson::Value dataTime(-nCurrentTimeCount);
	uavInfo.AddMember("dateTime", dataTime, allocator);

	//IMU传感器温度
	rapidjson::Value imuTemp(var.flightSate.imuTemp);
	uavInfo.AddMember("imuTemp", imuTemp, allocator);

	//架次号
	rapidjson::Value flightSortie(var.flightSate.flightSortie.c_str(), allocator);
	uavInfo.AddMember("flightSortie", flightSortie, allocator);

	//飞行模式
	//std::string desc = "testMode";
	rapidjson::Value flightMode(var.flightSate.flightMode.c_str(), allocator);
	uavInfo.AddMember("flightMode", flightMode, allocator);

	//飞行时常
	rapidjson::Value flightTime(var.flightSate.flightTime.c_str(), allocator);
	uavInfo.AddMember("flightTime", flightTime, allocator);

	//卫星数
	rapidjson::Value satCount(var.flightSate.satCount);
	uavInfo.AddMember("satCount", satCount, allocator);

	//.....
	rapidjson::Value armd(var.flightSate.armd);
	uavInfo.AddMember("armd", armd, allocator);

	data.AddMember("uavInfo", uavInfo, allocator);  // 添加uavInfo对象

	//---------------------------------------------------------------------------

	//挂载信息
	rapidjson::Value mountInfo(rapidjson::kArrayType);
	data.AddMember("mountInfo", mountInfo, allocator);

	//视频列表
	rapidjson::Value videoInfo(rapidjson::kArrayType);
	data.AddMember("videoInfo", videoInfo, allocator);

	deviceData.AddMember("data", data, allocator);  // 添加data对象

	//-------------------------------------------------------------------------

	//appId
	rapidjson::Value appId(var.stu_appId);
	deviceData.AddMember("appId", appId, allocator);

	//systemCode
	rapidjson::Value systemCode(var.stu_systemCode.c_str(), allocator);
	deviceData.AddMember("systemCode", systemCode, allocator);

	//deviceType
	rapidjson::Value deviceType(var.stu_deviceType);
	deviceData.AddMember("deviceType", deviceType, allocator);

	doc.AddMember("deviceData", deviceData, allocator); // 添加deviceData对象


	//------------------------------------------------------------------------------------

	//序列化
	rapidjson::StringBuffer strBuffer;
	rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<char>> writer(strBuffer);
	doc.Accept(writer);

	/*
	const char*  buffer = strBuffer.GetString();
	std::string filename = "test.json";
	std::ofstream ofs(filename);
	if (ofs.is_open())
	{
	ofs << strBuffer.GetString();
	ofs.close();
	}
	*/
	/*
	FILE* fp = fopen("test_1.json", "wb");
	if (fp)
	{
	std::string str(strBuffer.GetString());
	fwrite(str.data(), 1, str.length(), fp);
	fclose(fp);
	}
	*/
	return std::string(strBuffer.GetString());
}
bool  UomJsonPackage::ParseTokenStatus(const std::string&  json, std::string& strToken)
{
	rapidjson::Document doc;
	doc.Parse(json.c_str(), json.length());

	if (doc.HasParseError())
	{
		return false;
	}

	//---------------------------------------
	if (!doc.HasMember("status"))
	{
		return  false;
	}

	rapidjson::Value& statusVal = doc["status"];
	if (!statusVal.IsInt())
	{
		return false;
	}
	int nStatus = statusVal.GetInt();
	if (nStatus != 1)
	{
		return false;
	}


	//---------------------------------------
	if (!doc.HasMember("data"))
	{
		return false;
	}

	rapidjson::Value& dataObj = doc["data"];
	if (!dataObj.IsObject())
	{
		return false;
	}

	//----------------------------------

	if (!dataObj.HasMember("token"))
	{
		return  false;
	}

	rapidjson::Value& tokenVal = dataObj["token"];
	if (tokenVal.IsString())
	{
		strToken = tokenVal.GetString();
		return true;
	}
	else
	{
		return  false;
	}
	

}


bool  UomJsonPackage::ParseUomStatus(const std::string& json)
{
	rapidjson::Document doc;
	doc.Parse(json.c_str(), json.length());

	if (doc.HasParseError())
	{
		return false;
	}

	//---------------------------------------
	if (!doc.HasMember("status"))
	{
		return  false;
	}

	rapidjson::Value& statusVal = doc["status"];
	if (!statusVal.IsInt())
	{
		return false;
	}
	int nStatus = statusVal.GetInt();
	if (nStatus != 1)
	{
		return false;
	}

	return true;
}


#include <QString>
std::string UomJsonPackage::FlightModeConvertDesc(std::string mode)
{
    std::string  desc("速度模式");
	//QString aa = QString("速度模式");
	//std::string desc = aa.toLocal8Bit();
	return desc;
};


UomJsonPackage::~UomJsonPackage()
{


}
