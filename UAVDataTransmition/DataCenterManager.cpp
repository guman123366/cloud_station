#include "DataCenterManager.h"
#include "LoadConfigInfo.h"
#include <QCoreApplication>
#include "DataProcessFactoryInterface.h"
#include "../CommunicationInterface/UDPMultiCommunication.h"
#include "../CommunicationInterface/HTTPCommunication.h"
//#include "../DataAnalysisInterface/TD220DataAnalysis.h"
//#include "../DataAnalysisInterface/TD220DataAnalysisCar.h"
#include "Engineer9XXXProtocol.h"
#include "IntelligentAirSpaceProtocol.h"
#include "../DataAnalysisInterface/IntelligentASAdminAnalysis.h"
#include "IntelligentASAdminProtocol.h"
#include "IntelligentAirSpaceData.h"
#include "../CommunicationInterface/HTTPCommunicationToken.h"
#include "../CommunicationInterface/UDPSingleCommunication.h"
#include"../CommunicationInterface/MQTTCommunication.h"//增加mqtt通信
//#include "../DataAnalysisInterface/VideoDataAnalysis.h"
//#include "../DataAnalysisInterface/SanYaDataAnalysis.h"
//#include "SanYaProtocol.h"
//#include "GuoWangProtocol.h"
//#include "ZhouShanProtocol.h"
#include "GuoWangAdminProtocol.h"
//#include "UOMReportProtocol.h"
//#include "IntelligentAirSpace550Protocol.h"
//#include "IntelligentASAdmin550Protocol.h"
#include "../DataAnalysisInterface/TD550DataAnalysis.h"
//#include "UomDataUpload.h"
#include"MQTTProtocol.h"
void DataCenterManager::initData()
{


	QString stPa = QCoreApplication::applicationDirPath();
	if (!ConfigInfo)
		ConfigInfo = new LoadConfigInfo;
}

void DataCenterManager::CreateUAVConnect()
{
	UAVType type = (UAVType)(getConfigInfo()->value("UAVType/Type").toInt());
 UAVCommunication = new UDPMultiCommunication;
	DataAnalysisInterface* UAVDataAnalysis = NULL;							
	ConstructionProtocolInterface* UAVConstructionProtocol = NULL;
	switch (type)
	{
	case DataCenterManager::TD220Car:
		//UAVDataAnalysis = new TD220DataAnalysisCar;
		//UAVCommunicaitonName = "TD220Car";
		break;
	case DataCenterManager::TD220:
		//UAVDataAnalysis = new TD220DataAnalysis;
		//UAVCommunicaitonName = "TD220";
		break;
	case DataCenterManager::TD550Car:
		break;
	case DataCenterManager::TD550:
		UAVDataAnalysis = new TD550DataAnalysis;
		UAVCommunicaitonName = "TD550";
		//connect(UAVDataAnalysis, SIGNAL(si_uomReport(UomUAVSate)), this, SLOT(sl_uomReport(UomUAVSate)));
		break;
	case DataCenterManager::SanYa:
		//UAVDataAnalysis = new SanYaDataAnalysis;
		//UAVCommunicaitonName = "SanYa";
		break;
	case DataCenterManager::MQTT:

	default:
		break;
	}

	UAVDataProcessFactory = new DataProcessFactoryInterface(UAVCommunication, UAVDataAnalysis, UAVConstructionProtocol);
	QString CommunicationListenAddress = getConfigInfo()->value("UAVCommunication/ListenAddress").toString();
	int CommunicationListenPort = getConfigInfo()->value("UAVCommunication/ListenPort").toInt();
	QString CommunicationSendAddress = getConfigInfo()->value("UAVCommunication/SendAddress").toString();
	int CommunicationSendPort = getConfigInfo()->value("UAVCommunication/SendPort").toInt();
	int UAVSendTimes = getConfigInfo()->value("UAVCommunication/SendTimes").toInt();
	
	//printf("---wds---CreateUAVConnect---ListenAddress--%s\n", CommunicationListenAddress.toStdString().data());
	//printf("---wds---CreateUAVConnect---ListenPort--%s\n", QString::number(CommunicationListenPort).toStdString().data());

	UAVDataProcessFactory->setReceiveCommunicationParam(CommunicationListenAddress, CommunicationListenPort, CommunicationSendAddress, CommunicationSendPort);
	UAVDataProcessFactory->setCommunicationName(UAVCommunicaitonName);
	UAVDataProcessFactory->CreateConnect();
	connect(UAVDataProcessFactory, SIGNAL(si_ReceiveData(DataDefineInterface*)), this, SLOT(sl_ReceiveUAVData(DataDefineInterface*)));
}

