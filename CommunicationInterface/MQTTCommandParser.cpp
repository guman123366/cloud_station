#include "MQTTCommandParser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QUuid>
#include<QCoreApplication>
#include<QSettings>
MQTTCommandParser::MQTTCommandParser(QObject* parent) : QObject(parent)
{
    // 初始化指令映射表
    initCommandMap();
	initLogConfig();

 
}

void MQTTCommandParser::initCommandMap()
{
  
}

void MQTTCommandParser::initLogConfig()
{
	m_strExePath = QCoreApplication::applicationDirPath();
	QString configPath = m_strExePath + "/init/Config.ini";
	QSettings settings(configPath, QSettings::IniFormat);

	// 文件上传配置
	m_deviceModel = settings.value("Device/Model", "TD550").toString();
	m_deviceSn = settings.value("Device/sn", "6722390TD550").toString();
}

bool MQTTCommandParser::parseFligthCommand(const QByteArray& jsonData)
{
   
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON format: root is not an object";
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();

    // 检查必需字段
    const QStringList requiredFields = { "tid", "bid", "timestamp", "gateway", "method", "data" };
    for (const QString& field : requiredFields) {
        if (!rootObj.contains(field)) {
            qWarning() << "Missing required field:" << field;
            return false;
        }
    }
    CommandPacket1 outPacket;

    // 提取基本信息
    outPacket.tid = rootObj["tid"].toString();
    outPacket.bid = rootObj["bid"].toString();
    outPacket.timestamp = rootObj["timestamp"].toVariant().toLongLong();
    outPacket.gateway = rootObj["gateway"].toString();
    outPacket.method = rootObj["method"].toString();
    outPacket.gateway = rootObj["gateway"].toString();
    outPacket.need_reply = rootObj["need_reply"].toInt() == 1;

    // 解析data部分
    QJsonObject dataObj = rootObj["data"].toObject();
    if (!dataObj.contains("value")) {
        qWarning() << "Missing 'value' field in data";
        return false;
    }

    bool ok;
    outPacket.cmd_value = dataObj["value"].toString().toInt(&ok, 16);
    if (!ok) {
        qWarning() << "Invalid command value format";
        return false;
    }
 
    //if (m_commandMap.contains(outPacket.cmd_value))
    //{
        emit si_sendKgCommand(outPacket.cmd_value);
   // }

    // 获取指令名称
    outPacket.cmd_name = m_commandMap.value(outPacket.cmd_value, "未知指令");

    // 如果有额外参数
    if (dataObj.contains("params")) {
        outPacket.params = dataObj["params"].toObject();
    }

    return true;
}

