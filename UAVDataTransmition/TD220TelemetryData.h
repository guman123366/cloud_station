/*
	TD220便携控制软件遥测数据
*/
#pragma once

#include "DataDefineInterface.h"
#include <string>

static const double ZHZ_GLOBAL_ZERO = 1.0e-9;
static const double ZHZ_GLOBAL_PI = 3.1415926535897932384626433832795;
static const double ZHZ_GLOBAL_dRadianToAngle = 57.295779513082320876846364344191;		// 180/Pi 弧度转角度
static const double ZHZ_GLOBAL_dAngleToRadian = 0.017453292519943295;					// pi/180 角度转弧度
static const double ZHZ_GLOBAL_dEarthMeanRadius = 6371.393;							    // kilometers


#define FALSE 0
#define TRUE  1
static const int ZHZ_GLOBAL_nVersion = 100;
static const int ZHZ_GLOBAL_IDA5 = 0xA5;
static const int ZHZ_GLOBAL_ID55 = 0x55;

//ACK = acknowledge 承认
//NACK = No acknowledge 不承认
enum ZHZ_UavRxMsg
{
	UAV_NO_MSG = 0,				//没有收到数据 
	UAV_TELEMETRY_DATA_MSG,		//收到遥测数据 
	UAV_ID_MSG,					//成功收到飞机ID
	UAV_ACK_ASSISTED_MODE,		//切换速度模式指令发送 成功
	UAV_NACK_ASSISTED_MODE,		//切换速度模式指令发送 失败
	UAV_ACK_MISSION_MODE,			//飞另点指令发送 成功
	UAV_NACK_MISSION_MODE,		//飞另点指令发送 失败
	UAV_ACK_HOME_MODE,			//归航指令发送 成功
	UAV_NACK_HOME_MODE,			//归航指令发送 失败
	UAV_ACK_CONTINUE_MISSION,		//飞下点指令发送 成功
	UAV_NACK_CONTINUE_MISSION,	//飞下点指令发送 失败
	UAV_ACK_DELETE_WP_LIST,		//删除航线 成功(航点列表) 
	UAV_NACK_DELETE_WP_LIST,		//删除航线 失败(航点列表) 
	UAV_ACK_CONTROL_DATA,			//控制指令发送 成功
	UAV_NACK_CONTROL_DATA,		//控制指令发送 失败
	UAV_ACK_WAYPOINT_DATA,		//航点上传 成功
	UAV_NACK_WAYPOINT_DATA,		//航点上传 失败
	UAV_ACK_TAKEOFF,				//起飞指令发送 成功
	UAV_NACK_TAKEOFF,				//起飞指令发送 失败
	UAV_ACK_LANDING,				//着陆指令发送 成功
	UAV_NACK_LANDING,				//着陆指令发送 失败
	UAV_ACK_SHUTDOWNENGINE,		//关引擎指令发送 成功
	UAV_NACK_SHUTDOWNENGINE,		//关引擎指令发送 失败
	UAV_ACK_TESTSERVOS,			//测试伺服指令发送 成功
	UAV_NACK_TESTSERVOS,			//测试伺服指令发送 失败
	UAV_ACK_CONFIG_DATA,			//初始化配置指令发送 成功
	UAV_NACK_CONFIG_DATA,			//初始化配置指令发送 失败
	UAV_ACK_MAG_DECLINATION,		//磁偏角指令发送 成功
	UAV_NACK_MAG_DECLINATION,		//磁偏角指令发送 失败
	UAV_ACK_CONTROL_PAR,			//初始化配置指令（PAR）发送 成功
	UAV_NACK_CONTROL_PAR,			//初始化配置指令（PAR）发送 失败
	UAV_PLY_MSG,

	//此后的一些指令是地面站自己控制的，与飞控无关
	UAV_ACK_ROUTE_DATA = 200,		//航线上传 成功

};

enum ZHZ_ProtocolTelemetryDataID
{
	TelemetryDataID_POSNAVREL = 97,					//0x61	相对位置 
	TelemetryDataID_VELNAV = 98,					//0x62	速度