void DataCenterManager::DisUAVConnect()
{
	if (!UAVDataProcessFactory)
		return;

	UAVDataProcessFactory->DisConnect();
}

void DataCenterManager::CreateThirdConnect()
{
	ThirdType ThirdCommunicationType = (ThirdType)(getConfigInfo()->value("ThirdCommunication/ThirdType").toInt());
	CommunicationInterface* ThirdCommunication = NULL;						//第三方通讯方式
	DataAnalysisInterface* ThirdDataAnalysis = NULL;						//第三方数据解析
	ConstructionProtocolInterface* ThirdConstructionProtocol = NULL;		//第三方数据打包

	switch (ThirdCommunicationType)
	{
	case DataCenterManager::Engineer9XXX:
		ThirdCommunication = new UDPMultiCommunication;
		ThirdCommunicationName = "Engineer9XXX";
		////ThirdConstructionProtocol = new Engineer9XXXProtocol;
		break;
	case DataCenterManager::IntelligentAirSpace:
		ThirdCommunication = new HTTPCommunicationToken;
		ThirdCommunicationName = "IntelligentAirSpace";
		////ThirdConstructionProtocol = new IntelligentAirSpaceProtocol;

		//创建智能空域系统登陆连接
		CreateAirSpaceAdmin();
		//CreateVideoConnect();
		break;
	case DataCenterManager::IntelligentAirSpaceTD550:
		ThirdCommunication = new HTTPCommunicationToken;
		ThirdCommunicationName = "IntelligentAirSpaceTD550";
		////ThirdConstructionProtocol = new IntelligentAirSpace550Protocol;

		//创建智能空域系统登陆连接
		CreateAirSpaceAdmin();
		//CreateVideoConnect();
		break;
	case DataCenterManager::SupportJamming:
		break;
	case DataCenterManager::WuHanSanJing:
		break;
	case DataCenterManager::TD550_35suo:
		break;
	case DataCenterManager::TD550_SanYa:
		ThirdCommunication = new HTTPCommunication;
		ThirdCommunicationName = "SanYa";
		////ThirdConstructionProtocol = new SanYaProtocol;
		break;
	case DataCenterManager::TD550_GuoWang:
		ThirdCommunication = new HTTPCommunication;
		ThirdCommunicationName = "GuoWang";
		////ThirdConstructionProtocol = new GuoWangProtocol;
		//登陆连接
		CreateAirSpaceAdmin();
		break;
	case DataCenterManager::TD550_UOMReport:
		//ThirdCommunication = new HTTPCommunication;
		//ThirdCommunicationName = "UOMReport";
		//ThirdConstructionProtocol = new UOMReportProtocol;

		//调用马创锋的代码逻辑;
		////if (!dataUpload)
		////{
		////	dataUpload = new UomDataUpload();
		////	dataUpload->start();
		////}
		return;
		break;
	case DataCenterManager::TD550_ZhouShan:
		//没有视频推流功能;
		ThirdCommunication = new HTTPCommunication;
		ThirdCommunicationName = "ZhouShan";
		//ThirdConstructionProtocol = new ZhouShanProtocol;
		break;
	case DataCenterManager::MQTT:
		CreateMQTTConnect();
	default:
		break;
	}

	ThirdDataProcessFactory = new DataProcessFactoryInterface(ThirdCommunication, ThirdDataAnalysis, ThirdConstructionProtocol);
	QString ThirdListenAddress = getConfigInfo()->value("ThirdCommunication/ListenAddress").toString();
	int ThirdListenPort = getConfigInfo()->value("ThirdCommunication/ListenPort").toInt();
	QString ThirdSendAddress = getConfigInfo()->value("ThirdCommunication/SendAddress").toString();
	int ThirdSendPort = getConfigInfo()->value("ThirdCommunication/SendPort").toInt();
	ThirdSendTimes = getConfigInfo()->value("ThirdCommunication/SendTimes").toInt();
	ThirdDataProcessFactory->setReceiveCommunicationParam(ThirdListenAddress, ThirdListenPort, ThirdSendAddress, ThirdSendPort);
	ThirdDataProcessFactory->setCommunicationName(ThirdCommunicationName);
	ThirdDataProcessFactory->CreateConnect();
	ThirdDataProcessFactory->startSendTimer(ThirdSendTimes);
}
void DataCenterManager::test()
{

}
QString DataCenterManager::getVideoStreamIP()
{
	return getConfigInfo()->value("VideoCommunication/VideoPushStream").toString();
}
QString DataCenterManager::getVideoHd()
{
	return getConfigInfo()->value("VideoCommunication/VideoHd").toString();
}
void DataCenterManager::DisThirdConnect()
{
	if (!ThirdDataProcessFactory)
		return;

	ThirdDataProcessFactory->DisConnect();
	ThirdDataProcessFactory->stopSendTimer();
}

