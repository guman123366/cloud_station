/**
 * @file td550_commands.h
 * @brief TD550无人机指令定义头文件
 * @version 1.0
 * @date 2023-08-15
 */


#include <QObject>

 /**
  * @brief TD550无人机指令命名空间
  */

struct td550Method
{

	QString flight_mode = "flight_mode";          // 飞行模式切换
	QString takeoff = "takeoff";                 // 起飞
	QString land = "land";                       // 降落
	QString go_home = "go_home";                 // 返航
	QString set_home = "set_home";               // 设置返航点
	QString stop = "stop";                       // 悬停
	QString navigate = "navigate";               // 指点飞行

	// 航线相关指令
	QString route_info = "route_info";           // 航线信息
	QString route_progress = "route_progress";   // 航线进度
	QString route_execute = "route_execute";     // 航线执行
	QString route_pause = "route_pause";         // 航线暂停
	QString route_resume = "route_resume";       // 航线继续
	QString route_finish = "route_finish";       // 航线结束

	// 负载设备指令
	QString mount_tda = "mount_tda";             // 挂载数据透传
	QString gimbal_control = "gimbal_control";   // 云台控制
	QString attitude_control = "attitude_control"; // 姿态控制

	// 相机控制指令
	QString camera_mode_switch = "camera_mode_switch"; // 切换相机模式
	QString camera_photo_take = "camera_photo_take";   // 拍照
	QString camera_recording_start = "camera_recording_start"; // 开始录像
	QString camera_recording_stop = "camera_recording_stop";   // 停止录像
	QString camera_focal_length_set = "camera_focal_length_set"; // 变焦

	// 直播相关指令
	QString live_start_push = "live_start_push"; // 开始直播
	QString live_stop_push = "live_stop_push";   // 停止直播

	// 系统控制指令
	QString device_boot = "device_boot";         // 飞行器启停
	QString device_charge = "device_charge";     // 飞行器充电
	QString property = "property";               // 获取设备属性参数
	//飞行开关指令 550 1400 6000
	QString cmd_switch = "cmd_switch";
	QString wp_insert = "wp_insert";             // 航点插入
	QString wp_delete = "wp_delete";             // 航点删除
	QString wp_modify = "wp_modify";             // 航点修改
	QString wp_query = "wp_query";               // 航点查询
	QString wp_task_close = "wp_task_close";     // 航点任务关闭
	QString wr_query = "wr_query";               // 航线查询
	QString point_remote_adjustment = "point_remote_adjustment"; // 点号遥调
	QString wr_load = "wr_load";                 // 航线装订
		// ================ 遥调指令 ================
	QString longitudinal_pos_remote_adjustment = "longitudinal_pos_remote_adjustment"; // 纵向位置遥调
	QString lateral_pos_remote_adjustment = "lateral_pos_remote_adjustment";         // 侧向位置遥调
	QString alt_remote_adjustment = "alt_remote_adjustment";                         // 高度遥调给定
	QString yaw_remote_adjustment = "yaw_remote_adjustment";                         // 航向遥调给定
	QString longitudinal_speed_remote_adjustment = "longitudinal_speed_remote_adjustment"; // 纵向速度遥调
	QString vertical_speed_remote_adjustment = "vertical_speed_remote_adjustment";   // 垂直速度遥调
	QString lateral_speed_remote_adjustment = "lateral_speed_remote_adjustment";     // 侧向速度遥调
	QString pos_deviation_injection = "pos_deviation_injection";                     // 位置偏差注入
	QString height_injection = "height_injection";                                   // 场高注入
	QString magnetic_injection = "magnetic_injection";                               // 磁偏角注入
	
	// ================ 飞行连续指令 ================
	QString longitudinal_cyclic_control = "longitudinal_cyclic_control"; // 纵向周期变距
	QString lateral_cyclic_control = "lateral_cyclic_control";         // 横向周期变距
	QString total_distance_control = "total_distance_control";         // 总距控制


