 
#ifndef UOM_DATA_DEFINE_INCLUDE_H
#define UOM_DATA_DEFINE_INCLUDE_H

#include <string>
struct WayPoint
{
	double  longitude;
	double  latitude;
	float   height;
	WayPoint()
		:longitude(0.0), latitude(0.0), height(0.0)
	{
	}
};


struct UAVPose
{
	float yaw;
	float roll;
	float pitch;

	UAVPose()
		:yaw(0.0f), roll(0.0f), pitch(0.0f)
	{
	}
};
 
struct FlightState
{
	std::string flightMode;     //飞行模式
	float  flightDistance; //飞行距离
	std::string flightTime;     //飞行时间
	float  distanceToNext; //到下一点距离 
	float  distanceToHome; //到home点距离 



	float       climbRate;//爬升率;
	///int         customData;
	float       latitude;//纬度;
	float       height;//高度;
	std::string taskId;//任务ID;
	float       airSpeed;//水平飞行速度;
	float       groundSpeed;//地速;
	int         uid;//用户ID;
	float       yaw; //方位角;
	float       roll;//滚转，机身绕中轴线旋转;
	float       pitch;//倾斜 (机头上下摇摆);
	float       longitude;//经度
	int         platformType;//平台类型（数据来源)
	float       voltage;//电池电压
	float       barometerTemp;//气压计温度
	std::string isLocation;//是否定位成功
	float       current;//电流
	int         flightState;//飞行状态
	int         unmannedId;//无人机类型
	float       battaryRemain;//剩余电量
	std::string dateTime; //时间戳：特殊说明，必须是时间戳
	float			imuTemp;//IMU传感器温度
	float		altitude;//海拔高度
	std::string flightSortie;//架次号
	float			satCount;//卫星数
	int			armd; //无人机上锁状态（1 上锁，0解锁）  int类型
	FlightState()
		:flightDistance(0),
		distanceToNext(0), distanceToHome(0)
	{
		climbRate = 0.0;
		latitude = 0.0;
		height = 0.0;
		taskId = "53278";
		airSpeed = 0.0;
		groundSpeed = 0.0;
		uid = 0;
		yaw = 0.0;
		roll = 0.0;
		pitch = 0.0;
		longitude = 0.0;
		platformType = 0;
		voltage = 0.0;
		flightDistance = 0.0;
		barometerTemp = 0.0;
		isLocation = "";
		current = 0.0;
		flightState = 1;
		unmannedId = 1;
		battaryRemain = 0.0;
		dateTime = ""; 
		imuTemp = 0.0;
		altitude = 0.0;
		flightSortie ="zhz20230717";
		satCount = 10.0;
		armd = 0;
		flightMode = "speedMode";
		flightTime = "67";
	}
};


struct UomUAVSate
{
	struct WayPoint     wayPoints;
	struct UAVPose      uavPose;
	struct FlightState  flightSate;

	std::string stu_deviceHardId;//设备ID;
	std::string stu_rawData;//无人机原始数据;
	std::string stu_uavType;//无人机设备类型;
	std::string stu_sn;//厂商的无人机生产序列号;
	//TakeOff：代表当前架次飞行的首个轨迹点   String类型;
	//Inflight：代表当前架次飞行中除首尾以外的其它轨迹点;
	//Land：代表当前架次飞行的最后一个轨迹点;
	std::string stu_flightStatus;
	std::string stu_manufacturerID;// 生产厂商的统一社会信用代码;
	std::string stu_altitude;// 海拔高度;
	std::string stu_changgao;//场高;
	float		vs; // 垂直飞行速度值;
	float		gs; // 水平飞行速度值
	//std::string random;//随机值;
	int stu_appId;
	std::string stu_systemCode;
	int stu_deviceType;
	unsigned short stu_FCCState;
	UomUAVSate()
	{
		stu_appId = 87878;
		stu_systemCode = "UATAIR";
		stu_deviceType = 1;
		vs = 0.0;
		gs = 0.0;
		stu_altitude = 0.0;
		stu_manufacturerID = "91110302057391444C";
		stu_flightStatus = "速度模式";
		stu_sn = "sn987766";
		stu_deviceHardId = "20230815TD220";
		stu_uavType = "TD-550";
	}


};


#endif //UOM_DATA_DEFINE_INCLUDE_H