// MQTTProtocol.h
#pragma once
#include "ConstructionProtocolInterface.h"
#include "TD550TelemetryData.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include<QFile>
#include<QSettings>

#include<QCoreApplication>
class MQTTProtocol : public ConstructionProtocolInterface
{
    Q_OBJECT
public:
    explicit MQTTProtocol(QObject* parent = nullptr);
    ~MQTTProtocol() override;

    int ConstructionData(unsigned char* sendBuf) override;
    int constructionMemsData(unsigned char* sendBuf);//mems数据 
   // int constructionEngine(unsigned char* sendBuf);//发动机
    int constructionRemotecontrol(unsigned char* sendBuf);//遥控指令
    int constructionPower(unsigned char* sendBuf);//电源状态
    int constructionDiscrete(unsigned char* sendBuf);//离散状态
    int constructionDrivetrain(unsigned char* sendBuf);//传动数据
    int constructionFMS(unsigned char* sendBuf);//飞行管理
    int constructionGuidanceLaw(unsigned char* sendBuf);//制导律
    int constructionControlLaw(unsigned char* sendBuf);//控制律
   // int constructionEngine_550(unsigned char* sendBuf);//550发动机

    int constructionEngine_t1400(unsigned char* sendBuf);//1400发动机
    int constructionOther_t1400(unsigned char* sendBuf);
    void setLinkState(int type);

    
private:
    //QString QDateTime::currentDateTime().toMSecsSinceEpoch());;
    QString generateOrderId();
    QString toHexString(quint32 value, int byteSize);
    QString toHexString_64(quint64 value, int byteSize);
    QJsonObject createBaseJsonObject(const QString& method);

    int m_linkState;
    //检查油量标定文件是否正常可用;
    bool checkOilStandardFile();
    //wds-2024/09/23-9-59-00插值算法;
    double interpolation(double var);
    //wds-2024/09/23-10-09-05 寻找两个电压值;
    void findNearValue(double& var, double& nearValueL, double& nearValueR);
    //wds-2024/09/23-10-29-42 寻找两个油量值;
    int findNearOilValue(double nearKeyL);

    //读取油量标定标志位;
    bool readOilStandardFlag = false;
    //油量标定数据;
    std::vector<double> vec_Voltage;
    double m_xyldy = 0.0;
    void initLogConfig();
    QString m_deviceSn;//飞机序列号
    QString m_deviceModel;//飞行类型
    QString m_strExePath;
};