	//td550-td1400
	QString 	cmd_switch_550_1400 = "cmd_switch_550_1400";
	// ================ 飞行组合指令 ================
	QString uav_takeoff_weight_load = "uav_takeoff_weight_load"; // 飞机起飞重量装订
	QString a_coordinate_load = "a_coordinate_load";           // A点坐标装订
	QString b_coordinate_load = "b_coordinate_load";           // B点坐标装订
	QString c_coordinate_load = "c_coordinate_load";           // C点坐标装订
	QString alternate_point1 = "alternate_point1";             // 备降点1上传
	QString alternate_point2 = "alternate_point2";             // 备降点2上传
	QString alternate_point3 = "alternate_point3";             // 备降点3上传
	QString alternate_point4 = "alternate_point4";             // 备降点4上传

	//飞行连续指令
	const QString tail_rotor_control = "tail_rotor_control";// 尾桨距控制
	const QString damper_opening_control = "damper_opening_control";// 风门开度控制
	const QString longitudinal_cyclic_cont = "longitudinal_cyclic_cont";// 开关信号控制
	//550zhilibf
	QString cmd_switch_550 = "cmd_switch_550";
	//1·400单独开关指令
	QString cmd_switch_1400 = "cmd_switch_1400"; 


};
namespace TD550_1400_R6000SwitchCommands {

	/**
	 * @brief 飞行开关指令编码枚举 (兼容TD550/TD1400/R6000)
	 */
	enum CommandCode {
		// 保持模式指令
		ALT_HOLD_ABS = 0x29,   // 绝对高度保持 (R6000:0x0106)
		AIRSPEED_HOLD = 0x30,   // 空速保持 (R6000:0x0102)
		ALT_HOLD_REL = 0x31,   // 相对高度保持 (R6000:0x0107)
		GROUNDSPEED_HOLD = 0x32,   // 地速保持 (R6000:0x0103)
		POSITION_HOLD = 0x33,   // 位置保持 (R6000:0x0105)
		VERTICAL_SPEED_HOLD = 0x34,   // 垂直速度保持 (R6000:0x0104)
		HEADING_HOLD = 0x35,   // 航向保持 (R6000:0x0101)

		// 自动飞行指令
		AUTO_NAV = 0x36,   // 自动导航 (R6000:0x0302)
		AUTO_TAKEOFF = 0x38,   // 自动起飞 (R6000:0x0201)
		AUTO_LAND = 0x39,   // 自动着陆 (R6000:0x0202)

		// 解除指令
		ALT_HOLD_RELEASE = 0x3A,   // 解除高度保持 (R6000:0x8107)
		SPEED_HOLD_RELEASE = 0x3B,   // 解除速度保持 (R6000:0x8103)
		POS_HOLD_RELEASE = 0x3C,   // 解除位置保持 (R6000:0x8105)
		HEADING_RELEASE = 0x3D,   // 解除定向 (R6000:0x8101)
		NAV_EXIT = 0x3E,   // 退出导航 (R6000:0x83FF)
		MANEUVER_EXIT = 0x3F,   // 退出机动 (R6000:0x82FF)

