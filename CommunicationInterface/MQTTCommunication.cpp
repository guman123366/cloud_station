// MQTTCommunication.cpp
#include "MQTTCommunication.h"
#include <QDateTime>
#include<QCoreApplication>
#include "CurlUploader.h"  // 添加CurlUploader头文件
const QString MQTT_USER = "develop";
const QString MQTT_PASSWORD = "develop@uatair";
const QString MQTT_CLIENTID = "e5cd7e4891bf95d1d19206ce24a7b32e";

QString generateUuid() {
	// 创建一个随机UUID（版本4）
	QUuid uuid = QUuid::createUuid();

	// 转换为字符串并移除包围的花括号
	QString uuidStr = uuid.toString();
	uuidStr.remove(0, 1);  // 移除开头的'{'
	uuidStr.chop(1);       // 移除结尾的'}'

	return uuidStr;
}
MQTTCommunication::MQTTCommunication(QObject* parent)
	: CommunicationInterface(parent)
	, m_client(new QMqttClient(this))
	, m_mqttCmdParser(new MQTTCommandParser(this))
{
	QString strConfigFilePath = QCoreApplication::applicationDirPath() + "/init/Config.ini";
	m_pConfigFileSettings = new QSettings(strConfigFilePath, QSettings::IniFormat);
	m_client->setClientId(m_clientId);
	m_client->setProtocolVersion(QMqttClient::MQTT_3_1_1);
	connect(m_client, &QMqttClient::stateChanged, this, &MQTTCommunication::onStateChanged);
	connect(m_client, &QMqttClient::messageReceived, this, &MQTTCommunication::onMessageReceived);
	// 连接命令解析信号
	// 设置通信参数
	topicSubscribe = m_pConfigFileSettings->value("MQTTCommunication/MQTTCommandTopic").toString();
	topicPublish = m_pConfigFileSettings->value("MQTTCommunication/MQTTTelemetryTopic").toString();
	topicSubscribeReplay = m_pConfigFileSettings->value("MQTTCommunication/MQTTTelemetryReplayTopic").toString();
	m_responseTopic = m_pConfigFileSettings->value("MQTTCommunication/MQTTCommandReponseTopic").toString();

	//确定心跳包主题

	topicHeartBeat = m_pConfigFileSettings->value("MQTTCommunication/MQTTOnline").toString();
	topicHeartBeatReply = m_pConfigFileSettings->value("MQTTCommunication/MQTTOnlieReply").toString();
	connect(m_mqttCmdParser, &MQTTCommandParser::si_sendKgCommand, this, &MQTTCommunication::sl_recvKgCommand);
	connect(m_mqttCmdParser, &MQTTCommandParser::si_sendZuHeCommand, this, &MQTTCommunication::sl_recvZuHeCommand);
	// 初始化OSD ACK计时器
	//m_osdAckTimer = new QTimer(this);
	//connect(m_osdAckTimer, &QTimer::timeout, this, &MQTTCommunication::onOsdAckTimeout);
	//m_osdTimeoutInterval = 7000; // 7秒超时

	qDebug() << "heart" << topicHeartBeat << "___" << topicPublish;
  // 初始化心跳定时器
	m_heartbeatTimer = new QTimer(this);
	m_heartbeatTimer->setInterval(1000); // 5秒一次心跳（0.2Hz）
	connect(m_heartbeatTimer, &QTimer::timeout, this, &MQTTCommunication::sendHeartbeat);

	// 心跳响应超时定时器
	m_heartbeatAckTimer = new QTimer(this);
	m_heartbeatAckTimer->setSingleShot(true);
	m_heartbeatAckTimer->setInterval(10000); // 10秒超时
	connect(m_heartbeatAckTimer, &QTimer::timeout, this, &MQTTCommunication::onHeartbeatAckTimeout);

	m_heartbeatTimeoutInterval = 10000;
	m_isOnline = false;
	m_lastHeartbeatTime = QDateTime::currentDateTime();
	// 初始化重连机制
	m_reconnectTimer = new QTimer(this);
	m_reconnectTimer->setSingleShot(true); // 单次触发
	connect(m_reconnectTimer, &QTimer::timeout, this, &MQTTCommunication::attemptReconnect);

	m_reconnectInterval = 5000; // 5秒重连间隔
	m_maxReconnectAttempts = 10; // 最大重连尝试次数
	m_currentReconnectAttempts = 0;
	m_autoReconnect = true; // 默认开启自动重连

	// 读取网关ID（设备序列号）
	m_gatewayId = m_pConfigFileSettings->value("Device/sn", "6722390T1400").toString();

	// 读取MQTT认证相关配置
	m_mqttAuthUrl = m_pConfigFileSettings->value("FileUpload/UploadUrl",
		"https://cloud-gray.uatair.com/lfy-api").toString();
	m_mqttAuthUrl = m_mqttAuthUrl + "/auth/mqttUser/device/authenticate";

	m_clientId = m_pConfigFileSettings->value("MQTTCommunication/ClientId",
		"e5cd7e4891bf95d1d19206ce24a7b32e").toString();
	m_deviceModel = m_pConfigFileSettings->value("Device/Model", "TD550").toString();
	m_deviceSn = m_pConfigFileSettings->value("Device/sn", "6722390TD550").toString();
}

