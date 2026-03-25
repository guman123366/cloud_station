/*
	TD220控制车的遥控遥测复合帧数据
*/

#include "DataDefineInterface.h"

struct TD220TelemetryDataCar :public DataDefineInterface
{
	TD220TelemetryDataCar()
	{
		UAVID1 = 0;
		Status1 = 0;
		GuidanceMod = 0;
		BaseVal = 0;
		PositionN = 0;
		PositionE = 0;
		GPSHeight = 0;
		Status2 = 0;
		ServoVal = 0;
		ReBackCMD = 0;
		ReBackState = 0;
		CPUtemp = 0;
		Yaw = 0;
		roll = 0;
		pitch = 0;
		PositionD = 0;
		Airspeed = 0;

		UAVID2 = 0;
		SplitCode1 = 0;
		RefSpeedN = 0;
		RefSpeedE = 0;
		RefSpeedD = 0;
		RefHeading_rate = 0;
		RefPositionN = 0;
		RefPositionE = 0;
		RefPositionD = 0;
		ErrorZCode = 0;
		ErrorFCode = 0;
		ErrorSplitCode = 0;
		GeoCourse = 0;
		RefPositionYaw = 0;
		AirSpeedtemp = 0;
		AirSpeedHeight = 0;
		PositionD1 = 0;
		Throttle = 0;
		MotorTemp = 0;
		lat = 0;
		lon = 0;
		WaypointNum = 0;
		YawAcc = 0;
		pitchAcc = 0;
		rollAcc = 0;
		Waypotion = 0;
		Pdop = 0;

		UAVID3 = 0;
		guidanceXAcc = 0;
		guidanceYAcc = 0;
		guidanceZAcc = 0;
		U1ServoPos = 0;
		U2ServoPos = 0;
		U3ServoPos=0;//舵机u3位置反馈
		D1ServoPos=0;//舵机d1位置反馈
		D2ServoPos=0;//舵机d2位置反馈
		Fan_rpm1=0;//风扇1转速
		Fan_rpm2=0;//风扇2转速
		RPM=0;
		D3ServoPos=0;//舵机d3位置反馈
		guidanceSpeedD=0;//惯导地向速度
		RecvOil=0;//余油
		WaterTemp=0;//水温
		AfterGeo=0;//标校后地磁
		U0Con=0;//滚转控制
		U1Con=0;//俯仰控制
		U2Con=0;//总距控制
		U3Con=0;//航向控制
		//第四副帧
		UAVID4=0;//飞机号
		SplitCode2=0;//经纬度分组码
		SplitCode3=0;//经纬度分组码
		SpeedN=0;//北向速度
		SpeedE=0;//东向速度
		SpeedD=0;//地向速度
		U1Current=0;//u1舵机电流
		U2Current=0;//u2舵机电流
		U3Current=0;//u3舵机电流
		D1Current=0;//d1舵机电流
		D2Current=0;//d2舵机电流
		D3Current=0;//d3舵机电流
		BDSatNum=0;//BD星数
		GPSsatNum=0;//GPS星数
		DiffState=0;//差分状态
		InertialState=0;//惯导状态
		XAcc=0;//x加速度
		YAcc=0;//y加速度
		ZAcc=0;//z加速度
		GPSSpeed=0;//GPS速度
		WaypointDis=0;//航点距离
		PlaneType=0;//飞机类型
		PlaneNum=0;//飞机编号
		//////////////////////////////////////////////////北斗日期
		BDstate=0;//北斗状态
		B1SYear=0;//b1起始年月日
		B1SMonth=0;//
		B1SDay=0;//
		B1EYear=0;//b1结束年月日
		B1EMonth=0;//
		B1EDay=0;//
		B3SYear=0;//b3起始年月日
		B3SMonth=0;//
		B3SDay=0;//
		B3EYear=0;//b3结束年月日
		B3EMonth=0;//
		B3EDay=0;//
		ICYear=0;//IC结束年月日
		ICMonth=0;//
		ICDay=0;//
		////////////////////////////////////////////地磁标校
		Geo0=0;
		Geo45=0;
		Geo90=0;
		Geo135=0;
		Geo180=0;
		Geo225=0;
		Geo270=0;
		Geo315=0;
		////////////////////////////////////////////基准站数据
		StationLat=0;	//基准站纬度
		StationLon=0;	//基准站经度
		StationAlt=0;	//基准站高度
		StationState=0;	//基准站定位状态
		StationStarNum=0;//基准站卫星颗数

		memset(PodData, 0, 32);
	}

