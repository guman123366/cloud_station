#pragma once

#include <QObject>

class DataDefineInterface;

class ConstructionProtocolInterface : public QObject
{
	Q_OBJECT

public:
	ConstructionProtocolInterface(QObject *parent);
	virtual ~ConstructionProtocolInterface();

	virtual int ConstructionData(unsigned char* sendBuf) = 0;		//打包数据接口

	void setUAVData(DataDefineInterface* UAVData){ ReceiveUAVData = UAVData; }

	DataDefineInterface* ReceiveUAVData;
};