MQTTCommunication::~MQTTCommunication()
{
	closePort();
}


bool MQTTCommunication::openPort()
{
	//if (m_client->state() == QMqttClient::Disconnected) {

	//	m_client->setHostname(ListenAddress);
	//	m_client->setPort(ListenPort);

	//	m_client->setUsername(MQTT_USER);
	//	m_client->setPassword(MQTT_PASSWORD);
	//	m_client->setKeepAlive(60);

	//	m_client->connectToHost();
	//	return true;
	//}
	 // 已连接状态处理
	if (m_client->state() == QMqttClient::Connected) {
		qDebug() << "MQTT already connected";
		return true;
	}

	// 正在连接状态处理
	if (m_client->state() == QMqttClient::Connecting) {
		qWarning() << "MQTT connection in progress";
		return false;
	}
	//// 同步获取凭证（阻塞式）
	//if (!getMqttCredentials()) {
	//	qWarning() << "Failed to get MQTT credentials, cannot connect";
	//	return false;
	//}
	// 配置连接参数
	m_client->setHostname(ListenAddress);
	m_client->setPort(ListenPort);

	m_client->setUsername(MQTT_USER);
	m_client->setPassword(MQTT_PASSWORD);
	m_client->setKeepAlive(60);
	m_client->setClientId(MQTT_CLIENTID);
	//m_client->setUsername(m_mqttUsername);
	//m_client->setPassword(m_mqttPassword);
	//m_client->setKeepAlive(60);
	//m_client->setClientId(m_mqttCilentId);
	// 发起连接
	m_client->connectToHost();
	return true; // 仅表示连接流程已启动

}

bool MQTTCommunication::sendData(QByteArray ary, int nLength)
{
	//// 如果正在等待前一个OSD ACK，则跳过新命令
	//if (m_waitingForOsdAck) {
	//	qWarning() << "Waiting for previous OSD ACK, skipping command:";
	//	return false;
	//}

	//// 发送命令前启动OSD ACK计时器
	//m_osdAckTimer->start(m_osdTimeoutInterval);
	Q_UNUSED(nLength)
		if (m_client->state() != QMqttClient::Connected)
		{
			return false;
		}

	// 使用SendAddress作为topic，SendPort作为QoS
	quint8 qos = static_cast<quint8>(qMin(qMax(SendPort, 0), 2));

	m_client->publish(topicPublish, ary, qos, false);
	//qDebug().noquote() << u8"发送原始MQTT消息--";
	//qDebug().noquote() << u8"内容:" << ary;
	//qDebug().noquote() << QString::fromLocal8Bit("-----------------------------");
	return true;
}

bool MQTTCommunication::sendDataCmdAck(QByteArray ary, int nLength)
{
	// 使用SendAddress作为topic，SendPort作为QoS
	quint8 qos = static_cast<quint8>(qMin(qMax(SendPort, 0), 2));

	m_client->publish(m_responseTopic, ary, qos, false);
	//qDebug().noquote() << u8"发送原始MQTT消息--";
	//qDebug().noquote() << u8"内容:" << ary;
	//qDebug().noquote() << QString::fromLocal8Bit("-----------------------------");
	return true;
}

bool MQTTCommunication::closePort()
{
	if (getConnectSuccess()) {
		m_client->disconnectFromHost();
	}
	setConnectSuccess(false);
	return true;
}


void MQTTCommunication::setClientId(const QString& clientId)
{
	m_clientId = clientId;
}

