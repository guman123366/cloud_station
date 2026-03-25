
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
#include <QtMath>
#include <QTimer>
#include<QSettings>
#include<QFile>
#include<QtConcurrent>

#define TX_BUF_SIZE 512
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
void CALLBACK MultiTimerProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);
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
    unsigned short m_fccVsersion;
    QString m_appVersion;
    /**********************************日志文件上传相关的函数************************/
private:
    //存储文件相关
    // 
    int m_td550FileSizeMB;      // TD550文件大小限制（MB）
    int m_t1400FileSizeMB;      // T1400文件大小限制（MB）
    int m_R6000FileSizeMB;      // T1400文件大小限制（MB）
    int m_fileMergeTimeMinutes; // 文件合并时间（分钟）
    bool getLogFileConfig();
    void loadLogFileConfig();
    // 获取日志上传配置功能
    void getLogConfigFromCurl();
    // 初始化文件系统
    bool initialize();
    // 初始化配置文件
    void initConfig();
    // 创建数据目录
    bool createDataDirectories();
    // 初始化文件
    void initFiles();
    // 写入接收数据
    void writeRxData(const QByteArray& data);
    // 写入发送数据
    void writeTxData(const QByteArray& data);
    // 关闭所有文件
    void closeFiles();
    // 获取当前文件信息
    QString getCurrentRxFileName() const;
    QString getCurrentTxFileName() const;
    // 生成新文件名
    QString generateFileName(const QString& type);
    // 打开新文件
    bool openNewFile(QFile*& file, const QString& fileName);
    // 文件上传实现
    bool uploadFileImpl(const QString& filepath, const QString& data_type);
    std::string calculateFileMD5(const std::string& filename);
    std::string calculateFileMD5(const QString& filename);
    QSettings* getConfigInfo();
    bool uploadFile(const std::string& filepath, const std::string& data_type = "log");
    bool uploadFile(const QString& filepath, const QString& data_type = "log");

    void startUploadCheckTimer();
    void uploadFileWhenFull(const QString& fileName);
    void checkAndUploadCompletedFiles();
    bool isFileReadyForUpload(const QString& filePath);
    bool shouldScheduleFileForUpload(const QString& filePath, const QString& currentFilePath);
    void scheduleEligibleFilesFromDirectory(const QString& directoryPath, const QString& currentFilePath);
    void scheduleFileUpload(const QString& filePath);
    void processUploadQueue();
    int extractSequenceFromFilename(const QString& filename);
    QString extractTimestampFromFilename(const QString& filename);
    bool parseFilenameComponents(const QString& filename, QString& timestampStr, int& sequenceNum);
    bool isUploading() ;

    // 新增：获取上传队列大小的方法
    int getUploadQueueSize() ;
    void uploadAllRemainingFiles(); //在程序退出或停止时上传所有文件
    void uploadAllEligibleFiles();
    QJsonObject MetadataToJsonObject() ;
    QString getFCCVersion();
    qint64 getSortieWithRetry(int maxRetries,int retryDelayMs);//获取架次号

 
    // 对队列按时间戳从大到小排序
    void sortQueueByTimestampDescending();
private:
    QFile* m_pRxFile;        // 接收数据文件
    QFile* m_pTxFile;        // 发送数据文件

    QString m_strExePath;    // 应用程序路径
    QString m_dataFolder;    // 数据文件夹
    QString m_rxFileFolder;  // 接收文件文件夹
    QString m_txFileFolder;  // 发送文件文件夹

    qint64 m_maxFileSize;    // 最大文件大小 (bytes)
    bool m_enableFileSplit;  // 是否启用文件分割
    QString m_filePrefix;    // 文件前缀

    QString m_currentRxFileName; // 当前接收文件名
    QString m_currentTxFileName; // 当前发送文件名

    qint64 m_currentRxFileSize; // 当前接收文件大小
    qint64 m_currentTxFileSize; // 当前发送文件大小

    //配置读取
    std::string m_apiPath;
    std::string m_uploadUrl;
    std::string m_apiSucessPath;
    std::string m_successReportUrl;
    std::string m_logFileConfigUrl;
    QString m_defaultLogType;
    bool m_autoGenerateSortie;
    QString m_sortieResetValue;
    QString m_currentSortie;
    int m_sortieTimeout;
    std::string m_deviceModel;//飞行类型
    std::string m_deviceSn;//飞机序列号
    qint64 m_sortie;
    std::string m_appID;
    std::string m_appSecret;
    // 日志类型映射
    QMap<QString, QMap<QString, QString>> m_logTypeMappings;
    std::string m_sortiePath;
    std::string m_sortieUrl;
    /************* // 文件上传管理相关*/
// 文件上传相关
    QTimer* m_uploadCheckTimer;
    QList<QString> m_pendingUploadFiles;  // 待上传文件队列
    QMutex m_uploadMutex;               // 上传队列互斥锁
    bool m_isUploading;                 // 是否正在上传

};