		// 特殊模式
		AUTO_HOVER = 0x61,   // 自动悬停 (R6000:0x0203)
		LINE_RETURN = 0x62,   // 直线归航 (R6000:0x0303)
		PATH_RETURN = 0x63    // 原路归航 (R6000:0x0304)
	};
};
	namespace TD550_1400 {

		/**
		 * @brief TD550/TD1400共用开关指令编码枚举
		 * @note 指令编码适用于TD550和TD1400型号
		 */
		enum  TD550_TD1400CommandSwitch {
			// 基础控制模式
			REMOTE_CTRL_MODE = 0x1A,   // 遥控模式
			MIXED_CTRL_MODE = 0x1B,   // 混控模式
			PROGRAM_CTRL_MODE = 0x1C,   // 程控模式
			PRE_COMMAND = 0x1D,   // 预指令
			AIR_GROUND_SWITCH = 0x1E,   // 空地开关(预留)

			// 发动机控制
			ENGINE_START = 0x20,   // 发动机开车
			ENGINE_IDLE = 0x21,   // 发动机怠速
			ENGINE_WARMUP = 0x22,   // 发动机暖车
			ENGINE_RATED = 0x23,   // 发动机额定
			ENGINE_STOP = 0x24,   // 发动机停车
			ONE_KEY_SHUTDOWN = 0x26,   // 一键关闭
			RELEASE_CMD = 0x28,   // 放飞
			AUTO_RETURN = 0x37,   // 自动返航

			// 电源系统
			ECU_A_POWER_ON = 0x41,   // 前发动机ECU供电
			ECU_A_POWER_OFF = 0x42,   // 前发动机ECU断电
			ECU_B_POWER_ON = 0x43,   // 后发动机ECU供电
			ECU_B_POWER_OFF = 0x44,   // 后发动机ECU断电
			GENERATOR_CONNECT = 0x45,   // 发电机并网
			GENERATOR_DISCONNECT = 0x46,   // 发电机退网

			// 机械系统
			CLUTCH_ENGAGE = 0x47,   // 离合器接合
			CLUTCH_DISENGAGE = 0x48,   // 离合器分离

			// 燃油系统
			FUEL_PUMP_A_ON = 0x49,   // 前发油泵开
			FUEL_PUMP_A_OFF = 0x4A,   // 前发油泵关
			FUEL_PUMP_B_ON = 0x4B,   // 后发油泵开
			FUEL_PUMP_B_OFF = 0x4C,   // 后发油泵关
			POWER_SYSTEM_ON = 0x4D,   // 后发电机接通
			POWER_SYSTEM_OFF = 0x4E,   // 后发电机断开

			// 设备控制
			DROP_DEVICE_OPEN = 0x52,   // 投放装置开启
			DROP_DEVICE_CLOSE = 0x53,   // 投放装置关闭
			EO_POD_POWER_ON = 0x54,   // 光电吊舱设备上电
			EO_POD_POWER_OFF = 0x55,   // 光电吊舱设备下电

			// 测试指令
			REMOTE_LINK_TEST = 0x58,   // 遥控遥测链路测试
			SPIN_TEST_START = 0x59,   // 执行掉转测试
			SPIN_TEST_CANCEL = 0x60,   // 取消掉转测试

			// 安全控制
			AIRSPEED_PROTECT_ON = 0x5C,   // 开启空速保护
			AIRSPEED_PROTECT_OFF = 0x5D,   // 禁用空速保护

			// 控制权切换
			EXTERNAL_BOX_CTRL = 0x5E,   // 切换为外控盒
			REMOTE_CTRL = 0x5F,   // 切换为遥控器

			// 飞行模式
			CLIMB_RETURN = 0x64,   // 爬升归航
			LEFT_ORBIT = 0x66,   // 左盘旋
			RIGHT_ORBIT = 0x67,   // 右盘旋
			UPWARD_TRANSITION = 0x68,   // 向上过渡
			DOWNWARD_TRANSITION = 0x69,   // 向下过渡
			OBSTACLE_AVOIDANCE = 0x6A,   // 自主避障
			ANTI_DISTURBANCE = 0x70    // 一键抗扰动
		};
	};

		namespace TD550_Specific {

			/**
			 * @brief TD550专用开关指令编码枚举
			 */
			enum class CommandCode {
				OIL_COOLING_ON = 0x4F,   // 滑油散热接通
				OIL_COOLING_OFF = 0x51,   // 滑油散热关闭
				ENGINE_COOLING_ON = 0x5A,   // 发动机散热接通
				ENGINE_COOLING_OFF = 0x5B,   // 发动机散热断开
				WATER_PUMP_PRESS = 0x6D,   // 水泵加压
				WATER_PUMP_RELEASE = 0x6E    // 水泵减压
			};


		};// namespace TD550