void DataCenterManager::CreateAirSpaceAdmin()
{
	////ThirdType ThirdCommunicationType = (ThirdType)(getConfigInfo()->value("ThirdCommunication/ThirdType").toInt());
	////CommunicationInterface* AirSpaceAdminCommunication = new HTTPCommunication;//智能空域登陆通讯方式
	////DataAnalysisInterface* AirSpaceAdminDataAnalysis = new	IntelligentASAdminAnalysis;					//智能空域登陆数据解析
	////ConstructionProtocolInterface* AirSpaceAdminConstructionProtocol = NULL;		//智能空域登陆数据打包
	////QString initToken = "";//广西和宁夏应急
	////switch (ThirdCommunicationType)
	////{
	////case DataCenterManager::IntelligentAirSpace:
	////	////AirSpaceAdminConstructionProtocol = new IntelligentASAdminProtocol;
	////	break;
	////case DataCenterManager::IntelligentAirSpaceTD550:
	////	////AirSpaceAdminConstructionProtocol = new IntelligentASAdmin550Protocol;
	////	break;
	////case DataCenterManager::TD550_GuoWang:
	////	initToken = getConfigInfo()->value("AirSpaceAdminCommunication/uavToken").toString();
	////	////AirSpaceAdminConstructionProtocol = new GuoWangAdminProtocol(initToken);
	////	break;
	////default:
	////	break;
	////}
	////AirSpaceAdminProcessFactory = new DataProcessFactoryInterface(AirSpaceAdminCommunication, AirSpaceAdminDataAnalysis, AirSpaceAdminConstructionProtocol);
	////
	////QString AirSpaceAdminSendAddress = getConfigInfo()->value("AirSpaceAdminCommunication/SendAddress").toString();
	////AirSpaceAdminProcessFactory->setReceiveCommunicationParam("", 0, AirSpaceAdminSendAddress, 0);
	////AirSpaceAdminProcessFactory->setCommunicationName(ThirdCommunicationName);
	////AirSpaceAdminProcessFactory->CreateConnect();
	////AirSpaceAdminProcessFactory->startSendTimer(5000);
}

void DataCenterManager::DisAirSpaceAdmin()
{
	if (!AirSpaceAdminProcessFactory)
		return;

	AirSpaceAdminProcessFactory->DisConnect();
	AirSpaceAdminProcessFactory->stopSendTimer();

	delete AirSpaceAdminProcessFactory;
	AirSpaceAdminProcessFactory = NULL;
}