void MQTTCommunication::setKeepAlive(int seconds)
{
	m_keepAlive = seconds;
}

bool MQTTCommunication::getMqttCredentials()
{
	try {
		// 使用CurlUploader获取凭证
		CurlUploader curlUploader;
		curlUploader.setMqttAuthUrl(m_mqttAuthUrl.toStdString());

		// 发送请求
		CurlUploader::MqttCredentials credentials = curlUploader.requestMqttCredentials(
			
			m_gatewayId.toStdString(),
			"e5cd7e4891bf95d1d19206ce24a7b32e",
			"station");

		//// 检查错误
		//if (!credentials.error.empty()) {
		//	qWarning() << "Failed to get MQTT credentials:"
		//		<< QString::fromStdString(credentials.error);
		//	emit mqttCredentialsError(QString::fromStdString(credentials.error));
		//	return false;
		//}

		//// 验证凭证
		//if (credentials.username.empty() || credentials.password.empty()) {
		//	qWarning() << "Invalid MQTT credentials received";
		//	emit mqttCredentialsError("Invalid credentials received");
		//	return false;
		//}

		// 保存凭证
		m_mqttUsername = QString::fromStdString(credentials.username);
		m_mqttPassword = QString::fromStdString(credentials.password);
		m_mqttCilentId = QString::fromStdString(credentials.clientid);
		//// 如果服务器地址和端口在响应中提供，则使用它们
		//if (!credentials.server.empty()) {
		//	m_mqttServerHost = QString::fromStdString(credentials.server);
		//}
		//if (credentials.port > 0) {
		//	m_mqttServerPort = credentials.port;
		//}

		//qInfo() << "MQTT credentials received successfully";
		//qDebug() << "  Username:" << m_mqttUsername;
		//qDebug() << "  Password length:" << m_mqttPassword.length() << "chars";
		//qDebug() << "  Server:" << m_mqttServerHost << ":" << m_mqttServerPort;

		//// 更新客户端设置
		//updateClientCredentials();

		//// 发出信号
		//emit mqttCredentialsReady();
		return true;

	}
	catch (const std::exception& e) {
		/*qCritical() << "Exception while getting MQTT credentials:" << e.what();
		emit mqttCredentialsError(QString("Exception: %1").arg(e.what()));*/
		return false;
	}
}

bool MQTTCommunication::areCredentialsValid() const
{
	return false;
}

bool MQTTCommunication::updateCredentialsManually()
{
	return false;
}



void MQTTCommunication::onStateChanged(QMqttClient::ClientState state)
{
	bool connected = (state == QMqttClient::Connected);
	setConnectSuccess(connected);
	// 连接成功后自动订阅命令主题
	if (connected) {
		subscribeToCommands();
		// 连接成功后发送online消息
		//sendOnlineMessage();
		// 启动心跳定时器
		startHeartbeat();
		//subscribeToCommands();
	}
	else {
		//stopHeartbeat();
		/*m_isOnline = false;
		emit onlineStatusChanged(false);*/
		stopHeartbeat();
		//m_isOnline = false;
		//emit onlineStatusChanged(false);

		// 如果是意外断开且允许自动重连
		if (m_autoReconnect && state == QMqttClient::Disconnected) {
			startReconnect();
		}

		qWarning() << "MQTT connection state changed to:" << state;
	
	}
}
bool MQTTCommunication::parseTemReply(const QByteArray& jsonData, TelemetryReply& outReply)
{
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);

	if (error.error != QJsonParseError::NoError) {
		qDebug() << ("JSON解析错误: " + error.errorString());
		return false;
	}

	if (!doc.isObject()) {
		qDebug() << ("根元素不是JSON对象");
		return false;
	}

	QJsonObject root = doc.object();

	// 校验必需字段
	const QStringList requiredFields = { "tid", "bid", "timestamp", "gateway", "method", "data" };
	for (const auto& field : requiredFields) {
		if (!root.contains(field)) {

			return false;
		}
	}

	// 提取基础信息
	outReply.tid = root["tid"].toString();
	outReply.bid = root["bid"].toString();
	outReply.timestamp = root["timestamp"].toVariant().toLongLong();
	outReply.gateway = root["gateway"].toString();
	outReply.method = root["method"].toString();

	// 解析data段
	QJsonObject data = root["data"].toObject();
	if (!data.contains("result")) {

		return false;
	}
	outReply.result = data["result"].toInt();

	// 提取output数据（如果存在）
	if (data.contains("output") && data["output"].isObject()) {
		outReply.output = data["output"].toObject();
	}
	return true;

}

