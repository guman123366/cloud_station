
/*
	UDP组播通信
*/

#pragma once

#include "CommunicationInterface.h"
#include <QUdpSocket>
#include <QObject>
#include "communicationinterface_global.h"
#include "qt_windows.h"
#include <MMSystem.h>
#include <QMutex>
#include <QTimer>
#include <QVector>

#define TX_BUF_SIZE 512
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
void CALLBACK MultiTimerProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);
class LogReportCoordinator;
class COMMUNICATIONINTERFACE_EXPORT UDPMultiCommunication : public CommunicationInterface
{
	Q_OBJECT

public:
	UDPMultiCommunication(QObject *parent=NULL);
	~UDPMultiCommunication();
	bool openPort() override;
	bool closePort() override;
	bool sendData(QByteArray ary, int nLength) override;
	bool sendDat(char* buff, int nLength);
	int getType();
    bool isDeviceOnline() const;  // 新增：获取设备在线状态
private slots:
	void  sl_receiveData();			//接收数据槽函数
    void sl_recevieControlData();//接收地面站发送的遥控指令的数据
	void  sl_timeout();
    void sl_getFCCVersion(unsigned short version);

public slots:
	//顾东刚 2022/04/11 15-59-57	450项目定时发送遥控指令
	void sl_send450Control();
	bool SendKgCommand(unsigned char num);
	void sl_updateKgControl(unsigned char);
	void sl_sendZuHeCommand(unsigned char cType, QVector<double> vecData);
	void sl_upfateLxControl(char* cBuff, int nLength);
signals:
	void si_updateRadioInfo(int type);
	void deviceOnlineStatusChanged(bool isOnline);  // 新增：设备在线状态变化信号
	void dataReceived();  // 新增：收到数据信号
private:
    QUdpSocket* m_pSocket;			//UDP组播对象
    QUdpSocket* m_controlSocket;
    MMRESULT m_timerResult;
    UINT g_MultiTime;
    void createTimer(MMRESULT& idEvent, UINT timeSec, UINT timerRec);
    QTimer m_sendControlTimer;		//450发送遥控指令定时器
    unsigned char g_SendBuff[40];
    unsigned short CalCRC16_CCITT(unsigned char* chkbuf, int len);
    int sendEmptyControlFrame(unsigned char* buffer);
    bool m_bSendControl;
    bool m_bSendAck = true;
    QMutex m_Mutex;
    bool SendLxCommand(unsigned char* databuff, int len);
    QTimer m_time;
    int m_nTimeOut;				//数据超时时间(3秒钟没有收到数据则认为数传电台丢失)
    int m_linkState;
    bool m_deviceOnline;  // 新增：设备在线状态
    QTimer* m_offlineCheckTimer;  // 新增：离线检测定时器
    LogReportCoordinator* m_logCoordinator;

};
