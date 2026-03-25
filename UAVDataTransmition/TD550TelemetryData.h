/*
	TD550便携控制软件遥测数据
*/
#ifndef TD550TELEMETRYDATA_H
#define TD550TELEMETRYDATA_H
#include "DataDefineInterface.h"

typedef struct FirstsubOneframe
{
	FirstsubOneframe()
	{
		SubCounts = 0;
		FCVersion = 0;
		CalibAirSpeed = 0;
		TureAirSpeed = 0;
		AirPressHeight = 0;
		StaticTemp = 0;
		TotalTemp = 0;
		SmoothAirSpeed = 0;
		AvoidNorthPos1 = 0;
		AvoidEastPos1 = 0;
		AvoidEastPos1 = 0;
	}
	double				SubCounts;//小帧计数器
	unsigned short      FCVersion;//飞控版本号
	double              CalibAirSpeed;//校准空速
	double              TureAirSpeed;//真空速
	double              AirPressHeight;//气压高
	double              StaticTemp;//大气静温
	double              TotalTemp;//大气总温
	double              SmoothAirSpeed;//校准空速（平滑处理）
	double				AvoidNorthPos1;//障碍物---1北向相对位置x
	double				AvoidEastPos1;//障碍物---1北向相对位置y
	double				AvoidNorthPos2;//障碍物---2北向相对位置x
	double				AvoidEastPos2;//障碍物---2北向相对位置y
	double				AvoidNorthPos3;//障碍物---3北向相对位置x
	unsigned char		NaviState;		//惯导状态字
}*pFirstsubOneframe;

typedef struct FirstsubTwoframe
{
	FirstsubTwoframe()
	{
		D1ServoControl = 0;
		D2ServoControl = 0;
		D3ServoControl = 0;
		U1ServoControl = 0;
		U2ServoControl = 0;
		U3ServoControl = 0;
		DamperServoControl = 0;
		DirServoControl = 0;
		D1ServoPostion = 0;
		D2ServoPostion = 0;
		D3ServoPostion = 0;
		U1ServoPostion = 0;
		U2ServoPostion = 0;
		U3ServoPostion = 0;
		lat = 0;
		lon = 0;
	}
	double              D1ServoControl;//D1舵机位置控制律指令
	double              D2ServoControl;//D2舵机位置控制律指令
	double              D3ServoControl;//D3舵机位置控制律指令
	double              U1ServoControl;//U1舵机位置控制律指令
	double              U2ServoControl;//U2舵机位置控制律指令
	double              U3ServoControl;//U3舵机位置控制律指令
	double              DamperServoControl;//风门舵机位置控制律指令
	double              DirServoControl;//方向舵机位置控制律指令
	double              D1ServoPostion;//D1舵机位置反馈
	double              D2ServoPostion;//D2舵机位置反馈
	double              D3ServoPostion;//D3舵机位置反馈
	double              U1ServoPostion;//u1舵机位置反馈
	double              U2ServoPostion;//u2舵机位置反馈
	double              U3ServoPostion;//u3舵机位置反馈
	//double              DamperServoPostion;//风门舵机位置反馈
	//double              DirServoPostion;//方向舵机位置反馈
	double				QXYZJJJJ;//前旋翼总距桨距角
	double				QXYFYJJJ;//前旋翼俯仰桨距角
	double              lat;//纬度
	double              lon;//经度
}*pFirstsubTwoframe;

typedef struct FirstsubThreeframe
{
	FirstsubThreeframe()
	{
		AbsolutelyHeight = 0;
		XSpeed = 0;
		YSpeed = 0;
		ZSpeed = 0;
		XaSpeed = 0;
		YaSpeed = 0;
		ZaSpeed = 0;
		RollAngVelocity = 0;
		PitchAngVelocity = 0;
		YawAngVelocity = 0;
		Yaw = 0;
		Pitch = 0;
		Roll = 0;
		EastSpeed = 0;
		NothSpeed = 0;
		RelHeight = 0;
		AYB = 0;
		EngineR = 0;
		MainRotor = 0;
	}
	double              AbsolutelyHeight;//绝对高度
	double              XSpeed;//纵向地速;
	double              YSpeed;//侧向地速;
	double              ZSpeed;//地向地速;
	double              XaSpeed;//纵向加速度;
	double              YaSpeed;//侧向加速度;
	double              ZaSpeed;//地向加速度;
	double              RollAngVelocity;//滚转角速率
	double              PitchAngVelocity;//俯仰角速率
	double              YawAngVelocity;//偏航角速率
	double              Yaw;//偏航
	double              Pitch;//俯仰
	double              Roll;//滚转
	double              EastSpeed;//东向速度
	double              NothSpeed;//北向速度
	double              RelHeight;//相对高度
	double              AYB;//侧向综合加速度
	double              EngineR;//ECUA发动机转速
	double              MainRotor;//旋翼转速
}*pFirstsubThreeframe;

