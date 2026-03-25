// MQTTCommunication.h
#pragma once
#include "CommunicationInterface.h"
#include"QtMqtt/qmqttclient.h"
#include "../UAVDataTransmition/LoadConfigInfo.h"
#include <QSettings>
#include<QJsonObject>
#include<QJsonDocument>
#include"MQTTCommandParser.h"
#include"../UAVDataTransmition/DataDefineInterface.h"
#include"../UAVDataTransmition/TD550TelemetryData.h"
#include<QMutex>
#include<qthread.h>

struct CommandPacket {
    
    QString tid;
    QString bid;
    qint64 timestamp;
    QString gateway;
    QString method;
    quint8 cmd_value;
    QString cmd_name;
};
struct TelemetryReply {
    QString tid;
    QString bid;
    qint64 timestamp;
    QString gateway;
    QString method;
    int result;
    QJsonObject output; // 包含具体遥测数据
    bool needReply = false;
};
// 标准状态值（参考协议）
const QString STATUS_CANCELED = "canceled";
const QString STATUS_FAILED = "failed";
const QString STATUS_IN_PROGRESS = "in_progress";
const QString STATUS_SUCCESSED = "successed";
class COMMUNICATIONINTERFACE_EXPORT MQTTCommunication : public CommunicationInterface
{
    Q_OBJECT
public:
    explicit MQTTCommunication(QObject* parent = nullptr);
    ~MQTTCommunication() override;
    // 实现基类接口
    bool openPort() override;
    bool sendData(QByteArray ary, int nLength) override;
    bool sendDataCmdAck(QByteArray ary, int nLength);
    bool closePort() override;
    void sl_recvLastTem550(TD550TelemetryData*);

    // 设置MQTT特有参数
    void setClientId(const QString& clientId);
    void setKeepAlive(int seconds);
    //mqtt遥控指令立即回复
   // void setCurrentData(QSharedPointer<DataDefineInterface> data);
    // 新增MQTT凭证管理方法
    bool getMqttCredentials();
    bool areCredentialsValid() const;
    bool updateCredentialsManually();
signals:
    void commandReceived(int cmdValue, bool needReply);
    void si_sendKgCommand(unsigned char nType);			//开关指令
    void si_sendYtCommand(unsigned char nType, double dData);					//组合指令
    void si_sendLxCommand(char*, int);					//连续指令
    void si_sendZuHeCommand(unsigned char, QVector<double>);			//发送组合指令

private:
    QMqttClient* m_client;
    QString m_clientId;
    int m_keepAlive = 60;
    QString topicSubscribe;
    QString topicPublish;
    QString topicSubscribeReplay;
    QString m_responseTopic;
    QSettings* m_pConfigFileSettings;				//加载配置文件
    QString m_gatewayId;                           // 网关ID（设备序列号）
    QString m_mqttAuthUrl;                         // MQTT认证URL
    QString m_mqttUsername;                        // MQTT用户名（动态获取）
    QString m_mqttPassword;                        // MQTT密码（动态获取）
    QString m_mqttCilentId;
    //心跳包

    QString topicHeartBeat;
    QString topicHeartBeatReply;

    QVector<QVector<double>> pointsVec;
    //bool m_waitingForOsdAck=false;  // 是否正在等待OSD响应
    QTimer* m_osdAckTimer;    // OSD响应计时器
    int m_osdTimeoutInterval; // OSD响应超时时间(毫秒)
    QString m_method;
    MQTTCommandParser* m_mqttCmdParser = nullptr;
    TD550TelemetryData* m_sendToMqttTem = nullptr;
  //  MQTTProtocol m_mqttProtocol;

  //  bool parseCommand(const QByteArray& jsonData, CommandPacket& outPacket);
    void onCommandParsed(CommandPacket& packet);
    bool parseTemReply(const QByteArray& jsonData, TelemetryReply& outReply);
    int constructionCommandAck(unsigned char* sendBuf, const QString& method, int type, int code, const QString& msg, const QString& status, int percent, const QJsonObject& waypointInfo);
    void sendAck(QString method);
    void sendTemAck(QString method);
    void parseTemAck(QString method, QByteArray recvMsg);
    void recvControlData(QString method, QByteArray recvMsg);
private slots:
    void onStateChanged(QMqttClient::ClientState state);
    void onMessageReceived(const QByteArray&message, const QMqttTopicName& topic);
    void onOsdAckTimeout();
public slots:
    void subscribeToCommands();
    void sl_recvKgCommand(unsigned char nType);			//开关指令
    void sl_recvYtCommand(unsigned char nType, double dData);					//组合指令
    void sl_recvLxCommand(char*, int);					//连续指令
    void sl_recvZuHeCommand(unsigned char, QVector<double>);			//发送组合指令
    void sl_handleImmediateSend();   // 立即发送槽函数
    void sl_sendLoadAck(QString relpy);
    //void sl_recvtest(TD550TelemetryData* var);
    void sl_updateRadioInfo(int);

signals:
    void si_osdAckTimeout(bool ack);
    void si_getLastTem();
    void si_sendVec(QVector<QVector<double>>);
    void si_updateLinkState(int type);

private:
    QTimer* m_heartbeatTimer;
    QTimer* m_heartbeatAckTimer;
    QString m_deviceSn;
    QString m_deviceModel;
    QString m_deviceVersion;
    bool m_isOnline;
    QDateTime m_lastHeartbeatTime;
    int m_heartbeatTimeoutInterval;
    int m_type = 0;
    bool m_deviceOnline;

signals:
    void heartbeatAcknowledged(long uplinkDelay);
    void heartbeatFailed(int errorCode);
    void heartbeatTimeout();
    void onlineStatusChanged(bool isOnline);

public slots:
    void sendOnlineMessage();
    void sendHeartbeat();
    void startHeartbeat();
    void stopHeartbeat();
    void handleStatusReply(const QByteArray& message, const QMqttTopicName& topic);
    void onHeartbeatAckTimeout();
    void onUdpDeviceOnlineStatusChanged(bool isOnline);
    void onUdpDataReceived();
    //mqtt断开重连技术
    // MQTTCommunication.h
private:
    QTimer* m_reconnectTimer;        // 重连定时器
    int m_reconnectInterval;         // 重连间隔
    int m_maxReconnectAttempts;      // 最大重连尝试次数
    int m_currentReconnectAttempts;  // 当前重连尝试次数
    bool m_autoReconnect;            // 是否自动重连
    void startReconnect();
    void reconnectNow();
    void setReconnectSettings(int intervalMs, int maxAttempts, bool autoReconnect);
  
private slots:
    void attemptReconnect();         // 尝试重连
    void onReconnectTimeout();       // 重连超时处理


};