	TelemetryDataID_POSGPSABS = 104,				//0x68	GPS绝对位置
	TelemetryDataID_POSNAVABS = 105,				//0x69	
	TelemetryDataID_AIRCRAFT = 106,					//0x6A	航空器
	TelemetryDataID_STATUS = 107,					//0x6B	主轴转速/卫星定位等/遥控器/电池电压/舵机电压
	TelemetryDataID_GUIDANCE = 108,					//0x6C	指导


	TelemetryDataID_ATTITUDE = 150,					//0x96	姿态
	TelemetryDataID_CONTROL = 159,					//0x9F	控制 dialsWidget.ui界面油门等信息

	TelemetryDataID_REFERENCE = 161,				//0xA1	参考 dialsWidget.ui爬升速度、转向速度、侧飞、前飞

	TelemetryDataID_GPSUTCTIME = 170,				//0xAA
	TelemetryDataID_GIMBAL = 171,					//0xAB
	TelemetryDataID_TRUEAGL = 172,					//0xAC
	TelemetryDataID_SigCondition = 173,				//0xAD
	TelemetryDataID_ServoCommand = 174,				//0xAE	伺服电机控制
	TelemetryDataID_ServoCurrent = 175,				//0xAF	伺服电机当前 舵机电流
	TelemetryDataID_ADValue = 176,					//0xB0	水温/排温/CPU温度
	TelemetryDataID_AirSpeedVal = 177,				//0xB1	
	TelemetryDataID_GPS_BD_Status = 178,			//0xB2	GPS/BD STATUS
};

//遥控器
enum ZHZ_UavRCStatus
{
	UAV_RC_OK,		//遥控器可用
	UAV_RC_LOST,		//遥控器丢失
	UAV_RC_DISABLED	//遥控器失效
};



//GPS状态
enum ZHZ_UavGPSStatus
{
	UAV_GPS_INIT,			//GPS初始化
	UAV_GPS_VALID,		//GPS有效
	UAV_GPS_NOTVALID,		//GPS无效
	UAV_GPS_VALID_DGPS,
	UAV_GPS_LOST			//GPS丢失
};


//驾驶仪状态
enum ZHZ_UavFcsState
{
	UAV_FCS_STATE_NOTREADY,			//未就绪
	UAV_FCS_STATE_READY,			//就绪
	UAV_FCS_STATE_TESTSERVOS,		//测试伺服系统
	UAV_FCS_STATE_TAKEOFF,			//起飞
	UAV_FCS_STATE_LANDING,			//着陆
	UAV_FCS_STATE_SHUTDOWNENGINE,	//关引擎
	UAV_FCS_STATE_HOVER,			//悬停
	UAV_FCS_STATE_CRUISE,			//巡航
	UAV_FCS_STATE_GOTO_WP,			//飞往航点
	UAV_FCS_STATE_STOP_AT_WP,		//停在航点
	UAV_FCS_STATE_GOTO_HOME,		//归航
	UAV_FCS_STATE_STOP_AT_HOME,		//停在归航点
	UAV_FCS_STATE_LINKLOST			//失去连接
};

#ifndef MAXDATA
#define MAXDATA(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MINDATA
#define MINDATA(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//顾东刚 2019-4-22 15-43-28
//定位模式
enum ZHZ_SatelliteMode
{
	Sate_NoPos_Mode = 0,
	Sate_PointPos_Mode,
	Sate_PesudorangePos_Mode,
	Sate_DiffPos_Mode,
};
//导航模式
enum ZHZ_NavigationMode
{
	Navi_Pure_Mode = 0,
	Navi_Pure_Sate_Mode,
	Navi_Pure_Pressure_Mode,
	Navi_Wireless_Pressure_Mode,
	Navi_Sate_Pressure_Mode,
	Navi_Standy_Mode,
	Navi_Quick_Mode,
	Navi_Normal_Mode,
	Navi_Air_Mode,
	Navi_IMU_Magne_Radio_Mode,
};