typedef struct FirstsubFourframe
{
	FirstsubFourframe()
	{
		SignalSource1 = 0;
		SignalSource2 = 0;
		SignalSource3 = 0;
		SignalSource4 = 0;
		SignalSource5 = 0;
		FaultCode = 0;
		D1ServoMFault = 0;
		D1ServoBFault = 0;
		D2ServoMFault = 0;
		D2ServoBFault = 0;
		D3ServoMFault = 0;
		D3ServoBFault = 0;
		U1ServoMFault = 0;
		U1ServoBFault = 0;
		U2ServoMFault = 0;
		U2ServoBFault = 0;
		U3ServoMFault = 0;
		U3ServoBFault = 0;
		DirServoMFault = 0;
		DirServoBFault = 0;
		D1ServoCurrent = 0;
		D2ServoCurrent = 0;
		D3ServoCurrent = 0;
		U1ServoCurrent = 0;
		U2ServoCurrent = 0;
		U3ServoCurrent = 0;
		AvoidEastPos3 = 0;
	}
	unsigned char       SignalSource1;//信号源选择字
	unsigned char       SignalSource2;//信号源选择字
	unsigned char       SignalSource3;//信号源选择字
	unsigned char       SignalSource4;//信号源选择字
	unsigned char       SignalSource5;//信号源选择字
	unsigned int        FaultCode;//故障综合字
	unsigned char       D1ServoMFault;//D1舵机主余度状态
	unsigned char       D1ServoBFault;//D1舵机备余度状态
	unsigned char       D2ServoMFault;//D2舵机主余度状态
	unsigned char       D2ServoBFault;//D2舵机备余度状态
	unsigned char       D3ServoMFault;//D3舵机主余度状态
	unsigned char       D3ServoBFault;//D3舵机备余度状态
	unsigned char       U1ServoMFault;//U1舵机主余度状态
	unsigned char       U1ServoBFault;//U1舵机备余度状态
	unsigned char       U2ServoMFault;//U2舵机主余度状态
	unsigned char       U2ServoBFault;//U2舵机备余度状态
	unsigned char       U3ServoMFault;//U3舵机主余度状态
	unsigned char       U3ServoBFault;//U3舵机备余度状态
	//unsigned char       DamperServoMFault;//风门舵机主余度状态
	//unsigned char       DamperServoBFault;//风门舵机备余度状态
	double				QXYHGJJJ;//前旋翼横滚桨距角;
	unsigned char       DirServoMFault;//方向舵机主余度状态
	unsigned char       DirServoBFault;//方向舵机备余度状态
	double              D1ServoCurrent;//D1舵机电流
	double              D2ServoCurrent;//D2舵机电流
	double              D3ServoCurrent;//D3舵机电流
	double              U1ServoCurrent;//u1舵机电流
	double              U2ServoCurrent;//u2舵机电流
	double              U3ServoCurrent;//u3舵机电流
	//double              DamperServoCurrent;//风门舵机电流
	//double              DirServoCurrent;//方向舵机电流
	double				HXYFYJJJ;//后旋翼俯仰桨距角;
	double				HXYHGJJJ;//后旋翼横滚桨距角;
	double				AvoidEastPos3;//障碍物---3北向相对位置y
}*pFirstsubFourframe;

