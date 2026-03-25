#pragma once
#include <QObject>
#include "DataAnalysisInterface.h"
#include "dataanalysisinterface_global.h"

class DataDefineInterface;
class TD220TelemetryData;

class DATAANALYSISINTERFACE_EXPORT TD220DataAnalysis : public DataAnalysisInterface
{
	Q_OBJECT

public:
	TD220DataAnalysis(QObject *parent=NULL);
	~TD220DataAnalysis();

	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;

	enum DecoderState
	{
		DECODER_STATE_SYNC,
		DECODER_STATE_ADR,
		DECODER_STATE_LEN,
		DECODER_STATE_PAYLOAD,
		DECODER_STATE_CRC_1,
		DECODER_STATE_CRC_2,
		DECODER_STATE_RSSI
	};
private:
	unsigned short crc16Accumulate(char * data, unsigned int nLength);						//计算CRC16校验：初始值为0xFFFF，多项式为：0x1021
	DataDefineInterface* analyseTelemetryData(unsigned char* szData, int nIndex, const int len);			//解析Telemetry数据
	void AnalyseRelPosition(unsigned char* rxbuf, const int & nDataStartIndex=0);			//解析相对位置
	void AnalyseVelnav(unsigned char* szData, const int & nDataStartIndex = 0);				//解析速度
	void AnalyseReference(unsigned char* szData, const int & nDataStartIndex = 0);			//解析参考速度
	void AnalyseAttitude(unsigned char* szData, const int & nDataStartIndex = 0);			//解析姿态
	void AnalyseControl(unsigned char* szData, const int & nDataStartIndex = 0);			//解析遥控器数据
	void AnalyseServoPosition(unsigned char* szData, const int & nDataStartIndex = 0);		//解析伺服舵机位置
	void AnalyseADValue(unsigned char* szData, const int & nDataStartIndex = 0);			//解析外置传感器数据
	void AnalyModeInfo(const int szData);													//解析模式信息
	void AnalySensorInfo(const int szData);													//解析机载设备信息
	void AnalyseStatusAlarmInfo(const int szData);
	void AnalyseStatusServoLinkInfo(const int szData);
private:
	//校验时用的buf
	unsigned char m_rxCheckBuf[2048];
	TD220TelemetryData* AnalyseDataDefine;		//需要解析的数据
};