void DataCenterManager::CreateVideoConnect()
{
	CommunicationInterface *VideoCommunication = new UDPSingleCommunication;
	//DataAnalysisInterface *VideoAnalysis = new VideoDataAnalysis;
	//VideoProcessFactory = new DataProcessFactoryInterface(VideoCommunication, VideoAnalysis, NULL);

	QString VideoListenAddress = getConfigInfo()->value("VideoCommunication/ListenAddress").toString();
	int VideoListenPort = getConfigInfo()->value("VideoCommunication/ListenPort").toInt();
	QString VideoSendAddress = getConfigInfo()->value("VideoCommunication/SendAddress").toString();
	int VideoSendPort = getConfigInfo()->value("VideoCommunication/SendPort").toInt();
	VideoProcessFactory->setReceiveCommunicationParam(VideoListenAddress, VideoListenPort, VideoSendAddress, VideoSendPort);
	VideoProcessFactory->setCommunicationName(ThirdCommunicationName);
	VideoProcessFactory->CreateConnect();
}

void DataCenterManager::DisVideoConnect()
{
	if (!VideoProcessFactory)
		return;

	VideoProcessFactory->DisConnect();
	delete VideoProcessFactory;
	VideoProcessFactory = NULL;
}

QSettings* DataCenterManager::getConfigInfo()
{
	if (!ConfigInfo)
		return NULL;

	return ConfigInfo->getConfigSettings();
}

void DataCenterManager::CreateAllConnect()
{
	//创建无人机连接
	CreateUAVConnect();
	//创建第三方通信
	CreateThirdConnect();
	//CreateMQTTConnect();
	GetDataTimer.start(100);
	connect(&GetDataTimer, SIGNAL(timeout()), this, SLOT(sl_ReceiveData()));

	connect(mqttCommunication, SIGNAL(si_sendKgCommand(unsigned char)), UAVCommunication, SLOT(sl_updateKgControl(unsigned char)));
	connect(mqttCommunication, SIGNAL(si_sendZuHeCommand(unsigned char , QVector<double> )), UAVCommunication, SLOT(sl_sendZuHeCommand(unsigned char , QVector<double> )));
 	connect(UAVCommunication, SIGNAL(si_updateRadioInfo(int)), mqttCommunication, SLOT(sl_updateRadioInfo(int)));
	connect(UAVCommunication, SIGNAL(deviceOnlineStatusChanged(bool)),mqttCommunication, SLOT(onUdpDeviceOnlineStatusChanged(bool)));
	connect(UAVCommunication, SIGNAL(dataReceived()), mqttCommunication, SLOT(onUdpDataReceived()));
	//connect(mqttCommunication, SIGNAL(si_osdAckTimeout(bool)), UAVCommunication, SLOT(sl_osdAckTimeout(bool)));
	bool connected = connect(mqttCommunication, SIGNAL(si_getLastTem()), this, SLOT(sl_recvGetLastTem()));
	connect(mqttCommunication, SIGNAL(si_sendVec(QVector<QVector<double>>)), this, SLOT(sl_getVec(QVector<QVector<double>>)));
	connect(this, SIGNAL(si_updateOrderReply(QString)), mqttCommunication, SLOT(sl_sendLoadAck(QString)));
}

void DataCenterManager::DisAllConnect()
{
	//断开无人机连接
	DisThirdConnect();
	//断开第三方通信
	DisUAVConnect();
	//断开视频连接
	//DisVideoConnect();
}