typedef struct SecondsubOneframe
{
	SecondsubOneframe()
	{
		InertialState = 0;
		BDPdop = 0;
		MEMSNum = 0;
		UTCTime = 0;
		FlightState = 0;
		RouteNum = 0;
		WayPointNum = 0;
		WayPointState = 0;
		NextPointNum = 0;
		NextPointState = 0;
		HeadingControl = 0;
		TrackError = 0;
		SetoverDis = 0;
		MainNavNum = 0;
		FlushingDis = 0;
		FlushingTime = 0;
	}
	unsigned char       InertialState;//闭环光纤惯导状态字
	double              BDPdop;//卫星PDOP值
	int                 MEMSNum;//导航卫星颗数（MEMS）
	double              UTCTime;//UTC时间
	unsigned char       FlightState;//飞行阶段状态字
	int                 RouteNum;//当前航线编号
	int                 WayPointNum;//当前航点编号
	unsigned char       WayPointState;//当前航点特征字
	int                 NextPointNum;//下一个航点编号
	unsigned char       NextPointState;//下一个航点特征字
	double              HeadingControl;//航向导引信号
	double              TrackError;//航迹误差角
	int                 SetoverDis;//侧偏距信号
	int                 MainNavNum;//主惯导卫星颗数
	int                 FlushingDis;//待飞距信号
	int                 FlushingTime;//待飞时间
}*pSecondsubOneframe;

typedef struct SecondsubTwoframe
{
	SecondsubTwoframe()
	{
		GSB = 0;
		AvoidTimeIndex = 0;
		AvoidTaskState = 0;
		AvoidNorthSpeed = 0;
		AvoidEastSpeed = 0;
		AcoidFCState = 0;
		AvoidFCOrder = 0;
		ctc = 0;
		b1c = 0;
		a1c = 0;
		dtc = 0;
		RouteSpeed = 0;
		RouteVPostion = 0;
		RouteHPostion = 0;
		RouteYPostion = 0;
		RouteHeight = 0;
		RouteVSpeed = 0;
		RouteSideSpeed = 0;
		NaviState = 0;
	}
	double              GSB;//预制倾斜角GSB
	unsigned short		AvoidTimeIndex;//障碍机时间戳
	unsigned char		AvoidTaskState;//障碍机任务执行状态
	double				AvoidNorthSpeed;//障碍机北向速度参考
	double				AvoidEastSpeed;//障碍机东向速度参考
	unsigned char		AcoidFCState;//飞控避障状态
	unsigned char		AvoidFCOrder;//飞控避障命令
	double              ctc;//总距浆距角
	double              b1c;//俯仰浆距角
	double              a1c;//横滚浆距角
	double              dtc;//航向浆距角
	//double              chk;//风门舵机开度
	double				HZJJJJ;//后总距桨距角
	double              RouteSpeed;//纵向速度给定基准值
	double              RouteVPostion;//纵向位置给定基准值
	double              RouteHPostion;//横向位置给定基准值
	double              RouteYPostion;//航向角给定基准值
	double              RouteHeight;//高度给定基准值
	double              RouteVSpeed;//垂直速度给定基准值
	double				RouteSideSpeed;//侧向速度给定基准值
	unsigned char		NaviState;//卫导接收机状态字
}*pSecondsubTwoframe;

typedef struct SecondsubThreeframe
{
	SecondsubThreeframe()
	{
		FCCPower = 0;
		FCCState = 0;
		FCCTemp = 0;
		ServoTemp = 0;
		FCCError = 0;
		FCCLevel = 0;
		CollectionBoxState = 0;
		ControlMode = 0;
		ModeUsed = 0;
		ManeuMode = 0;
		EngineMode = 0;
		Startup = 0;
		Closedown = 0;
		PSI_DELTA = 0;
		DIS_XY = 0;
		DIS_X = 0;
	}
	double              FCCPower;//飞控计算机28V通道电源
	unsigned short      FCCState;//飞控计算机工作状态字
	double              FCCTemp;//飞控计算机温度
	int                 ServoTemp;//伺服控制器温度
	unsigned short      FCCError;//飞控计算机故障编码
	unsigned char       FCCLevel;//飞控计算机状态等级
	//zrm注释掉新增前发动机故障码
	unsigned char       ErrorID_qian;//前发动机故障序号
	unsigned short      ErrorNumber_qian;//前发动机故障码
	unsigned char       ErrorID_hou;//后发动机故障序号
	unsigned short      ErrorNumber_hou;//后发动机故障码

	unsigned short      CollectionBoxState;//采集盒工作状态（145专用）
	unsigned char       ForceLandPointIndex;//迫降点序号
	unsigned char       ControlMode;//控制模式状态字
	unsigned short      ModeUsed;//模态投入状态字
	unsigned short      ManeuMode;//机动功能模态状态字
	unsigned char       EngineMode;//发动机状态字
	unsigned char       Startup;//一键启动状态字
	unsigned char       Closedown;//一键关闭状态字
	double			    Reserved;//备用状态字->起动电池电压
	double              PSI_DELTA;//航向偏差
	double              DIS_XY;//飞机当前位置与起飞点之间距离
	double              DIS_X;//当前位置与起飞点位置的X轴偏差
}*pSecondsubThreeframe;

