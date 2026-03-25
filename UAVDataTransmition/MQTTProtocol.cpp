#include "MQTTProtocol.h"
// MQTTProtocol.cpp
#include <QBitArray>
#include<QMessageBox>
MQTTProtocol::MQTTProtocol(QObject* parent)
	: ConstructionProtocolInterface(parent)
{
	initLogConfig();
}

MQTTProtocol::~MQTTProtocol()
{
}

int MQTTProtocol::ConstructionData(unsigned char* sendBuf)
{
	if (!ReceiveUAVData)
		return 0;
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;

	// 构造根JSON对象
	QJsonObject rootObject= createBaseJsonObject("osd");

	//// 1. 基本信息段
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "osd");
	//rootObject.insert("source", "station");
	// 2. 数据段
	QJsonObject dataObj;

	// 2.1 通用信息
	dataObj.insert("order_id", "UAQ200529AQ01-20240529-FAQEGC01");
	dataObj.insert("manufacturer_id", "UATAIR");
	dataObj.insert("uas_id", "UAS-DEFAULT");
	dataObj.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());

	dataObj.insert("uas_model", "T1400");
	dataObj.insert("coordinate", 1); // 1:WGS-84

	// 2.2 位置信息
	dataObj.insert("longitude", QString::number(protocolData->m_FirstSubTwoFrame.lon,'f',7));
	dataObj.insert("latitude", QString::number(protocolData->m_FirstSubTwoFrame.lat, 'f', 7));
	double height = protocolData->m_FirstSubThreeFrame.AbsolutelyHeight - protocolData->m_SecondSubFourFrame.takeoffHeight;
	dataObj.insert("height", QString::number(height,'f', 2));
	dataObj.insert("altitude", QString::number(protocolData->m_FirstSubThreeFrame.AbsolutelyHeight, 'f', 1));//绝对高度
	// 2.3 运动信息
	dataObj.insert("vs", QString::number(protocolData->m_FirstSubThreeFrame.ZSpeed, 'f', 1));
	dataObj.insert("gs", QString::number(protocolData->m_FirstSubThreeFrame.XSpeed, 'f', 1));
	dataObj.insert("heading", QString::number(protocolData->m_FirstSubThreeFrame.Yaw, 'f', 1));
	dataObj.insert("pitch", QString::number(protocolData->m_FirstSubThreeFrame.Pitch, 'f', 1));
	dataObj.insert("roll", QString::number(protocolData->m_FirstSubThreeFrame.Roll, 'f', 1));
	if (!checkOilStandardFile())
	{
		qDebug()<< tr(u8"读取油量标定文件失败！");
		//GCS::coreLog.info("youliangbiaodingwenjianshibai!!!");
		readOilStandardFlag = false;
	}
	else
	{
		readOilStandardFlag = true;
	}
	if (readOilStandardFlag)
	{
		m_xyldy = protocolData->m_ThirdSubOneFrame.OilVolume; //下油量电压;

			//wds-2024/09/23-9-45-14
			//通过电压值进行油量的插值;
		double oil = interpolation(protocolData->m_ThirdSubTwoFrame.OilPress + m_xyldy);
		//qDebug()<< tr(u8"读取油量成功！");
		//qDebug() << "oil" << oil;
		if (oil != -1.0)
		{
			dataObj.insert("fuel", QString::number( oil,'F',2));  // 燃油油量
		}
	}
	// 2.4 能源信息
	dataObj.insert("battery", 0);
	//dataObj.insert("voltage", QString::number(protocolData->m_SecondSubFourFrame.Power24V, 'f', 2));
	dataObj.insert("voltage", (int)protocolData->m_SecondSubFourFrame.Power24V);
	// 2.5 飞行状态
	dataObj.insert("mode", "MANUAL");

	if ((protocolData->m_SecondSubThreeFrame.FCCState & 0x4000) == 0x4000)
	{
		dataObj.insert("flying", 0);//地面状态
	}
	else
	{
		dataObj.insert("flying",1);//地面状态
	}
	
	dataObj.insert("fly_time", protocolData->m_SecondSubFourFrame.FlightTime);
	dataObj.insert("fly_distance", 23);

	// 2.6 返航点信息
	dataObj.insert("home_set", 0);
	dataObj.insert("home_distance", 0);
	QJsonObject homeLocation;
	homeLocation.insert("longitude", 0);
	homeLocation.insert("latitude", 0);
	homeLocation.insert("altitude",0);
	dataObj.insert("home_location", homeLocation);

	// 2.7 设备状态
	QJsonObject stateObj;
	stateObj.insert("safe_enabled", 0);
	stateObj.insert("rtk_enabled", 0);
	stateObj.insert("rtk_connected", 0);
	if ((protocolData->m_FirstSubFourFrame.FaultCode & 0x08000000) == 0x08000000)
	{
		stateObj.insert("rc_connected", 0);//遥控器状态
	}
	else
	{
		stateObj.insert("rc_connected", 1);//遥控器状态
	}
	
	stateObj.insert("obs_enabled", 0);

	stateObj.insert("accel_cal", 0);
	stateObj.insert("gyro_cal", 0);
	stateObj.insert("hor_cal", 0);
	stateObj.insert("mag_cal", 0);

	stateObj.insert("fpv_live", 0);
	stateObj.insert("stream_live", 0);
	dataObj.insert("state", stateObj);
	dataObj.insert("down_link_state", m_linkState);
	rootObject.insert("data", dataObj);
	rootObject.insert("need_reply", 1);

	// 转换为JSON字节数组
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);

	// 拷贝到发送缓冲区
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	
	//qDebug().noquote() << "osd MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);


	return byteArray.size();
}
int MQTTProtocol::constructionMemsData(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	// 构造根JSON对象
	QJsonObject rootObject = createBaseJsonObject("main_ins");;

	//// 1. 基本信息段
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "main_ins");
	//rootObject.insert("source", "station");
	// 2. 数据段
	QJsonObject dataObj;

	// 3.1 主MEMS状态
	dataObj.insert("main_mems_state", toHexString(protocolData->m_SecondSubOneFrame.InertialState, 1));

	   // 3.2 主MEMS故障字
	dataObj.insert("main_mems_error", toHexString(0, 1));

	// 3.3 卫星信息
	dataObj.insert("main_pdop", QString::number( protocolData->m_SecondSubOneFrame.BDPdop,'f', 2));
	//卫星颗数
	dataObj.insert("sat_num", protocolData->m_SecondSubOneFrame.MainNavNum);
	//卫导接收机
	dataObj.insert("BD_positon_station", protocolData->m_SecondSubTwoFrame.NaviState);
	// 3.5 备MEMS状态
	dataObj.insert("backup_mems_state", toHexString(protocolData->m_FirstSubOneFrame.NaviState, 1));
	// 3.7 离散输出回绕2
	dataObj.insert("disperse_outRe2", toHexString(protocolData->m_ThirdSubOneFrame.DisperseOutRe2, 4));
	// 3.8 温度信息
	dataObj.insert("mems_temp", 0.0);

	//int back_mems_error = ((uint32_t)protocolData->m_SecondSubFourFrame.BackupNaviError3 << 24) |
	//	((uint32_t)protocolData->m_SecondSubFourFrame.BackupNaviError2 << 16) |
	//	(protocolData->m_SecondSubThreeFrame.BackupNaviError1);
	dataObj.insert("backup_mems_error", toHexString(0,1));
	// 3.9起飞场 高度信息
	dataObj.insert("takeoff_airport_elevation", QString::number(protocolData->m_SecondSubFourFrame.takeoffHeight,'f',1));
	//备pstd值
	dataObj.insert("backup_pdop", QString::number(protocolData->m_ThirdSubFourFrame.NaviPDOP, 'f', 2));

	dataObj.insert("east_speed", QString::number(protocolData->m_FirstSubThreeFrame.EastSpeed, 'f', 2));  // 0.01分辨率
	dataObj.insert("north_speed", QString::number(protocolData->m_FirstSubThreeFrame.NothSpeed, 'f', 2));
	dataObj.insert("down_speed", QString::number(protocolData->m_FirstSubThreeFrame.ZSpeed, 'f', 2));
	dataObj.insert("airspeed", QString::number(protocolData->m_FirstSubOneFrame.TureAirSpeed, 'f', 5)); // 0.00001分辨率
	dataObj.insert("yaw_rate", QString::number(protocolData->m_FirstSubThreeFrame.YawAngVelocity, 'f', 2));
	dataObj.insert("longitudinal_speed", QString::number(protocolData->m_FirstSubThreeFrame.XSpeed, 'f', 2)); // 0.00001分辨率
	dataObj.insert("lateral_speed", QString::number(protocolData->m_FirstSubThreeFrame.YSpeed, 'f', 2));
	// 原有其他数据段... 
	rootObject.insert("data", dataObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "Complete MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}
int MQTTProtocol::constructionRemotecontrol(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	// 构造根JSON对象
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "remote_control_td550_t1400");
	//rootObject.insert("source", "station");
	 QJsonObject rootObject = createBaseJsonObject("remote_control_td550_t1400");
	// 发动机数据段
	QJsonObject RemotecontrolObj;
	RemotecontrolObj.insert("vertical_remote_control", protocolData->m_ThirdSubFourFrame.B2YControl);  // 纵向遥控指令回报
	RemotecontrolObj.insert("lateral_remote_control", protocolData->m_ThirdSubFourFrame.B2XControl);  // 横向遥控指令回报
	RemotecontrolObj.insert("total_distance_remote_control", protocolData->m_ThirdSubFourFrame.B1YControl);  // 总距遥控指令回报
	RemotecontrolObj.insert("heading_remote_control", protocolData->m_ThirdSubFourFrame.B1XControl);  //航向遥控指令回报
	RemotecontrolObj.insert("FUTABA_state", toHexString(protocolData->m_ThirdSubFourFrame.FUTABAState,1));	//FUTABA状态字
	// 构建完整数据结构
	rootObject.insert("data", RemotecontrolObj);
	rootObject.insert("need_reply", 1);
	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());
	//qDebug().noquote() << "RemotecontrolObj MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}