//飞机控制结构体
struct ZHZ_ControlData
{
	ZHZ_ControlData()
	{
		memset(payloadData, 0, 8);
		m_dForwSpeed = 0.0;
		m_dSideSpeed = 0.0;
		m_dClimbSpeed = 0.0;
		m_dHeadingRate = 0.0;
		m_dThrottle = 0.0;
	}

	char payloadData[8]; // Survey-Copter specific data
	double m_dForwSpeed;    //前飞速度命令	forward speed [m/s]
	double m_dSideSpeed;    //侧飞速度命令	sideward speed [m/s]
	double m_dClimbSpeed;   //爬升速度命令	climb rate [m/s]
	double m_dHeadingRate;  //转向速度命令	heading rate [rad/s]
	double m_dThrottle;     //油门命令		throttle [%], range [0,+100]
};

//命令值、参考值和实际值
struct ZHZ_ValueEx
{
	ZHZ_ValueEx()
	{
		m_dCommandVal = 0.0;	//命令值
		m_dReferenceVal = 0.0;	//参考值
		m_dPracticalVal = 0.0;	//实际值
	}

	double m_dCommandVal;	//命令值
	double m_dReferenceVal;	//参考值
	double m_dPracticalVal;	//实际值
};

//舵机位置 舵机电流
struct ZHZ_ServoPosition
{
	ZHZ_ServoPosition()
	{
		pitch = 0.0;
		roll = 0.0;
		collection = 0.0;
		throttle = 0.0;
		heading = 0.0;
		rudder = 0.0;
	}

	double pitch;		// Pitch/D1
	double roll;		// Roll/D2
	double collection;	// Collect/D3
	double throttle;	// Throttle/U1
	double heading;		// Heading/U2
	double rudder;		// Rudder/U3
};

//姿态
struct ZHZ_Attitude
{
	ZHZ_Attitude()
	{
		roll = 0.0;
		pitch = 0.0;
		yaw = 0.0;
	}

	double roll;	//滚转 rool angle[rad],range= [-pi,+pi]
	double pitch;	//俯仰 pitch angle[rad],range= [-pi,+pi]
	double yaw;		//偏航 yaw angle[rad],range= [-pi,+pi]
};

//绝对位置 Abs = absolute(绝对)
struct ZHZ_AbsPosition
{
	ZHZ_AbsPosition()
	{
		m_dLat = 0.0;
		m_dLon = 0.0;
		m_dAlt = 0.0;
	}

	double m_dLat;		//纬度 latitude [rad]
	double m_dLon;		//经度 longitude [rad]
	double m_dAlt;		//高度 altitude [m]
};

//相对位置 rel = relative(相对的)
struct ZHZ_RelPosition
{
	ZHZ_RelPosition()
	{
		m_dNorth = 0.0;
		m_dEast = 0.0;
		m_dDown = 0.0;
	}

	double m_dNorth;	//position in north direction [m]
	double m_dEast;		//position in east direction [m]
	double m_dDown;		//position in down direction [m]
};

//速度
struct ZHZ_Velocity
{
	ZHZ_Velocity()
	{
		m_dNorth = 0.0;
		m_dEast = 0.0;
		m_dDown = 0.0;
	}

	double m_dNorth;			//velocity in north direction [m/s]
	double m_dEast;			//velocity in east direction [m/s]
	double m_dDown;			//velocity in down direction [m/s]
};

//无人机基本速度
struct ZHZ_UAVBaseVelocity
{
	ZHZ_UAVBaseVelocity()
	{
		m_dForward = 0.0;
		m_dSide = 0.0;
		m_dSwerve = 0.0;
		m_dClimb = 0.0;
	}

	double m_dForward;	//前飞速度
	double m_dSide;		//侧飞速度
	double m_dSwerve;	//转向速度
	double m_dClimb;	//爬升速度
};