int MQTTCommunication::constructionCommandAck(unsigned char* sendBuf, const QString& method, int type, int code, const QString& msg, const QString& status, int percent, const QJsonObject& waypointInfo)
{
	// 构造根JSON对象
	QJsonObject rootObject;
	rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
	rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
	rootObject.insert("timestamp", QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
	rootObject.insert("gateway", m_deviceSn);
	rootObject.insert("method", method); // 保持与原指令相同的method

	// 构造数据段
	QJsonObject dataObj;
	dataObj.insert("type", type);  // 0-地面状态 1-无人机状态
	dataObj.insert("code", code);  // 0=成功

	// 可选字段处理
	if (!msg.isEmpty()) {
		dataObj.insert("msg", msg);
	}

	if (!status.isEmpty()) {
		dataObj.insert("status", status);
	}

	if (percent >= 0) {
		dataObj.insert("percent", percent);
	}

	if (!waypointInfo.isEmpty()) {
		dataObj.insert("waypoint", waypointInfo);
	}

	rootObject.insert("data", dataObj);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "CommandAck MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}

void MQTTCommunication::sendAck(QString method)
{
	
	unsigned char temp[2048] = { 0 };
	QJsonObject waypoint;
	int len = m_mqttCmdParser->constructionCommandAck(temp, method, 0, 0, method, STATUS_SUCCESSED, waypoint);
	if (len > 0) {
		this->sendDataCmdAck(QByteArray((char*)temp, len), len);
	}
}

void MQTTCommunication::sendTemAck(QString method)
{
	// 如果不是MQTTProtocol，只发送基础数据
	unsigned char temp[2048] = { 0 };
	QJsonObject waypoint;
	if (method == "")
	{
	}
	else if (method == "")
	{
	}
	else
	{
		if (m_sendToMqttTem)
		{
			QString msg = m_mqttCmdParser->KgOrderReply(m_sendToMqttTem->m_ThirdSubFourFrame.KgReply);
			int len = m_mqttCmdParser->constructionCommandAck(temp, method, 1, 0, msg.toUtf8(), STATUS_SUCCESSED, waypoint);
			/*qDebug() << "msg" << msg.toUtf8();*/
			if (len > 0) {
				this->sendDataCmdAck(QByteArray((char*)temp, len), len);
			}
		}
	}

}

void MQTTCommunication::parseTemAck(QString method, QByteArray recvMsg)
{
	TelemetryReply reply;
	if (method == "osd")//osd遥测指令回报
	{
		//m_waitingForOsdAck = true;
		//if (!parseTemReply(recvMsg, reply)) {
		//	qWarning() << "Failed to parse telemetry reply";
		//	return;

		//}
		//// 收到OSD响应，停止计时器
		//if (m_waitingForOsdAck) {
		//	m_osdAckTimer->stop();
		//	m_waitingForOsdAck = false;
		//}
	}
	else if (method == "main_ins")//组合导航系统数据
	{

		TelemetryReply reply;
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "engine_td550_t1400")//发动机
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "remote_control_td550_t1400")//遥控指令
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "power_td550_t1400")//电源状态
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "discrete_td550_t1400")//离散状态
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "drivetrain_td550_t1400")//传动系统
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "fms_td550_t1400")//飞行管理
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "guidance_law_td550_t1400")//制导律
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}
	else if (method == "control_law_td550_t1400")//控制律
	{
		if (!parseTemReply(recvMsg, reply)) {
			qWarning() << "Failed to parse telemetry reply";
			return;
		}
	}

}

