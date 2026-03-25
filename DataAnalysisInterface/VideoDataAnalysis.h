#pragma once
#include <QObject>
#include "DataAnalysisInterface.h"
#include "dataanalysisinterface_global.h"
//#include "CircleBuffer.h"
//#include "SeriousTmer.h"
//#include "RTMPStreamTranslation.h"
#include <QTimer>

class DataDefineInterface;

class DATAANALYSISINTERFACE_EXPORT VideoDataAnalysis : public DataAnalysisInterface
{
	Q_OBJECT

public:
	VideoDataAnalysis(QObject *parent=NULL);
	~VideoDataAnalysis();

	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;

private:
	int findMove(int& nIndex, int& nMoveBit);		//获取数据移动位数
	bool cmpData(unsigned char* src, unsigned char* dis, int nSize);
	//数据移位函数
	void moveDataBit(unsigned char *buff, unsigned char *processData, int nLength, int nMoveBit);
private slots:
	void sl_ProcessTimer();
private:
	//CircleBuffer *m_pCirCleBuffer;	//环形缓存，用于存储2171格式视频数据
	QTimer *ProcessTimer;		//处理数据定时器
	int m_nIndex, m_nMoveBit;
	//VideoData *SendVideoData;		//发送的H264视频数据
	//RTMPStreamTranslation *VideoRTMP;
};