typedef struct SecondsubFourframe
{
	SecondsubFourframe()
	{
		DIS_Y = 0;
		takeoffHeight = 0;
		takeoffWeight = 0;
		NowWeight = 0;
		AvoidPosX = 0;
		AvoidPosY = 0;
		EquipmentState = 0;
		EquipmentAlarm1 = 0;
		EquipmentAlarm2 = 0;
		EquipmentAlarm3 = 0;
		EquipmentAlarm4 = 0;
		FlightTime = 0;
		FlightSurTime = 0;
		WaterFan = 0;
		MiddleFan = 0;
		MidFanCurrent = 0;
		WaterFacCurrent = 0;
		GeneratorCurrent = 0;
		RadioHeight = 0;
		RadioSmoothAlt = 0;
		Power12V = 0;
		Power24V = 0;
	}
	double              DIS_Y;//当前位置与起飞点位置的Y轴偏差
	double              takeoffHeight;//起飞机场场高
	double              takeoffWeight;//飞机起飞重量
	double              NowWeight;//飞机实时重量
	short				AvoidPosX;//当前位置与避障起点偏差X
	short				AvoidPosY;//当前位置与避障起点偏差Y
	unsigned short      EquipmentState;//机载设备状态字
	unsigned char       EquipmentAlarm1;//机载设备告警字1
	unsigned char       EquipmentAlarm2;//机载设备告警字2
	unsigned char       EquipmentAlarm3;//机载设备告警字3
	unsigned char       EquipmentAlarm4;//机载设备告警字4
	short               FlightTime;//飞行工作时间
	short               FlightSurTime;//剩余航行时间
	unsigned char       WaterFan;//发动机水散热风扇状态
	unsigned char       MiddleFan;//发动机中冷风扇状态
	//double              RadarHeight;//毫米波雷达高度
	//double              LandHeight;//毫米波雷达近地速度
	double				MidFanCurrent;//中冷风扇供电电流
	double				WaterFacCurrent;//水冷风扇供电电流
	double				GeneratorCurrentBack;//电源管理盒7.4V电压
	double				GeneratorCurrent;//前发电机供电电流
	double              RadioHeight;//无线电高度
	double              RadioSmoothAlt;//无线电高度（平滑滤波后）
	double				Power74V;//电源管理盒7.4V电压
	double              Power12V;//电源管理盒12V电压
	double              Power24V;//电源管理盒28V电压
}*pSecondsubFourframe;

typedef struct ThirdsubOneframe
{
	ThirdsubOneframe()
	{
		DisperseIn1 = 0;
		DisperseOut2 = 0;
		DisperseOutRe2 = 0;
		Battery = 0;
		OpenK2 = 0;
		OilPressure = 0;
		OilVolume = 0;
		OpenK4 = 0;
		RetarderPressure = 0;
		RetarderTemp = 0;
		OpenK1 = 0;
		Tension = 0;
	}
	unsigned char       DisperseIn1;//外部离散量输入字1
	//unsigned char       DisperseOut1;//离散输出回绕字1
	unsigned short      DisperseOut2;//离散输出指令2
	unsigned short      DisperseOutRe2;//离散输出回绕字2
	unsigned char       Battery;//蓄电池状态
	double              OpenK2;//上限位开关K2电压值(弃用)->前旋翼转速
	double              OilPressure;//燃油压力电压值（弃用）->前发燃油压力;
	double              OilPressure2;//燃油压力电压值2（弃用）->后发燃油压力;
	double              OilVolume;//燃油油量电压值（弃用）->下油量电压;
//	double              OilVolume2;//燃油油量电压值（弃用）->下油量电压;
//	double				OilRemianing;//剩余燃油;
//	double				RetarderTemperature;//前减速器滑油温度;
//	double				RetarderTemperature2;//后减速器滑油温度;

//	double				ThrottleOpening;//前发油门开度;
//	double				ThrottleOpening2;//后发油门开度;
	double              OpenK4;//右到位开关K4电压值
	double              RetarderPressure;//减速器滑油压力电压值
	double              RetarderTemp;//减速器滑油温度电阻值
	double              OpenK1;//电源张紧盒状态（弃用）->离合状态;
	double              Tension;//张紧状态指示电压值（弃用）->离合反馈电压;
}*pThirdsubOneframe;