void MQTTCommunication::recvControlData(QString method, QByteArray message)
{
	td550Method methods; // 指令定义结构体实例
	// 1. 基本飞行指令
	if (method == methods.flight_mode)
	{

	}
	else if (method == methods.takeoff)
	{

	}
	else if (method == methods.land)
	{

	}
	else if (method == methods.go_home)
	{

	}
	else if (method == methods.set_home)
	{

	}
	else if (method == methods.stop)
	{

	}
	else if (method == methods.navigate)
	{

	}

	// 2. 航线相关指令
	else if (method == methods.route_info)
	{

	}
	else if (method == methods.route_progress)
	{

	}
	else if (method == methods.route_execute)
	{

	}
	else if (method == methods.route_pause)
	{

	}
	else if (method == methods.route_resume)
	{

	}
	else if (method == methods.route_finish)
	{

	}

	// 3. 负载设备指令
	else if (method == methods.mount_tda)
	{

	}
	else if (method == methods.gimbal_control)
	{

	}
	else if (method == methods.attitude_control)
	{

	}

	// 4. 相机控制指令
	else if (method == methods.camera_mode_switch)
	{

	}
	else if (method == methods.camera_photo_take)
	{

	}
	else if (method == methods.camera_recording_start)
	{

	}
	else if (method == methods.camera_recording_stop)
	{

	}
	else if (method == methods.camera_focal_length_set)
	{

	}

	// 5. 直播相关指令
	else if (method == methods.live_start_push)
	{

	}
	else if (method == methods.live_stop_push)
	{

	}

	// 6. 系统控制指令
	else if (method == methods.device_boot)
	{

	}
	else if (method == methods.device_charge)
	{

	}
	else if (method == methods.property)
	{

	}
	else if (method == methods.cmd_switch)
	{
		CommandPacket packet1;
		if (m_mqttCmdParser->parseFligthCommand(message))
		{
			//解析成功之后发送ack函数
			sendAck(method);
			//获取飞控最新的遥测指令
			//emit si_getLastTem();
			m_method = method;
		}
		else {
			qWarning() << "协议解析失败";
		}

	}
	else if (method == methods.cmd_switch_1400)
	{
	CommandPacket packet1;
	if (m_mqttCmdParser->parseFligthCommand(message))
	{
		//解析成功之后发送ack函数
		sendAck(method);
		//获取飞控最新的遥测指令
		//emit si_getLastTem();
		m_method = method;
	}
	else {
		qWarning() << "协议解析失败";
	}
	}

	// 7. 航点相关指令
	else if (method == methods.wp_insert)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
		else {
			qWarning() << "协议解析失败";
		}



	}
	else if (method == methods.wp_delete)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.wp_modify)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.wp_query)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.wp_task_close)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.wr_query)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.point_remote_adjustment)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
			m_method = method;
		}
	}
	else if (method == methods.wr_load)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
			pointsVec.clear();
			pointsVec = m_mqttCmdParser->getCurrentWPData();
			emit  si_sendVec(pointsVec);
		}
	}

	// 8. 遥调指令
	else if (method == methods.longitudinal_pos_remote_adjustment)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.lateral_pos_remote_adjustment)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.alt_remote_adjustment)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}

	}
	else if (method == methods.yaw_remote_adjustment)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.longitudinal_speed_remote_adjustment)
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.vertical_speed_remote_adjustment) {

		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.lateral_speed_remote_adjustment) {
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.pos_deviation_injection) {

		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.height_injection) {
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.magnetic_injection) {
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			sendAck(method);
		}
	}
	// 9. 飞行连续指令
	else if (method == methods.longitudinal_cyclic_control)//纵向周期变距
	{

		if (m_mqttCmdParser->parseLxCmd(message))
		{

			sendAck(method);
		}

	}
	else if (method == methods.lateral_cyclic_control) //横向周期变距
	{
		if (m_mqttCmdParser->parseLxCmd(message))
		{

			sendAck(method);
		}
	}
	else if (method == methods.total_distance_control)//总距
	{
		if (m_mqttCmdParser->parseLxCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.tail_rotor_control)//尾桨距
	{
		if (m_mqttCmdParser->parseLxCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.damper_opening_control)//风门开度
	{
		if (m_mqttCmdParser->parseLxCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}

	}
	else if (method == methods.longitudinal_cyclic_cont)//开关信号
	{
		if (m_mqttCmdParser->parseLxCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}

	// 10. //飞行开关指令
	else if (method == methods.cmd_switch_550_1400)
	{
		CommandPacket packet1;
		if (m_mqttCmdParser->parseFligthCommand(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
			//获取飞控最新的遥测指令
			//emit si_getLastTem();
			m_method = method;
		}
		else {
			qWarning() << "协议解析失败";
		}

	}
	else if (method == methods.cmd_switch_550)
	{
		CommandPacket packet1;
		if (m_mqttCmdParser->parseFligthCommand(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
			//获取飞控最新的遥测指令
			//emit si_getLastTem();
			//sendTemAck(method);
						//emit si_getLastTem();
			m_method = method;
		}
		else {
			qWarning() << "协议解析失败";
		}
	}
	else if (method == methods.cmd_switch_1400)
	{
	CommandPacket packet1;
	if (m_mqttCmdParser->parseFligthCommand(message))
	{

		//解析成功之后发送ack函数
		sendAck(method);
		//获取飞控最新的遥测指令
		//emit si_getLastTem();
		//sendTemAck(method);
					//emit si_getLastTem();
		m_method = method;
	}
	else {
		qWarning() << "协议解析失败";
	}
	}
	

	// 11. 飞行组合指令
	else if (method == methods.uav_takeoff_weight_load)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.a_coordinate_load)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.b_coordinate_load)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.c_coordinate_load)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.alternate_point1)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.alternate_point2)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.alternate_point3)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}
	}
	else if (method == methods.alternate_point4)//
	{
		if (m_mqttCmdParser->parseZuheCmd(message))
		{

			//解析成功之后发送ack函数
			sendAck(method);
		}

	}
	else
	{
		//qWarning() << u8"未知的指令类型:" << method;
	}
}