bool DataCenterManager::getUAVLinkState()
{
	if (!UAVDataProcessFactory)
		return false;
	else
		return UAVDataProcessFactory->getLinkState();
}
void DataCenterManager::sl_uomReport(UomUAVSate var)
{
	//{{{{**********将飞机的相关状态，赋值在这个对象里。************/
	////UomUAVSate state;
	////state.wayPoints.longitude = 116.3365;
	////state.wayPoints.latitude = 80.1354365;
	////state.wayPoints.height = 1500.0;
	//state.uavPose;
	//state.flightSate;
	//**********将飞机的相关状态，赋值在这个对象里。************}}}/
	////if (dataUpload)
	////{
	////	dataUpload->uploadData(var);
	////}
	//printf("------wds-----------\n");
}
void DataCenterManager::sl_recvGetLastTem()
{
		TD550TelemetryData* protocolData = dynamic_cast<TD550TelemetryData*>(UAVDataProcessFactory->getAnalysisData());
		MQTTCommunication* mqtt = dynamic_cast<MQTTCommunication*>(mqttCommunication);
		if (protocolData && mqtt)
		{
			mqtt->sl_recvLastTem550(protocolData);
		}

}
void DataCenterManager::sl_getVec(QVector<QVector<double>> vec)
{
	m_pointsVec.clear();
	m_pointsVec = vec;
	m_bUpRouteState = true;

	//发送航线装订指令

	if (!m_upRouteTimer.isActive())
	{
		m_upRouteTimer.start(100);
		connect(&m_upRouteTimer, SIGNAL(timeout()), this, SLOT(sl_updateUpRouteState()));
	}
	m_nUpWPIndex = 0;
	m_nWPIndex++;
	dynamic_cast<UDPMultiCommunication*>(UAVCommunication)->sl_sendZuHeCommand(0x16, m_pointsVec.at(m_nUpWPIndex));

}
void DataCenterManager::sl_updateUpRouteState()
{
	if (m_nWPIndex == 5)
	{
		QString strText = "";
		strText = u8"上传航线失败";
		m_bUpRouteState = false;
		emit si_updateOrderReply(strText);
		m_nWPIndex = 0;
		m_nWPTime = 0;
		m_nUpWPIndex = 0;

		if (m_upRouteTimer.isActive())
		{
			m_upRouteTimer.stop();
			disconnect(&m_upRouteTimer, SIGNAL(timeout()), this, SLOT(sl_updateUpRouteState()));
		}

		return;
	}

	m_nWPTime += 100;
	if (m_nWPTime > 2000)
	{
		dynamic_cast<UDPMultiCommunication*>(UAVCommunication)->sl_sendZuHeCommand(0x16, m_pointsVec.at(m_nUpWPIndex));
		m_nWPIndex++;
		m_nWPTime = 0;
	}
}
void DataCenterManager::sl_ReceiveData()
{
	if (!UAVDataProcessFactory || !ThirdDataProcessFactory||!MQTTProcessFactory)
		return;
	DataDefineInterface* aaa = UAVDataProcessFactory->getAnalysisData();
	//获取遥测数据指令回复
	TD550TelemetryData* protocolData = dynamic_cast<TD550TelemetryData*>(aaa);
	if(protocolData)
	{ 
	unsigned short  strReply =protocolData->m_ThirdSubFourFrame.KgReply;
	//qDebug() << "strReply" << strReply;
	if ((strReply != 0x00) && (strReply != m_strForwardReply))
	{
		MQTTCommunication* mqtt = dynamic_cast<MQTTCommunication*>(mqttCommunication);
		if (protocolData && mqtt)
		{
			mqtt->sl_recvLastTem550(protocolData);
		}
		m_strForwardReply = strReply;
	}
	}
//航线上传功能

	if (m_bUpRouteState)
	{
		if (m_nUpWPIndex == protocolData->m_ThirdSubFourFrame.byte12Reply-1)
		{
			qDebug() << "byte12Reply" << protocolData->m_ThirdSubFourFrame.byte12Reply - 1;
			m_nWPTime = 0;
			m_nWPIndex = 0;
			QString	strReply = QString(u8"上传航点%1成功").arg(m_nUpWPIndex);
			emit si_updateOrderReply(strReply);
			m_nUpWPIndex++;
			m_nWPIndex++;
			if (m_nUpWPIndex < m_pointsVec.size())
			{
				
				dynamic_cast<UDPMultiCommunication*>(UAVCommunication)->sl_sendZuHeCommand(0x16, m_pointsVec.at(m_nUpWPIndex));
				qDebug() << "sl_sendZuHeCommand"<< m_pointsVec.at(m_nUpWPIndex);
			}
			else
			{
				QString strText = u8"上传航线成功";
				emit si_updateOrderReply(strText);
				m_nWPIndex = 0;
				m_nWPTime = 0;
				m_nUpWPIndex = 0;
				m_bUpRouteState = false;
				if (m_upRouteTimer.isActive())
				{
					m_upRouteTimer.stop();
					disconnect(&m_upRouteTimer, SIGNAL(timeout()), this, SLOT(sl_updateUpRouteState()));
				}

			}
		}
	}

	ThirdDataProcessFactory->setThirdData(UAVDataProcessFactory->getAnalysisData());
	MQTTProcessFactory->setThirdData(UAVDataProcessFactory->getAnalysisData());

	if (AirSpaceAdminProcessFactory)
	{
		if (AirSpaceAdminProcessFactory->getAnalysisData())
		{
			IntelligentAirSpaceAdmin *AirSpaceAdminData = (IntelligentAirSpaceAdmin *)(AirSpaceAdminProcessFactory->getAnalysisData());
			QByteArray token = AirSpaceAdminData->Token.toUtf8();
			HTTPCommunication *TokenCommunication = (HTTPCommunication *)ThirdDataProcessFactory->getCommunication();
			TokenCommunication->setToken(AirSpaceAdminData->Token.toUtf8());
			DisAirSpaceAdmin();
			ThirdDataProcessFactory->startSendTimer(ThirdSendTimes);
		}
	}
}