//参考数据
struct ZHZ_References
{
	ZHZ_References()
	{
		m_dVel_north = 0.0;
		m_dVel_east = 0.0;
		m_dVel_down = 0.0;
		m_dHeading = 0.0;
		m_dPos_north = 0.0;
		m_dPos_east = 0.0;
		m_dPos_down = 0.0;
		m_dHeading_rate = 0.0;
	}

	double m_dVel_north;	//reference velocity in north direction [m/s]
	double m_dVel_east;		//reference velocity in east direction [m/s]
	double m_dVel_down;		//reference velocity in down direction [m/s]
	double m_dHeading;		//reference heading [rad]
	double m_dPos_north;	//北 reference position in north direction [m]
	double m_dPos_east;		//东 reference position in east direction [m]
	double m_dPos_down;		//下 reference position in down direction [m]
	double m_dHeading_rate;	//reference heading rate [rad/s]
};

//航点信息
struct ZHZ_WaypointInfo
{
	ZHZ_WaypointInfo()
	{
		m_nWPNo = 0;
		m_dDist = 0.0;
		m_nIsValid = 0;
	}

	int m_nWPNo;			//waypoint number [0,255]
	double m_dDist;			//distance to waypoint [m]
	int m_nIsValid;			//waypoint table status: 0=empty, 1=valid
};

struct ZHZ_ADValue
{
	ZHZ_ADValue()
	{
		m_dAirTemp1 = 0.0;
		m_dAirTemp2 = 0.0;
		m_dAirSpeed = 0.0;
		m_dWaterTemp = 0.0;
		m_dCpuTemp = 0.0;
		m_dLastOil = 0.0;
	}
	//temperature --> Temp
	double m_dAirTemp1;		//排气温度1  ℃
	double m_dAirTemp2;		//排气温度2  ℃ 或者散热前水温
	double m_dAirSpeed;		//飞机速度(空速度) m/s
	double m_dWaterTemp;	//散热后水温 ℃
	double m_dCpuTemp;		//cpu温度   ℃
	double m_dLastOil;		//剩余油量   L(暂时没有确定是V(伏特)还是L(升),因为协议里也没有写。因为解析的过程中我除了1000,推测应该是V)
};

//顾东刚 2018-12-7 13-51-42
struct ZHZ_ModeInfo
{
	ZHZ_ModeInfo()
	{
		m_RpmState1 = "";
		m_RpmState2 = "";
		m_SatelliteMode = "";
		m_NavigationMode = "";
	}
	std::string m_RpmState1;
	std::string m_RpmState2;
	std::string m_SatelliteMode;
	std::string m_NavigationMode;
};

//顾东刚 2019-5-21 15-56-15
//干扰型链路信息
struct ZHZ_LinkInfoParam
{
	ZHZ_LinkInfoParam()
	{
		m_nOnChannel = 0;
		m_nOnLock = 0;
		m_nOnRecAGC = 0;
		m_nOnDesAGC = 0;
		m_strDownPower = "";
		m_nDownChannel = 0;
		m_nDownLock = 0;
		m_nDownRecAGC = 0;
		m_nDownDesAGC = 0;
		m_strOnPower = "";
		m_nVITERBCode = 0;
	}
	int m_nOnChannel;
	int m_nOnLock;
	int m_nOnRecAGC;
	int m_nOnDesAGC;
	std::string m_strDownPower;
	int m_nDownChannel;
	int m_nDownLock;
	int m_nDownRecAGC;
	int m_nDownDesAGC;
	std::string m_strOnPower;
	int m_nVITERBCode;
};