void MQTTCommunication::onMessageReceived(const QByteArray& message, const QMqttTopicName& topic)
{
	// 检查是否为状态回复主题
	if (topic.name().contains("status_reply")) {
		handleStatusReply(message, topic);
		return;
	}
	//QString method = topic.name();
	QJsonParseError parseError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message, &parseError);

	if (parseError.error != QJsonParseError::NoError) {
		qWarning() << "JSON parse error:" << parseError.errorString();

	}

	if (!jsonDoc.isObject()) {
		qWarning() << "Invalid JSON format: root is not an object";

	}

	QJsonObject rootObj = jsonDoc.object();

	// 检查必需字段
	const QStringList requiredFields = { "tid", "bid", "timestamp", "gateway", "method", "data" };
	for (const QString& field : requiredFields) {
		if (!rootObj.contains(field)) {
			qWarning() << "Missing required field:" << field;

		}
	}

	QString method = rootObj["method"].toString();

	parseTemAck(method, message);
	recvControlData(method, message);

	//qDebug().noquote() << u8"收到原始MQTT消息--";
	//qDebug().noquote() << u8"主题:" << topic.name();
	//qDebug().noquote() << u8"内容:" << message;
	//qDebug().noquote() << QString::fromLocal8Bit("-----------------------------");


}

void MQTTCommunication::onOsdAckTimeout()
{
	//if (m_waitingForOsdAck) {
	//	qWarning() << "OSD ACK timeout! Stopping remote control commands.";
	//	m_waitingForOsdAck = true;;
	//	emit si_osdAckTimeout(m_waitingForOsdAck); // 通知其他组件
	//}
}

void MQTTCommunication::sl_recvKgCommand(unsigned char nType)
{

	emit si_sendKgCommand(nType);
}
void MQTTCommunication::sl_recvYtCommand(unsigned char nType, double dData)
{
}
void MQTTCommunication::sl_recvLxCommand(char*, int)
{

}
void MQTTCommunication::sl_recvZuHeCommand(unsigned char type, QVector<double> vec)
{
	si_sendZuHeCommand(type, vec);
}
void MQTTCommunication::sl_handleImmediateSend()
{
}

void MQTTCommunication::sl_sendLoadAck(QString relpy)
{
	// 如果不是MQTTProtocol，只发送基础数据
	unsigned char temp[2048] = { 0 };
	QJsonObject waypoint;
	QString method = "wr_load";


	int len = m_mqttCmdParser->constructionCommandAck(temp, method, 1, 0, relpy, STATUS_SUCCESSED, waypoint);

	if (len > 0) {
		this->sendDataCmdAck(QByteArray((char*)temp, len), len);
	}
}

void MQTTCommunication::sl_updateRadioInfo(int type)
{
	emit si_updateLinkState(type);
	m_type = type;
}

void MQTTCommunication::sendOnlineMessage()
{
	if (m_client->state() != QMqttClient::Connected) {
		qWarning() << "Cannot send online message: MQTT client not connected";
		return;
	}
	unsigned char temp[2048] = { 0 };
	QJsonObject waypoint;
	int len = m_mqttCmdParser->constructionOnline(temp);
	if (len > 0) {
		
		quint8 qos = static_cast<quint8>(qMin(qMax(SendPort, 0), 2));
		QByteArray sendData = (char*)temp;
		m_client->publish(topicHeartBeat, sendData,qos, false);
	}
}

