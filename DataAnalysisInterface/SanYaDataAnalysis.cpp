#include "SanYaDataAnalysis.h"
#include "../UAVDataTransmition/SanYaData.h"
#include <QDebug>

SanYaDataAnalysis::SanYaDataAnalysis(QObject *parent)
	: DataAnalysisInterface(parent),m_SanYaData(NULL)
{
}

SanYaDataAnalysis::~SanYaDataAnalysis()
{
}

DataDefineInterface* SanYaDataAnalysis::AnalyseData(QByteArray ary, int nLength)
{
	unsigned char SanYaBuf[1024] = { 0 };
	memcpy(SanYaBuf, ary.data(), nLength);

	if ((SanYaBuf[0] == 0x14) && (SanYaBuf[1] == 0x01))
	{
		if (!m_SanYaData)
		{
			m_SanYaData = new SanYaData;
		}
		
		m_SanYaData->m_dLon = (SanYaBuf[16] + SanYaBuf[17] * 0x100 + SanYaBuf[18] * 0x10000 + SanYaBuf[19] * 0x1000000)*1.0*1e-7;
		m_SanYaData->m_dLat = (SanYaBuf[20] + SanYaBuf[21] * 0x100 + SanYaBuf[22] * 0x10000 + SanYaBuf[23] * 0x1000000)*1.0*1e-7;
		m_SanYaData->m_dAlt = (SanYaBuf[26] + SanYaBuf[27] * 0x100 + SanYaBuf[28] * 0x10000 + SanYaBuf[29] * 0x1000000)*1.0*1e-3;

		return m_SanYaData;
	}

	return NULL;
}
