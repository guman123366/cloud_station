#pragma once
#include <QObject>
#include "DataAnalysisInterface.h"
#include "dataanalysisinterface_global.h"
#include <qmutex.h>
//#include "../UAVDataTransmition/UomDataDefine.h"
class TD550TelemetryData;

class DATAANALYSISINTERFACE_EXPORT TD550DataAnalysis : public DataAnalysisInterface
{
	Q_OBJECT

public:
	TD550DataAnalysis(QObject *parent=NULL);
	~TD550DataAnalysis();
	unsigned short getSendFCCVersion();
	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;
signals:
	////void si_uomReport(UomUAVSate var);
	void si_sendTemData();
private:
	void handleData();
	unsigned short CalCRC16_CCITT(unsigned char *chkbuf, int len);
	void decode550(unsigned char * m_rebuf, int m_rxlen);
private:
	int m_nBufSize;
	int m_nDataSize;
	unsigned char *m_pBuf;
	unsigned char *m_pBufSwap;

	unsigned char m_arrDecodeData[256];
	int m_nFlag;
	unsigned short  m_FCCVersion;
	enum DecoderState450 {
		DECODER_STATE_SYNC0,
		DECODER_STATE_SYNC1,
		DECODER_STATE_SYNC2,
		DECODER_STATE_SYNC3
	};

	enum
	{
		HEAD81 = 0x81,
		HEAD81_1 = 0x11,
		HEAD81_2 = 0x21,
		HEAD81_3 = 0x31,
		HEAD82 = 0x82,
		HEAD82_1 = 0x12,
		HEAD82_2 = 0x22,
		HEAD82_3 = 0x32,
		HEAD83 = 0x83,
		HEAD83_1 = 0x13,
		HEAD83_2 = 0x23,
		HEAD83_3 = 0x33,
		HEAD84 = 0x84,
		HEAD84_1 = 0x14,
		HEAD84_2 = 0x24,
		HEAD84_3 = 0x34,
		TAILEB = 0xEB,
		TAIL90 = 0x90,
	};

	enum Decode450State
	{
		//Decode_NoState,			//初始状态
		Decode_Head81,			//第一副帧标识
		Decode_Head81_NUM,		//第一副帧编号
		Decode_Frame81_EB,		//第一副帧同步字1
		Decode_Frame81_90,		//第一副帧同步字2
		Decode_Head82,			//第二副帧标识
		Decode_Head82_NUM,		//第二副帧编号
		Decode_Frame82_EB,		//第二副帧同步字1
		Decode_Frame82_90,		//第二副帧同步字2
		Decode_Head83,			//第三副帧标识
		Decode_Head83_NUM,		//第三副帧编号
		Decode_Frame83_EB,		//第三副帧同步字1
		Decode_Frame83_90,		//第三副帧同步字2
		Decode_Head84,			//第四副帧标识
		Decode_Head84_NUM,		//第四副帧编号
		Decode_Frame84_EB, //第四副帧同步字1
		Decode_Frame84_90,//第四副帧同步字2
	};

	const unsigned char START1 = 0x81, START2 = 0x82, START3 = 0x83, START4 = 0x84;
	const unsigned char SYNC1 = 0x55, SYNC2 = 0XAA, SYNC3 = 0XEB, SYNC4 = 0X90;

	TD550TelemetryData* m_TD550TelemetryData;

	QMutex m_YCMutex;

	//UomUAVSate m_uomData;

};
