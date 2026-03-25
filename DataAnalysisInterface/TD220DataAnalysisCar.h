/*
	解析TD220控制车版本数据
*/

#pragma once
#include <QObject>
#include "DataAnalysisInterface.h"
#include "../UAVDataTransmition/TD220TelemetryDataCar.h"
#include <QTimer>
#include "dataanalysisinterface_global.h"

class DATAANALYSISINTERFACE_EXPORT TD220DataAnalysisCar : public DataAnalysisInterface
{
	Q_OBJECT

public:
	TD220DataAnalysisCar(QObject *parent=NULL);
	~TD220DataAnalysisCar();

	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;

	enum DecoderState {
		DECODER_STATE_SYNC0,
		DECODER_STATE_SYNC1,
		DECODER_STATE_SYNC2,
		DECODER_STATE_SYNC3
	};

private:
	void ParseTelemetryData(unsigned char* buf, int nLength);	//解析遥测数据
	void ParseStationData(unsigned char* buf, int nLength);			//解析基准站位置数据
	unsigned short cal_crc(char *data, unsigned int len);		//CRC校验计算

	TD220TelemetryDataCar* m_pTD220TelemetryDataCar;			//TD220控制车数据
	unsigned int ReceiveDataIndex;
};