	//第一副帧
	unsigned char		UAVID1;//飞机号
	unsigned int		Status1;//监测字1
	unsigned char		GuidanceMod;//制导状态
	unsigned short		BaseVal;//总电电压
	unsigned int		PositionN;//位置北
	unsigned int		PositionE;//位置东
	unsigned int		GPSHeight;//GPS高度
	unsigned int		Status2;//监测字2
	unsigned char       ServoVal;//油门舵机电压
	unsigned char       ReBackCMD;//回报指令
	unsigned char       ReBackState;//回报状态
	unsigned short      CPUtemp;//CPU温度
	unsigned short      Yaw;//航向角
	unsigned short		roll;// 横滚角 [-pi, +pi]
	unsigned short		pitch;// 俯仰角 [-pi, +pi]
	unsigned short		PositionD;//位置下
	unsigned short      Airspeed;//真空速
	//第二副帧
	unsigned char		UAVID2;//飞机号
	unsigned char		SplitCode1;//经纬度分组码
	unsigned short      RefSpeedN;//北向速度参考
	unsigned short      RefSpeedE;//东向速度参考
	unsigned short      RefSpeedD;//地向速度参考
	unsigned short      RefHeading_rate;//航向速度参考
	unsigned int        RefPositionN;//北向位置参考
	unsigned int        RefPositionE;//东向位置参考
	unsigned int        RefPositionD;//地向位置参考
	unsigned int		ErrorZCode;//故障码0	
	unsigned int		ErrorFCode;//故障码1
	unsigned char       ErrorSplitCode;//故障码分组码
	unsigned short      GeoCourse;//地磁航向
	unsigned short      RefPositionYaw;//航向参考
	unsigned short      AirSpeedtemp;//空速管总温
	unsigned int        AirSpeedHeight;//空速管高度
	unsigned short		PositionD1;
	unsigned char       Throttle;//油门
	unsigned short      MotorTemp;//缸温
	int                 lat;//纬度
	int                 lon;//经度
	unsigned char       WaypointNum;//航点号
	unsigned short      YawAcc;//航向角速度
	unsigned short      pitchAcc;//俯仰角速度
	unsigned short      rollAcc;//横滚角速度
	unsigned char       Waypotion;
	unsigned short      Pdop;//回传的PDOP值
	//第三副帧
	unsigned char		UAVID3;//飞机号
	unsigned short      guidanceXAcc;//惯导X向加速度
	unsigned short      guidanceYAcc;//惯导Y向加速度
	unsigned short      guidanceZAcc;//惯导Z向加速度
	unsigned short      U1ServoPos;//舵机u1位置反馈
	unsigned short      U2ServoPos;//舵机u2位置反馈
	unsigned short      U3ServoPos;//舵机u3位置反馈
	unsigned short      D1ServoPos;//舵机d1位置反馈
	unsigned short      D2ServoPos;//舵机d2位置反馈
	unsigned int        Fan_rpm1;//风扇1转速
	unsigned int        Fan_rpm2;//风扇2转速
	unsigned short      RPM;
	unsigned short      D3ServoPos;//舵机d3位置反馈
	unsigned short      guidanceSpeedD;//惯导地向速度
	unsigned short      RecvOil;//余油
	unsigned short      WaterTemp;//水温
	unsigned short      AfterGeo;//标校后地磁
	unsigned short      U0Con;//滚转控制
	unsigned short      U1Con;//俯仰控制
	unsigned short      U2Con;//总距控制
	unsigned short      U3Con;//航向控制
	//第四副帧
	unsigned char		UAVID4;//飞机号
	unsigned char		SplitCode2;//经纬度分组码
	unsigned char		SplitCode3;//经纬度分组码
	unsigned short      SpeedN;//北向速度
	unsigned short      SpeedE;//东向速度
	unsigned short      SpeedD;//地向速度
	unsigned short      U1Current;//u1舵机电流
	unsigned short      U2Current;//u2舵机电流
	unsigned short      U3Current;//u3舵机电流
	unsigned short      D1Current;//d1舵机电流
	unsigned short      D2Current;//d2舵机电流
	unsigned short      D3Current;//d3舵机电流
	unsigned char		BDSatNum;//BD星数
	unsigned char		GPSsatNum;//GPS星数
	unsigned char		DiffState;//差分状态
	unsigned char		InertialState;//惯导状态
	unsigned short      XAcc;//x加速度
	unsigned short      YAcc;//y加速度
	unsigned short      ZAcc;//z加速度
	unsigned short      GPSSpeed;//GPS速度
	unsigned int        WaypointDis;//航点距离
	unsigned char       PlaneType;//飞机类型
	unsigned char       PlaneNum;//飞机编号
	//////////////////////////////////////////////////北斗日期
	unsigned char                BDstate;//北斗状态
	unsigned char                B1SYear;//b1起始年月日
	unsigned char                B1SMonth;//
	unsigned char                B1SDay;//
	unsigned char                B1EYear;//b1结束年月日
	unsigned char                B1EMonth;//
	unsigned char                B1EDay;//
	unsigned char                B3SYear;//b3起始年月日
	unsigned char                B3SMonth;//
	unsigned char                B3SDay;//
	unsigned char                B3EYear;//b3结束年月日
	unsigned char                B3EMonth;//
	unsigned char                B3EDay;//
	unsigned char                ICYear;//IC结束年月日
	unsigned char                ICMonth;//
	unsigned char                ICDay;//
	////////////////////////////////////////////地磁标校
	unsigned int        Geo0;
	unsigned int        Geo45;
	unsigned int        Geo90;
	unsigned int        Geo135;
	unsigned int        Geo180;
	unsigned int        Geo225;
	unsigned int        Geo270;
	unsigned int        Geo315;
	////////////////////////////////////////////基准站数据
	double						 StationLat;	//基准站纬度
	double						 StationLon;	//基准站经度
	double						 StationAlt;	//基准站高度
	unsigned char				 StationState;	//基准站定位状态
	unsigned char				 StationStarNum;//基准站卫星颗数

	unsigned char PodData[32];					//载荷数据	
};