int MQTTProtocol::constructionPower(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;

	// 构造根JSON对象
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "power_td550_t1400");
	//rootObject.insert("source", "station");
	QJsonObject rootObject = createBaseJsonObject("power_td550_t1400");
	// 电源系统数据段
	QJsonObject powerObj;

	// 1. 电压参数（按不同精度处理）
	powerObj.insert("fc_computer_28V",QString::number( protocolData->m_SecondSubThreeFrame.FCCPower,'f',2));  // 飞控28V
	powerObj.insert("power_management_Box28V", QString::number(protocolData->m_SecondSubFourFrame.Power24V, 'f', 2));  // 电源盒28V
	powerObj.insert("power_management_box12V", QString::number(protocolData->m_SecondSubFourFrame.Power12V, 'f', 2));  // 电源盒12V
// 2. 电流参数
	powerObj.insert("generator_current", QString::number(protocolData->m_SecondSubFourFrame.GeneratorCurrent, 'f', 2));  // 发电机电流
    powerObj.insert("power_management_box7.4V", QString::number(protocolData->m_SecondSubFourFrame.Power74V, 'f', 2));  // 发电机电流

// 构建完整数据结构
	rootObject.insert("data", powerObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "PowerSystem MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}
int MQTTProtocol::constructionDiscrete(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;

	// 构造根JSON对象
	QJsonObject rootObject = createBaseJsonObject("discrete_td550_t1400");
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "discrete_td550_t1400");
	//rootObject.insert("source", "station");
	// 离散状态数据段
	QJsonObject discreteObj;

	// 1. 故障和状态信息
	discreteObj.insert("fault_code", toHexString(protocolData->m_SecondSubThreeFrame.FCCError, 2)); // 故障编码
	discreteObj.insert("fc_computer_status", toHexString(protocolData->m_SecondSubThreeFrame.FCCState, 2));// 飞控状态字
	discreteObj.insert("fault_status_level", toHexString(protocolData->m_SecondSubThreeFrame.FCCLevel,1)); // 故障等级

	// 2. 控制模式状态
	discreteObj.insert("control_mode_status", toHexString(protocolData->m_SecondSubThreeFrame.ControlMode,1));
	discreteObj.insert("mobility_modal_status", toHexString(protocolData->m_SecondSubThreeFrame.ManeuMode,1));
	discreteObj.insert("Modal_input_status", toHexString(protocolData->m_SecondSubThreeFrame.ModeUsed, 2));

	// 3. 发动机状态（枚举值转换）
	discreteObj.insert("engine_status", toHexString(protocolData->m_SecondSubThreeFrame.EngineMode,1));
	quint64 combinedSignal = 0;
	combinedSignal |= (static_cast<quint64>(protocolData->m_FirstSubFourFrame.SignalSource5) << 32);
	combinedSignal |= (static_cast<quint64>(protocolData->m_FirstSubFourFrame.SignalSource4) << 24);
	combinedSignal |= (static_cast<quint64>(protocolData->m_FirstSubFourFrame.SignalSource3) << 16);
	combinedSignal |= (static_cast<quint64>(protocolData->m_FirstSubFourFrame.SignalSource2) << 8);
	combinedSignal |= static_cast<quint64>(protocolData->m_FirstSubFourFrame.SignalSource1);
	discreteObj.insert("signal_source_selection_state", toHexString_64(combinedSignal, 5));
	//discreteObj.insert("signal_source_selection_state", toHexString(protocolData->m_FirstSubFourFrame.SignalSource1,1));

	// 5. 综合故障信息
	discreteObj.insert("fault_comprehensive_character", toHexString(static_cast<qint64>(protocolData->m_FirstSubFourFrame.FaultCode),4));

	// 6. 机载设备状态
	discreteObj.insert("airborne_equipment_status", toHexString(protocolData->m_SecondSubFourFrame.EquipmentState,2));
	discreteObj.insert("onboard_equipment_alarm_character1", toHexString(protocolData->m_SecondSubFourFrame.EquipmentAlarm1,1));
	discreteObj.insert("onboard_equipment_alarm_character2", toHexString(protocolData->m_SecondSubFourFrame.EquipmentAlarm2, 1));
	discreteObj.insert("onboard_equipment_alarm_character3", toHexString(protocolData->m_SecondSubFourFrame.EquipmentAlarm3, 1));
	discreteObj.insert("onboard_equipment_alarm_character4", toHexString(protocolData->m_SecondSubFourFrame.EquipmentAlarm4, 1));

	// 构建完整数据结构
	rootObject.insert("data", discreteObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "DiscreteStatus MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}
int MQTTProtocol::constructionDrivetrain(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	// 构造根JSON对象
	QJsonObject rootObject = createBaseJsonObject("drivetrain_td550_t1400");
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "drivetrain_td550_t1400");
	//rootObject.insert("source", "station");
	// 传动系统数据段
	QJsonObject drivetrainObj;

	// 1. 开关状态（按不同精度处理）
	drivetrainObj.insert("switch_K4",QString::number( protocolData->m_ThirdSubOneFrame.OpenK4,'f',6));  // 右到位K4
	drivetrainObj.insert("switch_K1", QString::number(protocolData->m_ThirdSubOneFrame.OpenK1, 'f', 3));  // 下限位K1
	drivetrainObj.insert("tension_state", QString::number(protocolData->m_ThirdSubOneFrame.Tension, 'f', 6));  // 张紧状态
	drivetrainObj.insert("switch_K3", QString::number(protocolData->m_ThirdSubTwoFrame.OpenK3, 'f', 3));  // 左到位K3
	drivetrainObj.insert("switch_K2", QString::number(protocolData->m_ThirdSubOneFrame.OpenK2, 'f', 3));  // 上限位K2
// 2. 转速和温度压力
	drivetrainObj.insert("rotor_speed", QString::number(protocolData->m_FirstSubThreeFrame.MainRotor,'f',1));  // 旋翼转速
	drivetrainObj.insert("Reducer_lubricating_oil_temperature", QString::number(protocolData->m_ThirdSubTwoFrame.ROiltemp, 'f', 1));  // 滑油温度
	drivetrainObj.insert("reducer_lubricating_oil_pressure", QString::number(protocolData->m_ThirdSubOneFrame.RetarderPressure, 'f', 2));  // 滑油压力
//舵机状态
	drivetrainObj.insert("main_steer_state_D1", toHexString(protocolData->m_FirstSubFourFrame.D1ServoMFault,1));  // d1
	drivetrainObj.insert("backup_steer_state_D1", toHexString(protocolData->m_FirstSubFourFrame.D1ServoBFault, 1));  // d1
	drivetrainObj.insert("main_steer_state_D2", toHexString(protocolData->m_FirstSubFourFrame.D2ServoMFault, 1));  // d1
	drivetrainObj.insert("backup_steer_state_D2", toHexString(protocolData->m_FirstSubFourFrame.D2ServoBFault, 1));  // d1
	drivetrainObj.insert("main_steer_state_D3", toHexString(protocolData->m_FirstSubFourFrame.D3ServoMFault, 1));  // d1
	drivetrainObj.insert("backup_steer_state_D3", toHexString(protocolData->m_FirstSubFourFrame.D3ServoBFault, 1));  // d1
	drivetrainObj.insert("main_steer_state_U1", toHexString(protocolData->m_FirstSubFourFrame.U1ServoMFault, 1));  // d1
	drivetrainObj.insert("backup_steer_state_U1", toHexString(protocolData->m_FirstSubFourFrame.U1ServoBFault, 1));  // d1
	drivetrainObj.insert("main_steer_state_U2", toHexString(protocolData->m_FirstSubFourFrame.U2ServoMFault, 1));  // d1
	drivetrainObj.insert("backup_steer_state_U2", toHexString(protocolData->m_FirstSubFourFrame.U2ServoBFault, 1));  // d1
	drivetrainObj.insert("main_steer_state_U3", toHexString(protocolData->m_FirstSubFourFrame.U3ServoMFault, 1));  // d1
	drivetrainObj.insert("backup_steer_state_U3", toHexString(protocolData->m_FirstSubFourFrame.U3ServoMFault, 1));  // d1
	drivetrainObj.insert("main_steer_state_door", toHexString(0, 1));  // d1
	drivetrainObj.insert("backup_steer_state_door", toHexString(0, 1));  // d1
	drivetrainObj.insert("main_steer_state_direction", toHexString(protocolData->m_FirstSubFourFrame.DirServoMFault, 1));  // d1
	drivetrainObj.insert("backup_steer_state_direction", toHexString(protocolData->m_FirstSubFourFrame.DirServoBFault, 1));  // d1
	
// 构建完整数据结构
	rootObject.insert("data", drivetrainObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "Drivetrain MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}
int MQTTProtocol::constructionFMS(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;

	QJsonObject rootObject = createBaseJsonObject("fms_td550_t1400");
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "fms_td550_t1400");
	//rootObject.insert("source", "station");
	// 飞行管理数据段
	QJsonObject fmsObj;

	// 1. 航路点信息
	fmsObj.insert("current_route_number", protocolData->m_SecondSubOneFrame.RouteNum);
	fmsObj.insert("current_waypoint_number", protocolData->m_SecondSubOneFrame.WayPointNum);
	fmsObj.insert("current_waypoint_characters", protocolData->m_SecondSubOneFrame.WayPointState);
	fmsObj.insert("next_waypoint_number", protocolData->m_SecondSubOneFrame.NextPointNum);
	fmsObj.insert("next_waypoint_character", protocolData->m_SecondSubOneFrame.NextPointState);

	// 2. 飞行阶段（枚举值转换）

	fmsObj.insert("flight_phase", toHexString(protocolData->m_SecondSubOneFrame.FlightState,1));

	// 构建完整数据结构

	rootObject.insert("data", fmsObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "FMS MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();

}
int MQTTProtocol::constructionGuidanceLaw(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	QJsonObject rootObject = createBaseJsonObject("guidance_law_td550_t1400");
	//// 构造根JSON对象
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "guidance_law_td550_t1400");
	//rootObject.insert("source", "station");
	//// 制导律数据段
	QJsonObject guidanceObj;

	// 1. 导航信号参数
	guidanceObj.insert("lateral_offset_signal", protocolData->m_SecondSubOneFrame.SetoverDis); // 侧偏距
	guidanceObj.insert("pending_flight_distance_signal", protocolData->m_SecondSubOneFrame.FlushingDis); // 待飞距
	guidanceObj.insert("waiting_time", protocolData->m_SecondSubOneFrame.FlushingTime); // 待飞时间
	guidanceObj.insert("track_error_angle",QString::number( protocolData->m_SecondSubOneFrame.TrackError,'f',2)); // 航迹误差角
	guidanceObj.insert("GSB", QString::number(protocolData->m_SecondSubTwoFrame.GSB, 'f', 2)); // 预置倾斜角

	// 2. 位置信息
	guidanceObj.insert("d_xy", QString::number(protocolData->m_SecondSubThreeFrame.DIS_XY, 'f', 1)); // 离起飞点距离
	guidanceObj.insert("radio_altitude", QString::number(protocolData->m_SecondSubFourFrame.RadioHeight, 'f', 1)); // 无线电高度

	// 3. 指令响应
	guidanceObj.insert("flight_switch_response", toHexString(protocolData->m_ThirdSubFourFrame.KgReply,4)); // 开关指令应答
	////qDebug() << protocolData->m_ThirdSubFourFrame.KgReply << u8"开关";
	guidanceObj.insert("flight_combination_response", toHexString(protocolData->m_ThirdSubFourFrame.YtReply,1)); // 组合指令回报

	rootObject.insert("data", guidanceObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "GuidanceLaw MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}
int MQTTProtocol::constructionControlLaw(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	QJsonObject rootObject = createBaseJsonObject("control_law_td550_t1400");
	//// 构造根JSON对象
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "control_law_td550_t1400");
	//rootObject.insert("source", "station");
	// 控制律数据段
	QJsonObject controlObj;

	// 1. 桨距控制（0.001°精度）
	controlObj.insert("b1c", QString::number(protocolData->m_SecondSubTwoFrame.b1c, 'f', 3));  // 俯仰
	controlObj.insert("a1c", QString::number(protocolData->m_SecondSubTwoFrame.a1c, 'f', 3));  // 滚转
	controlObj.insert("dtc", QString::number(protocolData->m_SecondSubTwoFrame.dtc, 'f', 3));  // 偏航
	controlObj.insert("ctc", QString::number(protocolData->m_SecondSubTwoFrame.ctc, 'f', 3));  // 总距

	// 2. 执行机构状态
	controlObj.insert("chk", 0.00);    // 风门开度
	// 3. 控制基准值
	controlObj.insert("give_value_speed_z", QString::number(protocolData->m_SecondSubTwoFrame.RouteVSpeed, 'f', 2));
	controlObj.insert("give_value_speed_y", QString::number(protocolData->m_SecondSubTwoFrame.RouteSpeed, 'f', 2));
	controlObj.insert("give_value_position_y", QString::number(protocolData->m_SecondSubTwoFrame.RouteVPostion,'f',1));
	controlObj.insert("give_value_speed_x", QString::number(protocolData->m_SecondSubTwoFrame.RouteSideSpeed ,'f', 2));
	controlObj.insert("give_value_position_x", QString::number(protocolData->m_SecondSubTwoFrame.RouteHPostion, 'f', 1));
	controlObj.insert("give_value_yaw", QString::number(protocolData->m_SecondSubTwoFrame.RouteYPostion, 'f', 2));
	controlObj.insert("give_value_alt", QString::number(protocolData->m_SecondSubTwoFrame.RouteHeight, 'f', 1));  // 0.2m精度处理

	// 4. 特殊参数
	controlObj.insert("landing_point_number", protocolData->m_SecondSubThreeFrame.ForceLandPointIndex);
	controlObj.insert("servo_pos", 0);      // 145专用
	controlObj.insert("servo_goal_pos", 0);

	// 构建完整数据结构
	rootObject.insert("data", controlObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "ControlLaw MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}

int MQTTProtocol::constructionEngine_t1400(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	QJsonObject rootObject = createBaseJsonObject("engine_t1400");
	// 构造根JSON对象
	/*QJsonObject rootObject;
	rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	rootObject.insert("gateway", "6722390T1400");
	rootObject.insert("method", "engine_t1400");
	rootObject.insert("source", "station");*/
	// 发动机数据段
	QJsonObject engineObj;

	// 1. 温度参数
	engineObj.insert("front_intake_manifold_temperature", QString::number(protocolData->m_ThirdSubThreeFrame.InairTemp,'f',1));  // 前发歧管温度
	engineObj.insert("rear_intake_manifold_temperature", QString::number(protocolData->m_ThirdSubThreeFrame.InairPre, 'f', 1));  // 后发歧管温度


	// 2. 转速参数
	engineObj.insert("front_Engine_speed", QString::number(protocolData->m_FirstSubThreeFrame.EngineR, 'f', 1));  // 前发发动机转速


	// 3. 压力参数
	engineObj.insert("front_fuel_pressure", QString::number((protocolData->m_ThirdSubOneFrame.OilPressure/5-0.1)/0.6667*10, 'f', 2));  // 前发燃油压力
	engineObj.insert("rear_fuel_pressure", QString::number((protocolData->m_ThirdSubOneFrame.OilPressure2 / 5 - 0.1) / 0.6667 * 10, 'f', 2));  // 后发燃油压力
	if (readOilStandardFlag)
	{
		m_xyldy = protocolData->m_ThirdSubOneFrame.OilVolume; //下油量电压;
	
			//wds-2024/09/23-9-45-14
			//通过电压值进行油量的插值;
			double oil = interpolation(protocolData->m_ThirdSubTwoFrame.OilPress + m_xyldy);

			if (oil != -1.0)
			{
				engineObj.insert("fuel_quantity", QString::number(oil, 'f', 2));  // 燃油油量
			}
	}
	engineObj.insert("front_oil_rail_pressure", QString::number(protocolData->m_ThirdSubTwoFrame.EngineOilPre145, 'f', 2)); // 前发油轨压力
	engineObj.insert("rear_oil_rail_pressure", QString::number(protocolData->m_ThirdSubTwoFrame.EngineOilTemp145, 'f', 2));  // 后发油轨压力
	// 4. 油量参数

	engineObj.insert("rear_Engine_speed", QString::number(protocolData->m_ThirdSubThreeFrame.EngineRAM, 'f', 1)); // 后发发动机转速
	engineObj.insert("front_intake_manifold_pressure", QString::number(protocolData->m_ThirdSubTwoFrame.TurboPressure, 'f', 1));// 前发歧管压力
	engineObj.insert("rear_intake_manifold_pressure", QString::number(protocolData->m_ThirdSubTwoFrame.CoolantTemp, 'f', 1));  // 后发歧管压力
	engineObj.insert("front_coolant_temperature", QString::number(protocolData->m_ThirdSubThreeFrame.CoolingTempMax, 'f', 1));  // 前发冷却液温度
	engineObj.insert("rear_coolant_temperature", QString::number(protocolData->m_ThirdSubThreeFrame.CoolingTempMin, 'f', 1));// 后发冷却液温度
	// 5. 油门开度
	engineObj.insert("front_feedback_throttle_opening", QString::number(protocolData->m_ThirdSubTwoFrame.ThrottlePos145, 'f', 2));  // 前发反馈油门开度
	engineObj.insert("rear_feedback_throttle_opening", QString::number(protocolData->m_ThirdSubThreeFrame.DamperPostion, 'f', 2));// 后发反馈油门开度
	engineObj.insert("front_given_throttle_opening", QString::number(protocolData->m_FirstSubTwoFrame.DamperServoControl, 'f', 2));  // 前发给定油门开度（使用前空燃比字段）
	engineObj.insert("rear_given_throttle_opening", QString::number(protocolData->m_FirstSubTwoFrame.DirServoControl, 'f', 2)); // 后发给定油门开度（使用后空燃比字段）

	// 6. 扭矩参数
	engineObj.insert("front_engine_torque", QString::number(protocolData->m_ThirdSubTwoFrame.ExhaustTemp1, 'f', 1));  // 前发动机扭矩
	engineObj.insert("rear_engine_torque", QString::number(protocolData->m_ThirdSubTwoFrame.ExhaustTemp2, 'f', 1)); // 后发动机扭矩

	// 7. 发动机状态
	engineObj.insert("front_engine_status", toHexString(protocolData->m_SecondSubThreeFrame.EngineMode&0x0F,1));  // 前发动机状态
	engineObj.insert("rear_engine_status", toHexString((protocolData->m_SecondSubThreeFrame.EngineMode>>4)&0x0F,1));  // 后发动机状态

	// 构建完整数据结构
	rootObject.insert("data", engineObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "Engine_t1400 MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}

int MQTTProtocol::constructionOther_t1400(unsigned char* sendBuf)
{
	TD550TelemetryData* protocolData = (TD550TelemetryData*)ReceiveUAVData;
	if (!protocolData)
		return 0;
	QJsonObject rootObject = createBaseJsonObject("addition_t1400");
	// 构造根JSON对象
	//QJsonObject rootObject;
	//rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	//rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	//rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	//rootObject.insert("gateway", "6722390T1400");
	//rootObject.insert("method", "addition_t1400");  // T1400专有数据
	//rootObject.insert("source", "station");
	// 专有数据段
	QJsonObject additionObj;

	// 1. 电压电流参数
	additionObj.insert("clutch_feedback_voltage", QString::number(protocolData->m_ThirdSubOneFrame.Tension, 'f', 3));   // 离合反馈电压
	additionObj.insert("front_generator_current", QString::number(protocolData->m_SecondSubFourFrame.GeneratorCurrent, 'f', 2));  // 前发电机电流
	additionObj.insert("rear_generator_current", QString::number(protocolData->m_SecondSubFourFrame.GeneratorCurrentBack, 'f', 2));  // 后发电机电流
	additionObj.insert("starting_battery_voltage", QString::number(protocolData->m_SecondSubThreeFrame.Reserved, 'f', 2));   // 起动电池电压

	// 2. 温度参数
	additionObj.insert("front_reducer_oil_temperature", QString::number(protocolData->m_ThirdSubTwoFrame.ROiltemp, 'f', 1));   // 前减滑油温度
	additionObj.insert("rear_reducer_oil_temperature", QString::number(protocolData->m_ThirdSubTwoFrame.ROilPress, 'f', 1));  // 后减滑油温度

	// 3. 转速参数
	additionObj.insert("front_rotor", QString::number(protocolData->m_ThirdSubOneFrame.OpenK2, 'f', 2));   // 前旋翼转速
	additionObj.insert("rear_rotor", QString::number(protocolData->m_ThirdSubTwoFrame.OpenK3, 'f', 2));   // 后旋翼转速

	// 4. 传动比参数（需要根据实际协议计算或从其他字段获取） 
	if (protocolData->m_ThirdSubOneFrame.OpenK2 != 0)
	{
		additionObj.insert("front_transmission_ratio", QString::number((protocolData->m_FirstSubThreeFrame.EngineR / protocolData->m_ThirdSubOneFrame.OpenK2), 'f', 2));  // 前发传动比（计算值）

	}
	else
	{
		additionObj.insert("front_transmission_ratio", 0);
	}
	if (protocolData->m_ThirdSubTwoFrame.OpenK3 != 0)
	{
		additionObj.insert("rear_transmission_ratio", QString::number((protocolData->m_ThirdSubThreeFrame.EngineRAM / protocolData->m_ThirdSubTwoFrame.OpenK3), 'f', 2));  // 后发传动比（计算值）

	}
	else
	{
		additionObj.insert("rear_transmission_ratio", 0);
	}

// 5. 导航参数
	additionObj.insert("backup_satellites_number",static_cast<int>(protocolData->m_SecondSubOneFrame.MEMSNum));  // 卫星颗数(备)

	// 6. 返航模式参数（需要根据实际协议从状态字中解析）
	additionObj.insert("broken_chain_return_mode",static_cast<int>(protocolData->m_SecondSubThreeFrame.ControlMode & 0x03));  // 断链返航方式（从控制模式状态字低2位解析）
	
	additionObj.insert("disperse_out2", toHexString(protocolData->m_ThirdSubOneFrame.DisperseOut2,4));  // 断链返航方式（从控制模式状态字低2位解析）

	// 构建完整数据结构
	rootObject.insert("data", additionObj);
	rootObject.insert("need_reply", 1);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "Addition_t1400 MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}

void MQTTProtocol::setLinkState(int type)
{
	m_linkState = type;
}

//QString MQTTProtocol::QDateTime::currentDateTime().toMSecsSinceEpoch());
//{
//	return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
//}

QString MQTTProtocol::generateOrderId()
{
	return QString("TD550-%1-%2")
		.arg(QDateTime::currentDateTime().toString("yyyyMMdd"))
		.arg(QUuid::createUuid().toString().mid(1, 8).toUpper());
}

QString MQTTProtocol::toHexString(quint32 value, int byteSize) {
	int width = byteSize * 2; // 每个字节对应2位16进制
	return QString("0x%1").arg(value, width, 16, QChar('0')).toUpper();
}

bool MQTTProtocol::checkOilStandardFile()
{
	//正常可用的情况下不给提示，不能正常使用的话给出提示;
	QString strPath = QCoreApplication::applicationDirPath() + "/init/OilStandard.ini";
	if (!QFile::exists(strPath))
		return false;

	vec_Voltage.clear();
	QSettings oilSettings(strPath, QSettings::IniFormat);
	//把数据读取到内存上来;
	for (int i = 0; i < 80; i++)
	{
		QString qs_key = QString("OilVoltage/Oil-%1").arg(i * 5 + 5);
		vec_Voltage.push_back(oilSettings.value(qs_key).toDouble());

	}

	if (vec_Voltage.size() != 80)
	{
		//GCS::coreLog.info("---WDS---checkOilStandardLFile---vec_Voltage.size()!=16");
		return false;
	}

	return true;
}

double MQTTProtocol::interpolation(double var)
{
	double nearValueL = 0.0;
	double nearValueR = 0.0;
	//wds-2024/09/23-10-07-32 寻找两个电压值;
	findNearValue(var, nearValueL, nearValueR);
	//wds-2024/09/23-11-37-30未找到相应值;
	if (nearValueL == 0.0 && nearValueR == 0.0)
	{
		//GCS::coreLog.info("---wds---interpolation---nearValueL:" + QString::number(nearValueL));
		//GCS::coreLog.info("---wds---interpolation---nearValueR:" + QString::number(nearValueR));
		return -1;
	}
	//寻找电压值对应的两个油量值;
	int oilLeft = findNearOilValue(nearValueL);
	int oilRight = oilLeft + 5;
	//插值;
	double result = 5 / ((nearValueR - nearValueL) / (var - nearValueL)) + oilLeft;
	return result;
}

void MQTTProtocol::findNearValue(double& var, double& nearValueL, double& nearValueR)
{
	if (var < vec_Voltage[0])
		var = vec_Voltage[0] + 0.001;
	if (var > vec_Voltage[vec_Voltage.size() - 1])
		var = vec_Voltage[vec_Voltage.size() - 1] - 0.001;

	for (int i = 0; i < vec_Voltage.size() - 1; i++)
	{
		if ((var > vec_Voltage[i] || var == vec_Voltage[i]) && var < vec_Voltage[i + 1])
		{
			nearValueL = vec_Voltage[i];
			nearValueR = vec_Voltage[i + 1];
			break;
		}
	}
}

int MQTTProtocol::findNearOilValue(double nearKeyL)
{
	for (int i = 0; i < vec_Voltage.size(); i++)
	{
		if (nearKeyL == vec_Voltage[i])
		{
			return 5 + (i * 5);
		}
	}
	return -1;
}
void MQTTProtocol::initLogConfig()
{
	m_strExePath = QCoreApplication::applicationDirPath();
	QString configPath = m_strExePath + "/init/Config.ini";
	QSettings settings(configPath, QSettings::IniFormat);

	// 文件上传配置
	m_deviceModel = settings.value("Device/Model", "TD550").toString();
	m_deviceSn = settings.value("Device/sn", "6722390TD550").toString();

}
QString MQTTProtocol::toHexString_64(quint64 value, int byteSize) {
	int width = byteSize * 2; // 每个字节对应2位16进制
	return QString("0x%1").arg(value, width, 16, QChar('0')).toUpper();
}

QJsonObject MQTTProtocol::createBaseJsonObject(const QString& method)
{
	
		QJsonObject rootObject;

		// 基本信息段
		rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
		rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
		rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
		rootObject.insert("gateway", m_deviceSn);
		rootObject.insert("method", method);
		rootObject.insert("source", "station");
		return rootObject;
	
}