//告警信息
struct ZHZ_AIRCRAFT
{
	ZHZ_AIRCRAFT()
	{
		//{ 0.0, 0.0, 0, 0,0, UAV_RC_DISABLED, UAV_GPS_NOTVALID, 0 }
		m_dBatFCS = 0.0;
		m_dBatServos = 0.0;
		m_nFuelLevel = 0;
		m_nEngineTemp = 0;
		//系留项目特有水温、CPU温度、
		m_dWaterTemp = 0.0;
		m_dCPUTemp = 0.0;
		m_dAirSpeed = 0.0;

		m_nRotorRPM = 0;
		RCStatus = UAV_RC_DISABLED;
		GPSStatus = UAV_GPS_NOTVALID;
		m_nSat = 0;

		//顾东刚 2019-3-19 17-51-32
		m_nFanRPM1 = 0;
		m_nFanRPM2 = 0;

		//顾东刚 2019-4-22 14-36-43
		m_nRpmSensor1 = 0;
		m_nRpmSensor2 = 0;
		m_nWaterSensor = 0;
		m_nRpmState1 = 0;
		m_nRpmState2 = 0;
		m_SateMode = Sate_NoPos_Mode;
		m_NaviMode = Navi_Pure_Mode;
		m_nFanRpmSensor1 = 0;
		m_nFanRpmSensor2 = 0;
		m_nAtmosphComm = 0;
		m_nAtmosphCalib = 0;
		m_nInertialNaviComm = 0;
		m_nInertialNaviCalib = 0;
		m_nBDComm = 0;
		m_nBDPos = 0;
		m_nBDDestruct = 0;
		m_nSteer1Comm = 0;
		m_nSteer1Calib = 0;
		m_nSteer2Comm = 0;
		m_nSteer2Calib = 0;
		m_nSteer3Comm = 0;
		m_nSteer3Calib = 0;
		m_nSteer4Comm = 0;
		m_nSteer4Calib = 0;
		m_nSteer5Comm = 0;
		m_nSteer5Calib = 0;
		m_nSteer6Comm = 0;
		m_nSteer6Calib = 0;
		m_nLowTension7 = 0;
		m_nHighTension7 = 0;
		m_nLowTension12 = 0;
		m_nHighTension12 = 0;
		m_nLowTension28 = 0;
		m_nHighTension28 = 0;
		m_nNacelleComm = 0;
		m_nWaterTemp = 0;
	}

	double m_dBatFCS;					//电池电压 battery FCS [Volts
	double m_dBatServos;				//舵机电压 battery Servos [Volts]
	int m_nFuelLevel;					//燃料水平 fuel level [%], range = [0,+100]	
	int m_nEngineTemp;					//引擎温度 engine temperature [℃]
	//系留项目特有水温、CPU温度、
	double m_dWaterTemp;				//水温
	double m_dCPUTemp;					//CPU温度
	double m_dAirSpeed;					//空速

	int m_nRotorRPM;					//主轴转速 转/分
	enum ZHZ_UavRCStatus RCStatus;		//遥控器状态 (RC = remote control)
	enum ZHZ_UavGPSStatus GPSStatus;
	int m_nSat;							//卫星定位颗数 number of GPS satellites, range = [0,15]	//GPS

	//顾东刚 2019-3-19 17-48-55
	int m_nFanRPM1;						//风扇1转速
	int m_nFanRPM2;						//风扇2转速

	//顾东刚 2019-4-22 14-35-14
	int m_nRpmSensor1;					//转速传感器1转速
	int m_nRpmSensor2;					//转速传感器2转速
	int m_nWaterSensor;					//水温传感器
	int m_nRpmState1;					//转速传感器1
	int m_nRpmState2;					//转速传感器2
	ZHZ_SatelliteMode m_SateMode;		//卫星定位模式
	ZHZ_NavigationMode m_NaviMode;		//导航模式
	int m_nFanRpmSensor1;				//风扇转速传感器1
	int m_nFanRpmSensor2;				//风扇转速传感器2
	int m_nAtmosphComm;					//大气处理机通信
	int m_nAtmosphCalib;				//大气处理机校准
	int m_nInertialNaviComm;			//惯导通信
	int m_nInertialNaviCalib;			//惯导校准
	int m_nBDComm;						//北斗通信
	int m_nBDPos;						//北斗定位
	int m_nBDDestruct;					//北斗自毁
	int m_nSteer1Comm;					//舵机1通信
	int m_nSteer1Calib;					//舵机1校准
	int m_nSteer2Comm;					//舵机2通信
	int m_nSteer2Calib;					//舵机2校准
	int m_nSteer3Comm;					//舵机3通信
	int m_nSteer3Calib;					//舵机3校准
	int m_nSteer4Comm;					//舵机4通信
	int m_nSteer4Calib;					//舵机4校准
	int m_nSteer5Comm;					//舵机5通信
	int m_nSteer5Calib;					//舵机5校准
	int m_nSteer6Comm;					//舵机6通信
	int m_nSteer6Calib;					//舵机6校准
	int m_nLowTension7;					//7.4V低压
	int m_nHighTension7;				//7.4V高压
	int m_nLowTension12;				//12V低压
	int m_nHighTension12;				//12V高压
	int m_nLowTension28;				//28V低压
	int m_nHighTension28;				//28V高压
	int m_nNacelleComm;					//吊舱通信
	int m_nWaterTemp;					//水温高标识

