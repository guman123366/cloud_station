#pragma once
#include <QObject>
#include "DataAnalysisInterface.h"
#include "dataanalysisinterface_global.h"
#include "../UAVDataTransmition/TD220TelemetryDataCar.h"

class DATAANALYSISINTERFACE_EXPORT TD220DataAnalysisCarCommon : public DataAnalysisInterface
{
	Q_OBJECT

public:
	TD220DataAnalysisCarCommon(QObject *parent=NULL);
	~TD220DataAnalysisCarCommon();

	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;

	enum DecoderState {
		DECODER_STATE_SYNC0,
		DECODER_STATE_SYNC1,
		DECODER_STATE_SYNC2,
		DECODER_STATE_SYNC3
	};

private:
	void ParseTelemetryData(unsigned char* buf, int nLength);	//解析遥测数据
	unsigned short cal_crc(char *data, unsigned int len);		//CRC校验计算

	TD220TelemetryDataCar* m_pTD220TelemetryDataCar;			//TD220控制车数据
	unsigned int ReceiveDataIndex;
};