void MQTTCommunication::startHeartbeat()
{
	if (!m_heartbeatTimer->isActive()) {
		m_heartbeatTimer->start();
		qInfo() << "Heartbeat timer started with interval:" << m_heartbeatTimer->interval() << "ms";
	}
}

void MQTTCommunication::stopHeartbeat()
{
	if (m_heartbeatTimer->isActive()) {
		m_heartbeatTimer->stop();
		qInfo() << "Heartbeat timer stopped";
	}

	if (m_heartbeatAckTimer->isActive()) {
		m_heartbeatAckTimer->stop();
	}

	//m_isOnline = false;
	//emit onlineStatusChanged(false);
}

void MQTTCommunication::handleStatusReply(const QByteArray& message, const QMqttTopicName& topic)
{
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(message, &error);

	if (error.error != QJsonParseError::NoError) {
		qWarning() << "JSON parse error in status reply:" << error.errorString();
		return;
	}

	QJsonObject root = doc.object();
	QString method = root["method"].toString();

	// 停止超时定时器
	if (m_heartbeatAckTimer->isActive()) {
		m_heartbeatAckTimer->stop();
	}

	QJsonObject data = root["data"].toObject();
	int result = data["result"].toInt();

	if (result == 0) {
		m_isOnline = true;
		emit onlineStatusChanged(true);

		if (method == "heartbeat") {
			QJsonObject output = data["output"].toObject();
			long uplinkDelay = output["uplink"].toVariant().toLongLong();

			//qDebug() << "Heartbeat acknowledged - Uplink delay:" << uplinkDelay << "ms";
			emit heartbeatAcknowledged(uplinkDelay);
		}
		else if (method == "online") {
			//qInfo() << "Device online confirmed by server";
		}
	}
	else {
		qWarning() << method << "failed with result code:" << result;
		//m_isOnline = false;
		//emit onlineStatusChanged(false);
		//emit heartbeatFailed(result);
	}
}

void MQTTCommunication::onHeartbeatAckTimeout()
{
	qWarning() << "Heartbeat acknowledgment timeout - No response from server within"
		<< m_heartbeatTimeoutInterval / 1000 << "seconds";

	//m_isOnline = false;
	//emit onlineStatusChanged(false);
	//emit heartbeatTimeout();

	// 可以选择重新发送心跳或尝试重连
	if (m_client->state() == QMqttClient::Connected) {
		qInfo() << "Resending heartbeat due to timeout";
		sendHeartbeat();
	}
}

void MQTTCommunication::sendHeartbeat()
{
	// 检查UDP设备是否在线
	if (!m_isOnline) {
		qWarning() << "Skipping heartbeat: UDP device is offline";
		return;
	}

	// 原有的心跳发送逻辑
	if (m_client->state() != QMqttClient::Connected) {
		qWarning() << "Cannot send heartbeat: MQTT client not connected";
		//m_isOnline = false;
		//emit onlineStatusChanged(false);
		return;
	}
	unsigned char temp[2048] = { 0 };
	QJsonObject waypoint;
	int len = m_mqttCmdParser->constructionHeartbeat(temp);
	if (len > 0) {

		quint8 qos = static_cast<quint8>(qMin(qMax(SendPort, 0), 2));
		QByteArray sendData = (char*)temp;
		m_client->publish(topicHeartBeat, sendData, qos, false);
	}
}

void MQTTCommunication::sl_recvLastTem550(TD550TelemetryData* ThirdDataUsed)
{
	//qDebug() << ThirdDataUsed << "ThirdDataUsed";
	if (ThirdDataUsed) {
		m_sendToMqttTem = ThirdDataUsed;
		sendTemAck(m_method);
	}
	else {
		qWarning() << "Invalid data type received in sl_recvLastTem550";
	}
}



// 实现订阅命令主题
void MQTTCommunication::subscribeToCommands()
{
	if (m_client->state() == QMqttClient::Connected)
	{
		auto subscription = m_client->subscribe(topicSubscribe, 1); // QoS 1
		//订阅回复的消息
		auto repalySubscription = m_client->subscribe(topicSubscribeReplay, 1); // QoS 1
		auto heartbeatSubscription = m_client->subscribe(topicHeartBeatReply, 1); // QoS 1
		if (!subscription) {
			qWarning() << "Failed to subscribe to command topic:";
		}
		else {
			qDebug() << "Subscribed to command topic:";
		}

	}
}



