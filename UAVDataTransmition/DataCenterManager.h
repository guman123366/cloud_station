#pragma once

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QTimer>
#include "UomDataDefine.h"
#include<qthread.h>
class LoadConfigInfo;
class DataProcessFactoryInterface;
class UDPMultiCommunication;
class HTTPCommunication;
class CommunicationInterface;
//class TD220DataAnalysis;
//class TD220DataAnalysisCar;
class DataAnalysisInterface;
class DataDefineInterface;
class HTTPCommunicationToken;
class UDPSingleCommunication;
//class SanYaDataAnalysis;
//class UomDataUpload;
class DataCenterManager : public QObject
{
	Q_OBJECT

public:
	DataCenterManager(QObject *parent = NULL);
	~DataCenterManager();

	QSettings* getConfigInfo();
	void CreateAllConnect();		//创建所有连接
	void DisAllConnect();			//断开所有连接
	bool getUAVLinkState();			//获取接收数据状态
	void test();					//测试方法;
	QString getVideoStreamIP();
	QString getVideoHd();
private slots:
void sl_ReceiveData();
void sl_uomReport(UomUAVSate var);
void sl_recvGetLastTem();
void sl_getVec(QVector<QVector<double>>);
void sl_updateUpRouteState();
private:
	void initData();
	void CreateUAVConnect();		//创建无人机连接
	void DisUAVConnect();			//断开无人机连接
	void CreateThirdConnect();		//创建第三方连接
	void DisThirdConnect();			//断开第三方连接
	void CreateAirSpaceAdmin();		//创建智能空域登陆连接
	void DisAirSpaceAdmin();		//断开智能空域登陆连接
	void CreateVideoConnect();		//创建视频数据连接
	void DisVideoConnect();			//断开视频数据连接

	LoadConfigInfo* ConfigInfo;

	QString UAVCommunicaitonName;		//无人机通信名称
	DataProcessFactoryInterface* UAVDataProcessFactory;		//无人机数据处理工厂
	QString ThirdCommunicationName;		//第三方通信名称
	DataProcessFactoryInterface* ThirdDataProcessFactory;	//第三方数据处理工厂
	QTimer GetDataTimer;				//获取接收数据定时器
	int ThirdSendTimes;
	DataProcessFactoryInterface *AirSpaceAdminProcessFactory;	//智能空域登陆数据处理工厂
	HTTPCommunicationToken* ThirdCommunication;
	DataProcessFactoryInterface *VideoProcessFactory;			//视频数据处理工厂

	void CreateMQTTConnect();      // 创建MQTT连接
	void DisMQTTConnect();        // 断开MQTT连接

	DataProcessFactoryInterface* MQTTProcessFactory; // MQTT数据处理工厂
	QString MQTTCommunicationName; // MQTT通信名称

	CommunicationInterface* UAVCommunication=nullptr;
	CommunicationInterface* mqttCommunication = nullptr;

	unsigned short  m_strForwardReply;
	////UomDataUpload*        dataUpload = nullptr;//马创锋的逻辑;
	enum UAVType
	{
		TD220Car=0,		//TD220Car
		TD220,			//TD220通用
		TD550Car,		//TD550Car
		TD550,			//TD550便携
		SanYa,			//三亚项目解析地面站软件发送给链路的数据
		//MQTT,//通过mqttt通信
	};

	enum ThirdType
	{
		Engineer9XXX=0,		//9XXX工程
		IntelligentAirSpace,//智能空域指挥系统
		IntelligentAirSpaceTD550,//TD550接入智能空域指挥系统
		SupportJamming,		//支援干扰
		WuHanSanJing,		//武汉三江
		TD550_35suo,		//TD55035suo
		TD550_SanYa,		//三亚演示
		TD550_GuoWang,		//国网项目;
		TD550_UOMReport,	//UOM数据上报;
		TD550_ZhouShan,		//舟山邮政物流项目
		MQTT,
	};

	int m_nWPIndex;	//未收到航点回复重复上传某个航点次数
	int m_nWPTime;			//未收到航点回复时间
	int m_nUpWPIndex;		//上传的航点编号
	bool m_bUpRouteState = false;	//上传航线状态
	QVector<double> m_vecSendWPData;//上传航点数据
	QVector<QVector<double>> m_pointsVec;
	QTimer m_upRouteTimer;										//加注航点定时
signals:
	void si_updateOrderReply(QString reply);
};