	//int joystick;
};

struct ZHZ_ServoLinkInfo
{
	ZHZ_ServoLinkInfo()
	{
		m_nServolinkNOK = 0;
		m_nServolinkLost = 0;
		m_nWatchdogReset = 0;
		m_nSoftwareReset = 0;
		m_nPoweronReset = 0;
		m_nAp2servolinkNOK = 0;
		m_nAp2servoChkErr = 0;
		m_nWatchdogResetFlag = 0;
		m_nSoftwareResetFlag = 0;
	}

	int m_nServolinkNOK;
	int m_nServolinkLost;
	int m_nWatchdogReset;
	int m_nSoftwareReset;
	int m_nPoweronReset;
	int m_nAp2servolinkNOK;
	int m_nAp2servoChkErr;
	int m_nWatchdogResetFlag;
	int m_nSoftwareResetFlag;
};

//遥控器 控制量
struct ZHZ_ControlSignals
{
	ZHZ_ControlSignals()
	{
		m_dA1s = 0.0;
		m_dB1s = 0.0;
		m_dAm = 0.0;
		m_dAt = 0.0;
		m_nTh = 0;
	}

	double m_dA1s;		//横滚 cyclic roll [%], range [-150,+150]
	double m_dB1s;		//俯仰 cyclic pitch [%], range [-150,+150]
	double m_dAm;		//总距 collective pitch [%], range [-100,+100]
	double m_dAt;		//尾舵 tail [%], range [-100,+100]
	int m_nTh;			//油门 throttle position [%], range [0,+100](油门 节流阀)
};

enum ZHZ_eCurveType
{
	THROTTLE_CURVETYPE = 0,	//油门曲线
	PITCH_CURVETYPE,		//总距曲线
};

//油门曲线与总距曲线
struct ZHZ_CurvePointData
{
	ZHZ_CurvePointData()
	{
		m_eCurveType = THROTTLE_CURVETYPE;

		for (int i = 0; i < m_nPointCount; i++)
		{
			m_dPointData[i] = 0.0;
		}
	}

	static const int m_nPointCount = 20;
	ZHZ_eCurveType m_eCurveType;
	double m_dPointData[m_nPointCount];
};


struct TD220TelemetryData :public DataDefineInterface
{
	ZHZ_AbsPosition UAVAbsPosition;	//无人机绝对位置
	ZHZ_RelPosition UAVRelPosition;	//无人机相对位置
	ZHZ_Velocity UAVVelocity;		//速度
	ZHZ_AIRCRAFT UAVAircraft;		//无人机设备参数
	ZHZ_ModeInfo UAVModeInfo;		//无人机模式信息
	ZHZ_ServoLinkInfo ServoLinkInfo;//无人机舵机信息
	ZHZ_WaypointInfo WayPointInfo;	//航点信息
	ZHZ_Attitude UAVAttitude;		//无人机姿态
	ZHZ_ControlSignals ControlSignals;//遥控器手柄数据
	ZHZ_ServoPosition ServoPostion;	//舵机位置
	ZHZ_ADValue UAVADValue;			//无人机外置传感器参数
	ZHZ_References UAVRelVelocity;	//无人机参考速度
};