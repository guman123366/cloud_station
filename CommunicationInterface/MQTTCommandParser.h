#ifndef MQTTCOMMANDPARSER_H
#define MQTTCOMMANDPARSER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include"../UAVDataTransmition\TD550CmdDefine.h"
#include<QTimer>
// 命令包结构体
struct CommandPacket1 {
    QString tid;        // 事务ID
    QString bid;        // 业务ID
    qint64 timestamp;   // 时间戳
    QString gateway;  // 设备ID
    QString method;     // 方法名
    bool need_reply;    // 是否需要回复

    int cmd_value;      // 指令值
    QString cmd_name;   // 指令名称
    QJsonObject params; // 附加参数
};

class MQTTCommandParser : public QObject
{
    Q_OBJECT
public:
    explicit MQTTCommandParser(QObject* parent = nullptr);

    // 解析MQTT命令
    bool parseFligthCommand(const QByteArray& jsonData);//开关指令
    bool parseZuheCmd(const QByteArray& jsonData);//解析组合指令
    bool parseLxCmd(const QByteArray& jsonData);
    // 记录命令日志
    void logCommand(const CommandPacket1& packet) const;
    int constructionCommandAck(unsigned char* sendBuf, const QString& method, int type, int code, const QString& msg, const QString& status, const QJsonObject& waypointInfo);
    QString KgOrderReply(unsigned short reply);		//开关指令应当
    QString YtOrderReply(unsigned char cReply);		//组合指令应答
    QVector<QVector<double>>  getCurrentWPData();
    int constructionOnline(unsigned char* sendBuf);
    int constructionHeartbeat(unsigned char* sendBuf);
    
private:
    // 初始化指令映射表
    void initCommandMap();


    // 指令映射表
    QMap<int, QString> m_commandMap;
    void initLogConfig();
    QString m_deviceSn;//飞机序列号
    QString m_deviceModel;//飞行类型
    QString m_strExePath;

signals:
    void si_sendKgCommand(unsigned char nType);			//开关指令
    void si_sendYtCommand(unsigned char nType, double dData);					//组合指令
    void si_sendLxCommand(char*, int);					//连续指令
    void si_sendZuHeCommand(unsigned char, QVector<double>);			//发送组合指令
private:
        // 解析具体指令类型
    bool parseWpInsert(const QJsonObject& data);
    bool parseWpDelete(const QJsonObject& data);
    bool parseWpModify(const QJsonObject& data);
    bool parseWpQuery(const QJsonObject& data);
    bool parseWpTaskClose(const QJsonObject& data);
    bool parseWrQuery(const QJsonObject& data);
    bool parsePointRemoteAdjustment(const QJsonObject& data);
    bool parseWrLoad(const QJsonObject& data);
    bool parseAdjustmentCommand(WaypointCommandType cmdType, const QJsonObject& data);
    bool parseLonLatAlt(WaypointCommandType cmdType, const QJsonObject& data);
    bool parseOpenControl(const QJsonObject& data);


    // 验证数据范围
   // bool validateWaypointData(const WaypointData& wpData);
    bool validateAdjustmentValue(float value, float min, float max);

    //航线装订相关
    QJsonArray pointsArray;
    int m_nWPIndex;	//未收到航点回复重复上传某个航点次数
    int m_nWPTime;			//未收到航点回复时间
    int m_nUpWPIndex;		//上传的航点编号
    bool m_bUpRouteState;	//上传航线状态
    QVector<QVector<double>> pointsVec;
};

#endif // MQTTCOMMANDPARSER_Hsssss