//对应地面站指令组合指令
		enum WaypointCommandType {
			WP_INSERT = 0x10,
			WP_DELETE = 0x11,
			WP_MODIFY = 0x12,
			WP_QUERY = 0x13,
			WP_TASK_CLOSE = 0x14,
			WR_QUERY = 0x15,
			WR_LOAD = 0x16,
			POINT_REMOTE_ADJUSTMENT = 0x21,
			LONGITUDINAL_POS_ADJUSTMENT = 0x22,
			LATERAL_POS_ADJUSTMENT = 0x23,
			ALT_ADJUSTMENT = 0x24,
			YAW_ADJUSTMENT = 0x25,
			LONGITUDINAL_SPEED_ADJUSTMENT = 0x26,
			VERTICAL_SPEED_ADJUSTMENT = 0x27,
			LATERAL_SPEED_ADJUSTMENT = 0x28,
			POS_DEVIATION_INJECTION = 0x31,
			HEIGHT_INJECTION = 0x32,
			MAGNETIC_INJECTION = 0x33,
			UAV_TAKEOFF_WEIGHT_LOAD = 0x17 , // 飞机起飞重量装订
			A_COORDINATE_LOAD = 0x34,
			B_COORDINATE_LOAD = 0x35,
			C_COORDINATE_LOAD = 0x36,
			ALTERNATE_POINT1=0x37,
			ALTERNATE_POINT2 = 0x38,
			ALTERNATE_POINT3 = 0x39,
			ALTERNATE_POINT4 = 0x40
		};


		// 航点数据结构
		struct WaypointData {
			int wr_num;     // 航线编号(1~20)
			int wp_num;     // 航点编号(1~150)
			int wp_type;    // 航点特征字(1~10)
			double wp_lon;  // 航点经度(±180°)
			double wp_lat;  // 航点纬度(±90°)
			float wp_alt;   // 航点高度(0~6000m)
			float wp_speed; // 航点速度(0~300km/h)
			int wp_time;    // 航点时间(0～65535s)
		};
		//航点类型
		enum ZHZ_UavWaypointType
		{
			UAV_YB_WAYPOINT = 0,	//一般航路点
			UAV_GD_WAYPOINT = 1,	//过顶航路点
			UAV_ZYX_WAYPOINT = 2,	//左圆心航路点
			UAV_YYX_WAYPOINT = 3,	//右圆心航路点
			UAV_JC_WAYPOINT = 4,	//机场航路点
			UAV_QF_WAYPOINT = 5,	//起飞航路点
			UAV_CC_WAYPOINT = 6,	//出场航路点
			UAV_JCH_WAYPOINT = 7,	//进场航路点
			UAV_XH_WAYPOINT = 8,	//下滑航路点
			UAV_ZL_WAYPOINT = 9,	//着陆航路点
			UAV_FH_WAYPOINT = 10,	//返航航路点
		};
		//基本航路点数据
		struct ZHZ_WaypointDataBase
		{
			ZHZ_WaypointDataBase()
			{
				m_nWPNum = 0;
				m_nARNum = 1;
				m_dLat = 0.0;
				m_dLon = 0.0;
				m_dAlt = 0.0;
				m_dSpeed = 0.0;
				m_nTime = 0;

				m_eWPType = UAV_YB_WAYPOINT;
			}

			int m_nWPNum;		//waypoint number [0,255]
			int m_nARNum;		//航线编号
			double m_dLat;		//latitude [rad] 纬度
			double m_dLon;		//longitude [rad] 经度
			double m_dAlt;		//altitude [m] 高度
			double m_dSpeed;	//cruise speed [m/s]
			int m_nTime;		//航点时间

			enum ZHZ_UavWaypointType m_eWPType;
		};