typedef struct ThirdsubTwoframe
{
	ThirdsubTwoframe()
	{
		OpenK3 = 0;
		Motortemp2 = 0;
		ROiltemp = 0;
		ROilPress = 0;
		Oil = 0;
		EngineRPM145 = 0;
		ThrottlePos145 = 0;
		EngineOilPre145 = 0;
		EngineOilTemp145 = 0;
		ExhaustTemp1 = 0;
		ExhaustTemp2 = 0;
		ExhaustTemp3 = 0;
		ExhaustTemp4 = 0;
		IntakePressure = 0;
		TurboPressure = 0;
		CoolantTemp = 0;
		EngineOilPre = 0;
	}
	double              OpenK3;//左限位开关K3电压值
	double              Motortemp1;//前空燃比
	double              Motortemp2;//后空燃比


	double              ROiltemp;//减速器滑油温度1
	double              ROilPress;//减速器滑油温度2
	double              OilPress;//燃油油量电压值2(弃用)->上油量电压;
	double              Oil;//燃油余量
	double				EngineRPM145;//发动机转速（145专用）
	double				ThrottlePos145;//前发电子节气门开度
	double				EngineOilPre145;//前发油轨压力
	double				EngineOilTemp145;//后发油轨压力
	double              ExhaustTemp1;//排气温度1（弃用）//->前发动机扭矩;
	double              ExhaustTemp2;//排气温度2（弃用）//->后发动机扭矩;
	double              ExhaustTemp3;//前中冷后温度（弃用）//->前发增压温度;
	double              ExhaustTemp4;//后中冷后温度（弃用）//->后发增压温度;
	double              IntakePressure;//前发动机进气压力
	double              TurboPressure;//前发歧管压力
	double              CoolantTemp;//后发歧管压力
	double              EngineOilPre;//前发涡轮旁通开度
}*pThirdsubTwoframe;

typedef struct ThirdsubThreeframe
{
	ThirdsubThreeframe()
	{
		EngineOilTemp = 0;
		DamperPostion = 0;
		InairTemp = 0;
		InairPre = 0;
		OutAirPostion = 0;
		EngineRAM = 0;
		EnineInAirTemp = 0;
		EcuAbus = 0;
		EcuBbus = 0;
		EngineTime = 0;
		OilPreDiff = 0;
		StableBoxPre = 0;
		StableBoxGoalPre = 0;
		StableBoxTemp = 0;
		CoolingTempMax = 0;
		CoolingTempMin = 0;
		ExhaustTempmax = 0;
		ExhaustTempmin = 0;
	}
	double              EngineOilTemp;//后发涡轮旁通开度
	double              DamperPostion;//后电子节气门开度
	double              InairTemp;//前发歧管温度
	double              InairPre;//后发歧管温度
	//double              OilUsed;//燃油流量
	unsigned char		FrontEngineIdleMarking;//前发怠速标识;
	unsigned char		BehindEngineIdleMarking;//后发怠速标识;		
	double              OutAirPostion;//前发瞬时油耗
	double              EngineRAM;//ECUB发动机转速(后发)
	double              EnineInAirTemp;//后发瞬时油耗
	double              EcuAbus;//ECUA-BUS电压
	double              EcuBbus;//ECUB-BUS电压
	int                 EngineTime;//发动机运行时间
	double				OilPreDiff;//燃油压力差（145专用）弃用->前电子风扇PWM;
	double				StableBoxPre;//稳压箱压力（145专用）弃用->后电子风扇PWM;
	double				StableBoxGoalPre;//稳压箱目标压力（145专用）
	double				StableBoxTemp;//稳压箱温度（145专用）
	double				CoolingTempMax;//前发水温
	double				CoolingTempMin;//后发水温
	double				ExhaustTempmax;//排温最高值（145专用）
	double				ExhaustTempmin;//排温最低值（145专用）
}*pThirdsubThreeframe;