void MQTTCommunication::onCommandParsed(CommandPacket& packet)
{
	QString timeStr = QDateTime::fromMSecsSinceEpoch(packet.timestamp)
		.toString("yyyy-MM-dd hh:mm:ss.zzz");

	/*qDebug().noquote() << u8"飞行开关指令 ";
	qDebug().noquote() << u8"设备ID:" << packet.gateway << "=============\n";
	qDebug().noquote() << u8"事务ID:" << packet.tid << "====================\n";
	qDebug().noquote() << u8"时间戳:" << timeStr << "====================\n";
	qDebug().noquote() << "";
	qDebug().noquote() << u8"指令码" << QString::number(packet.cmd_value, 16).toUpper() << "====================\n";
	qDebug().noquote() << u8"指令名称:" << packet.cmd_name << "====================\n";
	qDebug().noquote() << "\n";*/
}


void MQTTCommunication::onUdpDeviceOnlineStatusChanged(bool isOnline)
{
	m_deviceOnline = isOnline;
	m_isOnline = isOnline;
	if (isOnline) {
		qInfo() << "UDP device came online, ensuring MQTT connection";
		// 设备上线，确保MQTT连接
		//if (m_client->state() != QMqttClient::Connected) {
		//	//openPort();
		//}
		// 立即发送online消息
		sendOnlineMessage();
	}
	else {
		qWarning() << "UDP device went offline";
		// 可以根据需要决定是否断开MQTT连接
		stopHeartbeat();
	}
}

void MQTTCommunication::onUdpDataReceived()
{
	// 如果MQTT不在线，尝试重新连接
	if (m_client->state() != QMqttClient::Connected && !m_deviceOnline) {
		qInfo() << "UDP data received but MQTT offline, attempting to reconnect"<< m_client->state()<< m_deviceOnline;
		openPort();
	}
}
// 启动重连过程
void MQTTCommunication::startReconnect()
{
	if (m_currentReconnectAttempts >= m_maxReconnectAttempts) {
		qCritical() << "Maximum reconnect attempts reached (" << m_maxReconnectAttempts << "), giving up";
		//emit reconnectFailed(m_maxReconnectAttempts);
		return;
	}

	m_currentReconnectAttempts++;
	qInfo() << "Attempting to reconnect (" << m_currentReconnectAttempts << "/" << m_maxReconnectAttempts << ")";

	// 等待一段时间后尝试重连
	m_reconnectTimer->start(m_reconnectInterval);
}

// 尝试重连
void MQTTCommunication::attemptReconnect()
{
	if (m_client->state() == QMqttClient::Connected) {
		qDebug() << "Already connected, skipping reconnect attempt";
		return;
	}

	qInfo() << "Reconnecting to MQTT broker...";

	// 清理现有连接
	if (m_client->state() != QMqttClient::Disconnected) {
		m_client->disconnectFromHost();
		// 等待完全断开
		QTimer::singleShot(1000, this, [this]() {
			openPort();
			});
	}
	else {
		openPort();
	}
}

// 重连超时处理
void MQTTCommunication::onReconnectTimeout()
{
	qWarning() << "Reconnect attempt timed out";
	if (m_autoReconnect) {
		startReconnect();
	}
}

// 手动触发重连
void MQTTCommunication::reconnectNow()
{
	if (m_reconnectTimer->isActive()) {
		m_reconnectTimer->stop();
	}
	m_currentReconnectAttempts = 0;
	attemptReconnect();
}

// 设置重连参数
void MQTTCommunication::setReconnectSettings(int intervalMs, int maxAttempts, bool autoReconnect)
{
	m_reconnectInterval = qMax(1000, intervalMs); // 最小1秒
	m_maxReconnectAttempts = qMax(1, maxAttempts); // 至少尝试1次
	m_autoReconnect = autoReconnect;

	qInfo() << "Reconnect settings updated - Interval:" << m_reconnectInterval
		<< "ms, Max attempts:" << m_maxReconnectAttempts
		<< ", Auto reconnect:" << m_autoReconnect;
}