DataCenterManager::DataCenterManager(QObject *parent)
	: QObject(parent), ConfigInfo(NULL), UAVCommunicaitonName(""), ThirdCommunicationName(""), ThirdDataProcessFactory(NULL)
	, UAVDataProcessFactory(NULL), AirSpaceAdminProcessFactory(NULL), ThirdSendTimes(200)
{
	initData();
}

DataCenterManager::~DataCenterManager()
{
	if (UAVCommunication)
	{
		delete UAVCommunication;
	}
}


void DataCenterManager::CreateMQTTConnect()
{
	// 获取配置信息
	
	QString mqttClientId = getConfigInfo()->value("MQTTCommunication/ClientId").toString();
	QString mqttUsername = getConfigInfo()->value("MQTTCommunication/Username").toString();
	QString mqttPassword = getConfigInfo()->value("MQTTCommunication/Password").toString();
	QString ListenAddress=getConfigInfo()->value("MQTTCommunication/MQTTListenAddress").toString();
	int MQTTListenPort = getConfigInfo()->value("MQTTCommunication/MQTTListenPort").toInt();
	QString MQTTTelemetryTopic= getConfigInfo()->value("MQTTCommunication/MQTTTelemetryTopic").toString();
	QString MQTTCommandTopic = getConfigInfo()->value("MQTTCommunication/MQTTCommandTopic").toString();
	// 创建MQTT通信实例
	 mqttCommunication = new MQTTCommunication;


	// 设置MQTT特有参数
	static_cast<MQTTCommunication*>(mqttCommunication)->setClientId(mqttClientId);
	static_cast<MQTTCommunication*>(mqttCommunication)->setKeepAlive(60);

	// 创建数据解析和协议构造实例
	DataAnalysisInterface* MQTTDataAnalysis = new TD550DataAnalysis; // 复用TD550解析器
	ConstructionProtocolInterface* MQTTConstructionProtocol = new MQTTProtocol;

	MQTTCommunicationName = "MQTT_T1400";
	MQTTProcessFactory = new DataProcessFactoryInterface(
		mqttCommunication,
		MQTTDataAnalysis,
		MQTTConstructionProtocol
	);

	
	MQTTProcessFactory->setReceiveCommunicationParam(
		ListenAddress, // ListenAddress作为订阅主题
		MQTTListenPort,             // ListenPort不使用
		"",  // SendAddress作为发布主题
		  0           // SendPort作为QoS级别
	);

	MQTTProcessFactory->setCommunicationName(MQTTCommunicationName);
	MQTTProcessFactory->CreateConnect();
	MQTTProcessFactory->startSendTimer(1000);

	// 订阅主题
	//static_cast<MQTTCommunication*>(mqttCommunication)->subscribe(topicSubscribe);

	// 连接数据接收信号
	connect(MQTTProcessFactory, SIGNAL(si_ReceiveData(DataDefineInterface*)),
		this, SLOT(sl_ReceiveUAVData(DataDefineInterface*)));
}

void DataCenterManager::DisMQTTConnect()
{
	if (!MQTTProcessFactory)
		return;

	MQTTProcessFactory->DisConnect();
	delete MQTTProcessFactory;
	MQTTProcessFactory = nullptr;
}