bool MQTTCommandParser::parseZuheCmd(const QByteArray& jsonData)
{
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON format: root is not an object";
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();

    // 检查必需字段
    const QStringList requiredFields = { "tid", "bid", "timestamp", "gateway", "method", "data" };
    for (const QString& field : requiredFields) {
        if (!rootObj.contains(field)) {
            qWarning() << "Missing required field:" << field;
            return false;
        }
    }

    QString method = rootObj["method"].toString();
    QJsonObject dataObj = rootObj["data"].toObject();

   
    // 根据不同的method类型进行解析
    if (method == "wp_insert")  //航点插入
      
    {
        return parseWpInsert(dataObj);
    }
    else if (method == "wp_delete") //航点删除
    {
        return parseWpDelete(dataObj);
    }
    else if (method == "wp_modify") //航点修改
    {
        return parseWpModify(dataObj);
    }
    else if (method == "wp_query") //航点查询
    {
        return parseWpQuery(dataObj);
    }
    else if (method == "wp_task_close") //航点任务关闭
    {
        return parseWpTaskClose(dataObj);
    }
    else if (method == "wr_query") //航线查询
    {
        return parseWrQuery(dataObj);
    }
    else if (method == "point_remote_adjustment")//点号遥调 
    {
        return parsePointRemoteAdjustment(dataObj);
    }
    else if (method == "wr_load")//航线装订
    {
        return parseWrLoad(dataObj);
    }
    else if (method == "longitudinal_pos_remote_adjustment") //纵向位置遥调
    {
        return parseAdjustmentCommand(WaypointCommandType::LONGITUDINAL_POS_ADJUSTMENT,dataObj);
    }
    else if (method == "lateral_pos_remote_adjustment")//侧向位置遥调
    {
        return parseAdjustmentCommand(LATERAL_POS_ADJUSTMENT,dataObj);
    }
    else if (method == "alt_remote_adjustment") //高度遥调给定
    {
        return parseAdjustmentCommand(ALT_ADJUSTMENT,dataObj);
    }
    else if (method == "yaw_remote_adjustment")//航向遥调给定
    {
        return parseAdjustmentCommand(YAW_ADJUSTMENT, dataObj);
    }
    else if (method == "longitudinal_speed_remote_adjustment") //纵向速度遥调给定
    {
        return parseAdjustmentCommand(LONGITUDINAL_SPEED_ADJUSTMENT, dataObj);
    }
    else if (method == "vertical_speed_remote_adjustment")//垂直速度遥调给定
    {
        return parseAdjustmentCommand(VERTICAL_SPEED_ADJUSTMENT, dataObj);
    }
    else if (method == "lateral_speed_remote_adjustment") //侧向速度遥调给定
    {
        return parseAdjustmentCommand(LATERAL_SPEED_ADJUSTMENT,dataObj);
    }
    else if (method == "pos_deviation_injection") //位置偏差注入（预留）
    {
        return parseAdjustmentCommand(LATERAL_SPEED_ADJUSTMENT, dataObj);
    }
    else if (method == "height_injection") //场高注入
    {
        return parseAdjustmentCommand(LATERAL_SPEED_ADJUSTMENT, dataObj);
    }
    else if (method == "magnetic_injection") //磁偏角注入（预留）
    {
        return parseAdjustmentCommand(LATERAL_SPEED_ADJUSTMENT, dataObj);
    }
    else if (method == "magnetic_injection") //飞机起飞重量装订
    {
        return parseAdjustmentCommand(LATERAL_SPEED_ADJUSTMENT, dataObj);
    }
    else if (method == "a_coordinate_load") //a点坐标装订
    {
        return parseLonLatAlt(A_COORDINATE_LOAD, dataObj);
    }
    else if (method == "b_coordinate_load") //b点坐标装订
    {
        return parseLonLatAlt(B_COORDINATE_LOAD, dataObj);
    }
    else if (method == "c_coordinate_load") //c点坐标装订
    {
        return parseLonLatAlt(C_COORDINATE_LOAD, dataObj);
    }
    else if (method == "alternate_point1") //备降点1
    {
        return parseLonLatAlt(ALTERNATE_POINT1, dataObj);
    }
    else if (method == "alternate_point2") //备降点2
    {
        return parseLonLatAlt(ALTERNATE_POINT2, dataObj);
    }
    else if (method == "alternate_point3") //备降点1
    {
        return parseLonLatAlt(ALTERNATE_POINT3, dataObj);
    }
    else if (method == "alternate_point4") //备降点2
    {
        return parseLonLatAlt(ALTERNATE_POINT4, dataObj);
    }
    
    else 
    {
        qWarning() << "Unknown method type:" << method;
        return false;
    }
}
bool MQTTCommandParser::parseLxCmd(const QByteArray& jsonData)
{
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON format: root is not an object";
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();

    // 检查必需字段
    const QStringList requiredFields = { "tid", "bid", "timestamp", "gateway", "method", "data" };
    for (const QString& field : requiredFields) {
        if (!rootObj.contains(field)) {
            qWarning() << "Missing required field:" << field;
            return false;
        }
    }

    QString method = rootObj["method"].toString();
    QJsonObject dataObj = rootObj["data"].toObject();
    if (method == "damper_opening_control")  //航点插入

    {
        return parseOpenControl(dataObj);
    }

}
bool MQTTCommandParser::parseWpInsert(const QJsonObject& data)
{
    const QStringList requiredFields = { "wr_num", "wp_num", "wp_type", "wp_lon",
                                     "wp_lat", "wp_alt", "wp_speed", "wo_time" };

    for (const QString& field : requiredFields) {
        if (!data.contains(field)) {
            qWarning() << "wp_insert missing field:" << field;
            return false;
        }
    }

    // 验证数据范围
    int wr_num = data["wr_num"].toInt();
    if (wr_num < 1 || wr_num > 20) {
        qWarning() << "Invalid wr_num:" << wr_num;
        return false;
    }

    int wp_num = data["wp_num"].toInt();
    if (wp_num < 1 || wp_num > 150) {
        qWarning() << "Invalid wp_num:" << wp_num;
        return false;
    }

    int wp_type = data["wp_type"].toInt();
    if (wp_type < 0 || wp_type > 10) {
        qWarning() << "Invalid wp_type:" << wp_type;
        return false;
    }

    double wp_lon = data["wp_lon"].toDouble();
    if (wp_lon < -180 || wp_lon > 180) {
        qWarning() << "Invalid wp_lon:" << wp_lon;
        return false;
    }

    double wp_lat = data["wp_lat"].toDouble();
    if (wp_lat < -90 || wp_lat > 90) {
        qWarning() << "Invalid wp_lat:" << wp_lat;
        return false;
    }

    float wp_alt = data["wp_alt"].toDouble();
    if (wp_alt < 0 || wp_alt > 6000) {
        qWarning() << "Invalid wp_alt:" << wp_alt;
        return false;
    }

    float wp_speed = data["wp_speed"].toDouble();
    if (wp_speed < 0 || wp_speed > 300) {
        qWarning() << "Invalid wp_speed:" << wp_speed;
        return false;
    }

    int wo_time = data["wo_time"].toInt();
    if (wo_time < 0 || wo_time > 65535) {
        qWarning() << "Invalid wo_time:" << wo_time;
        return false;
    }

    //qDebug() << "Valid wp_insert command received";
    return true;

}

