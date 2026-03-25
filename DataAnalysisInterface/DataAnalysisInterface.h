/*
	解析地面控制软件数据接口
*/

#pragma once
#include "dataanalysisinterface_global.h"
#include <QObject>
#include "../UAVDataTransmition/DataDefineInterface.h"


class DATAANALYSISINTERFACE_EXPORT DataAnalysisInterface : public QObject
{
	Q_OBJECT

public:
	DataAnalysisInterface(QObject *parent);
	virtual ~DataAnalysisInterface();

	virtual DataDefineInterface* AnalyseData(QByteArray ary, int nLength) = 0;

	int GetInt4Low(unsigned char *buf, int nIndex);
	int GetInt2Low(unsigned char *buf, int nIndex);
	int GetUInt4Low(unsigned char *buf, int nIndex);
	int GetUInt2Low(unsigned char *buf, int nIndex);
};