typedef struct ThirdsubFourframe
{
	ThirdsubFourframe()
	{
		KgReply = 0;
		YtReply = 0;
		YtHdReply = 0;
		B2YReply = 0;
		B2XReply = 0;
		B1YReply = 0;
		B1XReply = 0;
		DamperReply = 0;
		byte11Reply = 0;
		byte12Reply = 0;
		byte13Reply = 0;
		byte14Reply = 0;
		byte15Reply = 0;
		byte16Reply = 0;
		byte17Reply = 0;
		byte18Reply = 0;
		byte19Reply = 0;
		byte20Reply = 0;
		byte21Reply = 0;
		byte22Reply = 0;
		byte23Reply = 0;
		byte24Reply = 0;
		byte25Reply = 0;
		byte26Reply = 0;
		byte27Reply = 0;
		LinkTest = 0;
		EngineAirPre = 0;
		TCUPower = 0;
		ServoPos = 0;
		ServoGoalPos = 0;
		B2YControl = 0;
		B2XControl = 0;
		B1YControl = 0;
		B1XControl = 0;
		FUTABAState = 0;
	}
	unsigned short      KgReply;//飞行开关指令应答
	unsigned char       YtReply;//飞行组合指令代码回报
	unsigned char       YtHdReply;//飞行组合指令代码应答
	double              B2YReply;//纵向周期变距遥控指令回报（副控盒）
	double              B2XReply;//横向周期变距遥控指令回报（副控盒）
	double              B1YReply;//总距遥控指令回报（副控盒）
	double              B1XReply;//航向遥控指令回报（副控盒）
	double              DamperReply;//风门遥控指令回报（副控盒）
	unsigned char       byte11Reply;//组合指令内容回报-原帧11字节
	unsigned char       byte12Reply;//组合指令内容回报-原帧12字节
	unsigned char       byte13Reply;//组合指令内容回报-原帧13字节
	unsigned char       byte14Reply;//组合指令内容回报-原帧14字节
	unsigned char       byte15Reply;//组合指令内容回报-原帧15字节
	unsigned char       byte16Reply;//组合指令内容回报-原帧16字节
	unsigned char       byte17Reply;//组合指令内容回报-原帧17字节
	unsigned char       byte18Reply;//组合指令内容回报-原帧18字节
	unsigned char       byte19Reply;//组合指令内容回报-原帧19字节
	unsigned char       byte20Reply;//组合指令内容回报-原帧20字节
	unsigned char       byte21Reply;//组合指令内容回报-原帧21字节
	unsigned char       byte22Reply;//组合指令内容回报-原帧22字节
	unsigned char       byte23Reply;//组合指令内容回报-原帧23字节
	unsigned char       byte24Reply;//组合指令内容回报-原帧24字节
	unsigned char       byte25Reply;//组合指令内容回报-原帧25字节
	unsigned char       byte26Reply;//组合指令内容回报-原帧26字节
	unsigned char       byte27Reply;//组合指令内容回报-原帧27字节
	unsigned char       LinkTest;//遥控遥测链路测试计数

	double				EngineAirPre;//后发动机进气压力（145专用）
	double				TCUPower;//TCU供电电压（145专用）
	double				ServoPos;//伺服电机位置（145专用）
	double				ServoGoalPos;//伺服电机目标位置（145专用）
	double				NaviPDOP;	//惯导PDOP值
	double              B2YControl;//纵向遥控指令回报（FUTABA)
	double              B2XControl;//横向遥控指令回报（FUTABA)
	double              B1YControl;//总距遥控指令回报（FUTABA)
	double              B1XControl;//航向遥控指令回报（FUTABA)
	unsigned char       FUTABAState;//FUTABA状态字
}*pThirdsubFourframe;

struct TD550TelemetryData :public DataDefineInterface
{
	FirstsubOneframe m_FirstSubOneFrame;				//第一副帧第一小帧
	FirstsubTwoframe m_FirstSubTwoFrame;				//第一副帧第二小帧
	FirstsubThreeframe m_FirstSubThreeFrame;			//第一副帧第三小帧
	FirstsubFourframe m_FirstSubFourFrame;				//第一副帧第四小帧
	SecondsubOneframe m_SecondSubOneFrame;				//第二副帧第一小帧
	SecondsubTwoframe m_SecondSubTwoFrame;				//第二副帧第二小帧
	SecondsubThreeframe m_SecondSubThreeFrame;			//第二副帧第三小帧
	SecondsubFourframe m_SecondSubFourFrame;			//第二副帧第四小帧
	ThirdsubOneframe m_ThirdSubOneFrame;				//第三副帧第一小帧
	ThirdsubTwoframe m_ThirdSubTwoFrame;				//第三副帧第二小帧
	ThirdsubThreeframe m_ThirdSubThreeFrame;			//第三副帧第三小帧
	ThirdsubFourframe m_ThirdSubFourFrame;				//第三副帧第四小帧
};
#endif // TD550TELEMETRYDATA_H