void MQTTCommandParser::logCommand(const CommandPacket1& packet) const
{
    QString timeStr = QDateTime::fromMSecsSinceEpoch(packet.timestamp)
        .toString("yyyy-MM-dd hh:mm:ss.zzz");

   /* qDebug().noquote() << "\n======= 飞行开关指令 =======";
    qDebug().noquote() << "设备ID:" << packet.gateway;
    qDebug().noquote() << "事务ID:" << packet.tid;
    qDebug().noquote() << "时间戳:" << timeStr;
    qDebug().noquote() << "指令码:" << QString("0x%1").arg(packet.cmd_value, 2, 16, QChar('0')).toUpper();
    qDebug().noquote() << "指令名称:" << packet.cmd_name;*/

    if (!packet.params.isEmpty()) {
        //qDebug().noquote() << "附加参数:";
        //qDebug().noquote() << QJsonDocument(packet.params).toJson(QJsonDocument::Indented);
    }

    //qDebug().noquote() << "==========================\n";
}

int MQTTCommandParser::constructionCommandAck(unsigned char* sendBuf, const QString& method, int type, int code, const QString& msg, const QString& status, const QJsonObject& waypointInfo)
{
    // 构造根JSON对象
    QJsonObject rootObject;
    rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096aa");
    rootObject.insert("bid", "42a19f36-5117-4520-bd13-fd61d818d712");
    rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
    rootObject.insert("gateway", m_deviceSn);
    rootObject.insert("method", method); // 保持与原指 令相同的method

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

QString MQTTCommandParser::KgOrderReply(unsigned short reply)
{
	QString strReplyText = "";
	unsigned short temp = reply;

	switch (temp)
	{
	case 0x0101:
		strReplyText =  u8"遥控指令接收成功";
		break;
	case 0x0102:
		strReplyText =  u8"遥控指令接收失败";
		break;
	case 0x0103:
		strReplyText =  u8"混控指令接收成功";
		break;
	case 0x0104:
		strReplyText =  u8"混控指令接收败";
		break;
	case 0x0105:
		strReplyText =  u8"程控指令接收成功";
		break;
	case 0x0106:
		strReplyText = u8"程控指令接收失败";
		break;
	case 0x0107:
		strReplyText = u8"开车指令接收成功";
		break;
	case 0x0108:
		strReplyText = u8"开车指令接收失败";
		break;
	case 0x0109:
		strReplyText = u8"怠速指令接收成功";
		break;
	case 0x010A:
		strReplyText = u8"怠速指令接收失败";
		break;
	case 0x010B:
		strReplyText = u8"暖车指令接收成功";
		break;
	case 0x010C:
		strReplyText = u8"暖车指令接收失败";
		break;
	case 0x010D:
		strReplyText = u8"额定指令接收成功";
		break;
	case 0x010E:
		strReplyText = u8"额定指令接收失败";
		break;
	case 0x010F:
		strReplyText = u8"停车指令接收成功";
		break;
	case 0x0110:
		strReplyText = u8"停车指令接收失败";
		break;
	case 0x0111:
		strReplyText = u8"绝对高度保持与给定指令接收成功";
		break;
	case 0x0112:
		strReplyText = u8"绝对高度保持与给定指令接收失败";
		break;
	case 0x0113:
		strReplyText = u8"空速保持与给定指令接收成功";
		break;
	case 0x0114:
		strReplyText = u8"空速保持与给定指令接收失败";
		break;
	case 0x0115:
		strReplyText = u8"相对高度保持与给定指令接收成功";
		break;
	case 0x0116:
		strReplyText = u8"相对高度保持与给定指令接收失败";
		break;
	case 0x0117:
		strReplyText = u8"地速保持与给定指令接收成功";
		break;
	case 0x0118:
		strReplyText = u8"地速保持与给定指令接收失败";
		break;
	case 0x0119:
		strReplyText = u8"位置保持与给定指令接收成功";
		break;
	case 0x011A:
		strReplyText = u8"位置保持与给定指令接收失败";
		break;
	case 0x011B:
		strReplyText = u8"垂直速度保持与给定指令接收成功";
		break;
	case 0x011C:
		strReplyText = u8"垂直速度保持与给定指令接收失败";
		break;
	case 0x011D:
		strReplyText = u8"航向保持与给定指令接收成功";
		break;
	case 0x011E:
		strReplyText = u8"航向保持与给定指令接收失败";
		break;
	case 0x011F:
		strReplyText = u8"自动导航指令接收成功";
		break;
	case 0x0120:
		strReplyText = u8"自动导航指令接收失败";
		break;
	case 0x0121:
		strReplyText = u8"自主起飞指令接收成功";
		break;
	case 0x0122:
		strReplyText = u8"自主起飞指令接收失败";
		break;
	case 0x0123:
		strReplyText = u8"自主着陆指令接收成功";
		break;
	case 0x0124:
		strReplyText = u8"自主着陆指令接收失败";
		break;
	case 0x0125:
		strReplyText = u8"自主悬停指令接收成功";
		break;
	case 0x0126:
		strReplyText = u8"自主悬停指令接收失败";
		break;
	case 0x0127:
		strReplyText = u8"直线归航指令接收成功";
		break;
	case 0x0128:
		strReplyText = u8"直线归航指令接收失败";
		break;
	case 0x0129:
		strReplyText = u8"原路归航指令接收成功";
		break;
	case 0x012A:
		strReplyText = u8"原路归航指令接收失败";
		break;
	case 0x012B:
		strReplyText = u8"直线归航2指令接收成功";
		break;
	case 0x012C:
		strReplyText = u8"直线归航2指令接收失败";
		break;
	case 0x012D:
		strReplyText = u8"断链路应急返航指令接收成功";
		break;
	case 0x012E:
		strReplyText = u8"断链路应急返航指令接收失败";
		break;
	case 0x012F:
		strReplyText = u8"左盘旋指令接收成功";
		break;
	case 0x0130:
		strReplyText = u8"左盘旋指令接收失败";
		break;
	case 0x0131:
		strReplyText = u8"右盘旋指令接收成功";
		break;
	case 0x0132:
		strReplyText = u8"右盘旋指令接收失败";
		break;
	case 0x0133:
		strReplyText = u8"吊挂飞行指令接收成功";
		break;
	case 0x0134:
		strReplyText = u8"吊挂飞行指令接收失败";
		break;
	case 0x0135:
		strReplyText = u8"定点飞行指令接收成功";
		break;
	case 0x0136:
		strReplyText = u8"定点飞行指令接收失败";
		break;
	case 0x0137:
		strReplyText = u8"自主避障指令接收成功";
		break;
	case 0x0138:
		strReplyText = u8"自主避障指令接收失败";
		break;
	case 0x0139:
		strReplyText = u8"退出机动指令接收成功";
		break;
	case 0x013A:
		strReplyText = u8"退出机动指令接收失败";
		break;
	case 0x013B:
		strReplyText = u8"解除高度保持指令接收成功";
		break;
	case 0x013C:
		strReplyText = u8"解除高度保持指令接收失败";
		break;
	case 0x0141:
		strReplyText = u8"解除定向指令接收成功";
		break;
	case 0x0142:
		strReplyText = u8"解除定向指令接收成功";
		break;
	case 0x013D:
		strReplyText = u8"解除速度指令接收成功";
		break;
	case 0x013E:
		strReplyText = u8"解除速度指令接收成功";
		break;
	case 0x013F:
		strReplyText = u8"解除位置指令接收成功";
		break;
	case 0x0140:
		strReplyText = u8"解除位置指令接收失败";
		break;
	case 0x0143:
		strReplyText = u8"解除导航指令接收成功";
		break;
	case 0x0144:
		strReplyText = u8"解除导航指令接收失败";
		break;
	case 0x0145:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车1接通指令接收成功";
		//}
		//else
		//{
		//	strReplyText = u8 "点火A接通指令接收成功";
		//}
		strReplyText = u8"着车1接通指令接收成功";
		break;
	case 0x0146:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车1接通指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8 "点火A接通指令接收失败";
		//}
		strReplyText = u8"着车1接通指令接收失败";
		break;
	case 0x0147:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车1断开指令接收成功";
		//}
		//else
		//{
		//	strReplyText = u8 "点火A断开指令接收成功";
		//}
		strReplyText = u8"着车1断开指令接收成功";
		break;
	case 0x0148:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车1断开指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8 "点火A断开指令接收失败";
		//}
		strReplyText = u8"着车1断开指令接收失败";
		break;
	case 0x0149:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车2接通指令接收成功";
		//}
		//else
		//{
		//	strReplyText = u8 "点火B接通指令接收成功";
		//}
		strReplyText = u8"着车2接通指令接收成功";
		break;
	case 0x014A:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车2接通指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8 "点火B接通指令接收失败";
		//}
		strReplyText = u8"着车2接通指令接收失败";
		break;
	case 0x014B:
		/*	if ("" == "145")
			{
				strReplyText = u8 "着车2指令断开接收成功";
			}
			else
			{
				strReplyText = u8 "点火B指令断开接收成功";
			}*/
		strReplyText = u8"着车2指令断开接收成功";
		break;
	case 0x014C:
		//if ("" == "145")
		//{
		//	strReplyText = u8 "着车2指令断开接收失败";
		//}
		//else
		//{
		//	strReplyText = u8 "点火B指令断开接收失败";
		//}
		strReplyText = u8"着车2指令断开接收失败";
		break;
	case 0x014D:
		strReplyText = u8"发电机接通指令接收成功";
		break;
	case 0x014E:
		strReplyText = u8"发电机接通指令接收失败";
		break;
	case 0x014F:
		strReplyText = u8"发电机断开指令接收成功";
		break;
	case 0x0150:
		strReplyText = u8"发电机断开指令接收失败";
		break;
	case 0x0151:
		strReplyText = u8"离合器接合指令接收成功";
		break;
	case 0x0152:
		strReplyText = u8"离合器接合指令接收失败";
		break;
	case 0x0153:
		strReplyText = u8"离合器分离指令接收成功";
		break;
	case 0x0154:
		strReplyText = u8"离合器分离指令接收失败";
		break;
	case 0x016F:
		/*if ("" == "145")
		{
			strReplyText = u8"主油泵接通指令接收成功";
		}
		else
		{
			strReplyText = u8"油泵A接通指令接收成功";
		}*/
		strReplyText = u8"主油泵接通指令接收成功";
		break;
	case 0x0170:
		//if ("" == "145")
		//{
		//	strReplyText = u8"主油泵接通指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8"油泵A接通指令接收失败";
		//}
		strReplyText = u8"主油泵接通指令接收失败";
		break;
	case 0x0171:
		//if ("" == "145")
		//{
		//	strReplyText = u8"主油泵断开指令接收成功";
		//}
		//else
		//{
		//	strReplyText = u8"油泵A断开指令接收成功";
		//}
		strReplyText = u8"主油泵断开指令接收成功";
		break;
	case 0x0172:
		//if ("" == "145")
		//{
		//	strReplyText = u8"主油泵断开指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8"油泵A断开指令接收失败";
		//}
		strReplyText = u8"主油泵断开指令接收失败";
		break;
	case 0x0155:
		//if ("" == "145")
		//{
		//	strReplyText = u8"副油泵接通指令接收成功";
		//}
		//else
		//{
		//	strReplyText = u8"油泵B接通指令接收成功";
		//}
		strReplyText = u8"副油泵接通指令接收成功";
		break;
	case 0x0156:
		//if ("" == "145")
		//{
		//	strReplyText = u8"副油泵接通指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8"油泵B接通指令接收失败";
		//}
		strReplyText = u8"副油泵接通指令接收失败";
		break;
	case 0x0157:
		//if ("" == "145")
		//{
		//	strReplyText = u8"副油泵断开指令接收成功";
		//}
		//else
		//{
		//	strReplyText = u8"油泵B断开指令接收成功";
		//}
		strReplyText = u8"副油泵断开指令接收成功";
		break;
	case 0x0158:
		//if ("" == "145")
		//{
		//	strReplyText = u8"副油泵断开指令接收失败";
		//}
		//else
		//{
		//	strReplyText = u8"油泵B断开指令接收失败";
		//}
		strReplyText = u8"副油泵断开指令接收失败";
		break;
	case 0x0159:
		strReplyText = u8"电源接通指令接收成功";
		break;
	case 0x015A:
		strReplyText = u8"电源接通指令接收失败";
		break;
	case 0x015B:
		strReplyText = u8"电源断开指令接收成功";
		break;
	case 0x015C:
		strReplyText = u8"电源断开指令接收失败";
		break;
	case 0x0173:
		strReplyText = u8"水泵接通指令接收成功";
		break;
	case 0x0174:
		strReplyText = u8"水泵接通指令接收失败";
		break;
	case 0x0175:
		strReplyText = u8"水泵关闭指令接收成功";
		break;
	case 0x0176:
		strReplyText = u8"水泵关闭指令接收失败";
		break;
	case 0x0177:
		strReplyText = u8"投放开指令接收成功";
		break;
	case 0x0178:
		strReplyText = u8"投放开指令接收失败";
		break;
	case 0x0179:
		strReplyText = u8"投放关指令接收成功";
		break;
	case 0x017A:
		strReplyText = u8"投放关指令接收失败";
		break;
	case 0x017B:
		strReplyText = u8"吊舱接通指令接收成功";
		break;
	case 0x017C:
		strReplyText = u8"吊舱接通指令接收失败";
		break;
	case 0x017D:
		strReplyText = u8"吊舱断开指令接收成功";
		break;
	case 0x017E:
		strReplyText = u8"吊舱断开指令接收失败";
		break;
	case 0x015D:
		strReplyText = u8"点火检查打开接收成功";
		break;
	case 0x015E:
		strReplyText = u8"点火检查打开接收失败";
		break;
	case 0x015F:
		strReplyText = u8"点火检查关闭接收成功";
		break;
	case 0x0160:
		strReplyText = u8"点火检查关闭接收成功";
		break;
	case 0x0161:
		strReplyText = u8"纵向位置遥调接收成功";
		break;
	case 0x0162:
		strReplyText = u8"纵向位置遥调接收失败";
		break;
	case 0x0163:
		strReplyText = u8"横向位置遥调接收成功";
		break;
	case 0x0164:
		strReplyText = u8"横向位置遥调接收失败";
		break;
	case 0x0165:
		strReplyText = u8"高度遥调接收成功";
		break;
	case 0x0166:
		strReplyText = u8"高度遥调接收失败";
		break;
	case 0x0167:
		strReplyText = u8"航向遥调接收成功";
		break;
	case 0x0168:
		strReplyText = u8"航向遥调接收失败";
		break;
	case 0x0169:
		strReplyText = u8"纵向速度遥调接收成功";
		break;
	case 0x016A:
		strReplyText = u8"纵向速度遥调接收失败";
		break;
	case 0x016B:
		strReplyText = u8"垂速遥调接收成功";
		break;
	case 0x016C:
		strReplyText = u8"垂速遥调接收成功";
		break;
	case 0x0185:
		strReplyText = u8"发动机散热接通接收成功";
		break;
	case 0x0186:
		strReplyText = u8"发动机散热接通接收失败";
		break;
	case 0x0187:
		strReplyText = u8"发动机散热断开接收成功";
		break;
	case 0x0188:
		strReplyText = u8"发动机散热断开接收失败";
		break;
	case 0x0189:
		strReplyText = u8"侧向速度遥调接收成功";
		break;
	case 0x018A:
		strReplyText = u8"侧向速度遥调接收成功";
		break;
	case 0x018B:
		strReplyText = u8"A点坐标装订成功";
		break;
	case 0x018C:
		strReplyText = u8"A点坐标装订失败";
		break;
	case 0x018D:
		strReplyText = u8"B点坐标装订成功";
		break;
	case 0x018E:
		strReplyText = u8"B点坐标装订失败";
		break;
	case 0x018F:
		strReplyText = u8"C点坐标装订成功";
		break;
	case 0x0190:
		strReplyText = u8"C点坐标装订失败";
		break;
	case 0x0191:
		strReplyText = u8"点号遥调成功";
		break;
	case 0x0192:
		strReplyText = u8"点号遥调失败";
		break;
	case 0x0193:
		strReplyText = u8"空速限制允许成功";
		break;
	case 0x0194:
		strReplyText = u8"空速限制允许失败";
		break;
	case 0x0195:
		strReplyText = u8"空速限制禁止成功";
		break;
	case 0x0196:
		strReplyText = u8"空速限制禁止失败";
		break;
	case 0x0203:
		strReplyText = u8"非失控迫降执行接收成功";
		break;
	case 0x0204:
		strReplyText = u8"非失控迫降执行接收失败";
		break;
	case 0x0205:
		strReplyText = u8"非失控迫降不执行接收成功";
		break;
	case 0x0206:
		strReplyText = u8"非失控迫降不执行接收失败";
		break;
	case 0x0207:
		strReplyText = u8"迫降点1装订接收成功";
		break;
	case 0x0208:
		strReplyText = u8"迫降点1装订接收失败";
		break;
	case 0x0209:
		strReplyText = u8"迫降点2装订接收成功";
		break;
	case 0x020A:
		strReplyText = u8"迫降点2装订接收失败";
		break;
	case 0x020B:
		strReplyText = u8"迫降点3装订接收成功";

		break;
	case 0x020C:
		strReplyText = u8"迫降点3装订接收失败";
		break;
	case 0x020D:
		strReplyText = u8"迫降点4装订接收成功";
		break;
	case 0x020E:
		strReplyText = u8"迫降点4装订接收失败";
		break;
	case 0x0246:
		strReplyText = u8"导航切换指令接收成功";
		break;
	case 0x0247:
		strReplyText = u8"导航切换指令接收失败";
		break;
	default:
		break;
	}

	return strReplyText;
}

QString MQTTCommandParser::YtOrderReply(unsigned char cReply)
{
	return QString();
}

QVector<QVector<double>> MQTTCommandParser::getCurrentWPData()
{
	if (pointsVec.size() >= 5)
	{
		return pointsVec;
	}
}

int MQTTCommandParser::constructionOnline(unsigned char* sendBuf)
{
	// 构造根JSON对象
	QJsonObject rootObject;
	rootObject.insert("tid","6a7bfe89-c386-4043-b600-b518e10196aa");
	rootObject.insert("bid", "6a7bfe89-c386-4043-b600-b518e12096aa");
	rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	rootObject.insert("gateway", m_deviceSn);
	rootObject.insert("method", "online");
	rootObject.insert("need_reply", 1);
	rootObject.insert("source", "station");
	// 数据段
	QJsonObject dataObj;
	dataObj.insert("model", m_deviceModel);
	dataObj.insert("version", "V1.0");

	rootObject.insert("data", dataObj);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "Online MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}

int MQTTCommandParser::constructionHeartbeat(unsigned char* sendBuf)
{
	// 构造根JSON对象
	QJsonObject rootObject;
	rootObject.insert("tid", "6a7bfe89-c386-4043-b600-b518e10096ab");
	rootObject.insert("bid", "6a7bfe89-c386-4043-b600-b518e10096ac");
	rootObject.insert("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
	rootObject.insert("gateway", m_deviceSn);
	rootObject.insert("method", "heartbeat");
	rootObject.insert("need_reply", 1);
	rootObject.insert("source", "station");
	// 数据段
	QJsonObject dataObj;
	dataObj.insert("model", m_deviceModel);
	dataObj.insert("version", "V1.0");

	rootObject.insert("data", dataObj);

	// 序列化和传输
	QByteArray byteArray = QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
	memcpy(sendBuf, byteArray.constData(), byteArray.size());

	//qDebug().noquote() << "Online MQTT Data:" << QJsonDocument(rootObject).toJson(QJsonDocument::Indented);
	return byteArray.size();
}




bool MQTTCommandParser::parseWpDelete(const QJsonObject& data)
{
    
    // 检查必需字段
    if (!data.contains("wr_num") || !data.contains("wp_num")) {
        qWarning() << "wp_delete missing required fields";
        return false;
    }

    int wr_num = data["wr_num"].toInt();
    int wp_num = data["wp_num"].toInt();

    // 验证数据范围
    if (wr_num < 1 || wr_num > 20) {
        qWarning() << "Invalid wr_num:" << wr_num;
        return false;
    }

    if (wp_num < 1 || wp_num > 150) {
        qWarning() << "Invalid wp_num:" << wp_num;
        return false;
    }


    QVector<double> vecData;
   // vecData.append(value);
    emit si_sendZuHeCommand(WaypointCommandType::WP_DELETE, vecData);

 
}

bool MQTTCommandParser::parseWpModify(const QJsonObject& data)
{
    WaypointData wpData;

    // 提取数据
    wpData.wr_num = data["wr_num"].toInt();
    wpData.wp_num = data["wp_num"].toInt();
    wpData.wp_type = data["wp_type"].toInt();
    wpData.wp_lon = data["wp_lon"].toDouble();
    wpData.wp_lat = data["wp_lat"].toDouble();
    wpData.wp_alt = data["wp_alt"].toDouble();
    wpData.wp_speed = data["wp_speed"].toDouble();
    wpData.wp_time = data["wo_time"].toInt();

    QVector<double> vecData;
    // vecData.append(value);
    emit si_sendZuHeCommand(WaypointCommandType::WP_MODIFY, vecData);

   

    return true;
}

bool MQTTCommandParser::parseWpQuery(const QJsonObject& data)
{
    // 检查必需字段
    if (!data.contains("wr_num") || !data.contains("wp_num")) {
        qWarning() << "wp_query missing required fields";
        return false;
    }

    int wr_num = data["wr_num"].toInt();
    int wp_num = data["wp_num"].toInt();

    // 验证数据范围
    if (wr_num < 1 || wr_num > 20) {
        qWarning() << "Invalid wr_num:" << wr_num;
        return false;
    }

    if (wp_num < 1 || wp_num > 150) {
        qWarning() << "Invalid wp_num:" << wp_num;
        return false;
    }

    QVector<double> vecData;
     vecData.append(wr_num);
     vecData.append(wp_num);
     
    emit si_sendZuHeCommand(WaypointCommandType::WP_QUERY, vecData);

    return true;
}

bool MQTTCommandParser::parseWpTaskClose(const QJsonObject& data)
{
    QVector<double> vecData;


    emit si_sendZuHeCommand(WaypointCommandType::WP_TASK_CLOSE, vecData);
    return true;
}

bool MQTTCommandParser::parseWrQuery(const QJsonObject& data)
{
    QVector<double> vecData;


    emit si_sendZuHeCommand(WaypointCommandType::WR_QUERY, vecData);
    return true;

}

bool MQTTCommandParser::parsePointRemoteAdjustment(const QJsonObject& data)
{
    //检查必需字段
        if (!data.contains("wr_num") || !data.contains("wp_num")) {
            qWarning() << "point_remote_adjustment missing required fields";
            return false;
        }

    int wr_num = data["wr_num"].toInt();
    int wp_num = data["wp_num"].toInt();

    // 验证数据范围
    if (wr_num < 1 || wr_num > 20) {
        qWarning() << "Invalid wr_num:" << wr_num;
        return false;
    }

    if (wp_num < 1 || wp_num > 150) {
        qWarning() << "Invalid wp_num:" << wp_num;
        return false;
    }
    QVector<double> vecData;
    vecData.append(wr_num);
    vecData.append(wp_num);

    emit si_sendZuHeCommand(WaypointCommandType::POINT_REMOTE_ADJUSTMENT, vecData);

    return true;
   

   
}

bool MQTTCommandParser::parseWrLoad(const QJsonObject& data)
{
	pointsVec.clear();
	// 1. 验证必需字段
	if (!data.contains("wr_num") || !data.contains("wr_points")) {
		qWarning() << "Missing required fields in wr_load command";
		return false;
	}

	// 2. 提取航线基本信息
	int wr_num = data["wr_num"].toInt();
	if (wr_num < 1 || wr_num > 20) {
		qWarning() << "Invalid route number:" << wr_num << "(valid range:1-20)";
		return false;
	}

	// 3. 处理航点数组
	QJsonArray pointsArray = data["wr_points"].toArray();
	if (pointsArray.isEmpty()) {
		qWarning() << "Empty waypoints array";
		return false;
	}

	// 4. 准备发送数据

	//vecData.append(pointsArray.size()); // 第二个元素是航点数量

	// 5. 遍历并验证每个航点
	for (const QJsonValue& pointValue : pointsArray) {
		QJsonObject pointObj = pointValue.toObject();

		// 验证航点必需字段
		if (!pointObj.contains("wp_num") || !pointObj.contains("wp_type") ||
			!pointObj.contains("wp_lon") || !pointObj.contains("wp_lat") ||
			!pointObj.contains("wp_alt") || !pointObj.contains("wp_speed") ||
			!pointObj.contains("wp_time")) {
			qWarning() << "Missing required fields in waypoint object";
			return false;
		}

		// 提取航点数据
		int wp_num = pointObj["wp_num"].toInt();
		int wp_type = pointObj["wp_type"].toInt();
		double wp_lon = pointObj["wp_lon"].toDouble();
		double wp_lat = pointObj["wp_lat"].toDouble();
		double wp_alt = pointObj["wp_alt"].toDouble();
		double wp_speed = pointObj["wp_speed"].toDouble();
		int wp_time = pointObj["wp_time"].toInt();

		// 验证航点数据范围
		if (wp_num < 1 || wp_num > 150) {
			qWarning() << "Invalid waypoint number:" << wp_num << "(valid range:1-150)";
			return false;
		}
		if (wp_type < 0 || wp_type > 10) {
			qWarning() << "Invalid waypoint type:" << wp_type << "(valid range:0-10)";
			return false;
		}
		if (wp_lon < -180.0 || wp_lon > 180.0) {
			qWarning() << "Invalid longitude:" << wp_lon << "(valid range:±180°)";
			return false;
		}
		if (wp_lat < -90.0 || wp_lat > 90.0) {
			qWarning() << "Invalid latitude:" << wp_lat << "(valid range:±90°)";
			return false;
		}
		if (wp_alt < 0.0 || wp_alt > 6000.0) {
			qWarning() << "Invalid altitude:" << wp_alt << "(valid range:0-6000m)";
			return false;
		}
		if (wp_speed < 0.0 || wp_speed > 300.0) {
			qWarning() << "Invalid speed:" << wp_speed << "(valid range:0-300km/h)";
			return false;
		}
		if (wp_time < 0 || wp_time > 65535) {
			qWarning() << "Invalid time:" << wp_time << "(valid range:0-65535s)";
			return false;
		}
		QVector<double> vecData;
		vecData.append(wr_num); // 第一个元素是航线编号
		// 添加航点数据到向量
		vecData.append(wp_num);
		vecData.append(wp_type);
		vecData.append(wp_lon);
		vecData.append(wp_lat);
		vecData.append(wp_alt);
		vecData.append(wp_speed);
		vecData.append(wp_time);
		m_bUpRouteState = true;
		m_nUpWPIndex = 0;
		m_nWPIndex++;
		pointsVec.append(vecData);
	}

	// 6. 发送组合指令
	//emit si_sendZuHeCommand(WaypointCommandType::WR_LOAD, pointsVec[0]);
	return true;

}

bool MQTTCommandParser::parseAdjustmentCommand(WaypointCommandType cmdType, const QJsonObject& data)
{
    if (!data.contains("value")) {
        qWarning() << "Adjustment command missing value field";
        return false;
    }

    double value = data["value"].toDouble();
    float min = 0, max = 0;

    // 设置不同指令的值范围
  /*  switch (cmdType) 
    {
    case LONGITUDINAL_POS_ADJUSTMENT:
    case LATERAL_POS_ADJUSTMENT:
        min = -100; max = 100;
        break;
    case ALT_ADJUSTMENT:
        min = 0; max = 6500;
        break;
    case YAW_ADJUSTMENT:
        min = -180; max = 180;
        break;
    case LONGITUDINAL_SPEED_ADJUSTMENT:
        min = -12; max = 50;
        break;
    case VERTICAL_SPEED_ADJUSTMENT:
        min = -3; max = 5;
        break;
    case LATERAL_SPEED_ADJUSTMENT:
        min = -12; max = 12;
        break;
    default:
        break;
    }*/

    QVector<double> vecData;
    vecData.append(value);
    emit si_sendZuHeCommand(cmdType, vecData);

    return true;
}

bool MQTTCommandParser::parseLonLatAlt(WaypointCommandType cmdType, const QJsonObject& data)
{
    // 检查必需字段
    const QStringList requiredFields = { "lon", "lat", "alt" };
    for (const auto& field : requiredFields) {
        if (!data.contains(field)) {
            qWarning() << "Coordinate command missing" << field << "field";
            return false;
        }
    }

    // 提取并验证经度
    double lon = data["lon"].toDouble();
    if (lon < -180.0 || lon > 180.0) {
        qWarning() << "Invalid longitude value:" << lon << "(valid range: -180.0 ~ 180.0)";
        return false;
    }

    // 提取并验证纬度
    double lat = data["lat"].toDouble();
    if (lat < -90.0 || lat > 90.0) {
        qWarning() << "Invalid latitude value:" << lat << "(valid range: -90.0 ~ 90.0)";
        return false;
    }

    // 提取并验证高度
    double alt = data["alt"].toDouble();
    if (alt < 0 || alt > 6000) {
        qWarning() << "Invalid altitude value:" << alt << "(valid range: 0 ~ 6000)";
        return false;
    }

    // 转换为二进制协议需要的格式
    QVector<double> vecData;
    vecData.append(lon);  // 经度
    vecData.append(lat);  // 纬度
    vecData.append(alt);  // 高度

    // 根据当前命令类型发送数据
    switch (cmdType) {

        emit si_sendZuHeCommand(cmdType, vecData);
        return true;
    default:
        qWarning() << "Current command type not a coordinate command:" << cmdType;
        return false;
    }
}

bool MQTTCommandParser::parseOpenControl(const QJsonObject& data)
{
    if (!data.contains("value")) {
        qWarning() << "Adjustment command missing value field";
        return false;
    }

    double value = data["value"].toDouble();
    char cData[6] = { 0 };
    cData[4] = value;

    emit si_sendLxCommand(cData, 6);
}
bool MQTTCommandParser::validateAdjustmentValue(float value, float min, float max)
{
    return false;
}

