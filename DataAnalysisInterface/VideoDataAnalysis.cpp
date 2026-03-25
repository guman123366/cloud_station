#include "VideoDataAnalysis.h"
#include "../UAVDataTransmition/VideoDataDefine.h"

unsigned char headFrame1[4] = { 0x00, 0x00, 0x8B, 0x02 };
unsigned char headFrame2[4] = { 0x00, 0x00, 0x45, 0x81 };
unsigned char headFrame3[4] = { 0x00, 0x00, 0x22, 0xC0 };
unsigned char headFrame4[4] = { 0x00, 0x00, 0x11, 0x60 };
unsigned char headFrame5[4] = { 0x00, 0x00, 0x08, 0xB0 };
unsigned char headFrame6[4] = { 0x00, 0x00, 0x04, 0x58 };
unsigned char headFrame7[4] = { 0x00, 0x00, 0x02, 0x2C };
unsigned char headFrame8[4] = { 0x00, 0x01, 0x16, 0x05 };

VideoDataAnalysis::VideoDataAnalysis(QObject *parent)
	: DataAnalysisInterface(parent), m_nMoveBit(0), m_nIndex(0)//, VideoRTMP(nullptr)
{
	//m_pCirCleBuffer = new CircleBuffer;

	ProcessTimer = new QTimer;
	connect(ProcessTimer, SIGNAL(timeout()), this, SLOT(sl_ProcessTimer()));
	ProcessTimer->start(50);

	//VideoRTMP = new RTMPStreamTranslation;
}

VideoDataAnalysis::~VideoDataAnalysis()
{
}

DataDefineInterface* VideoDataAnalysis::AnalyseData(QByteArray ary, int nLength)
{
	char VideoBuff2171[10000] = { 0 };
	memcpy(&VideoBuff2171, ary.data(), nLength);
	//if (nLength == 2171 && VideoBuff2171[0] == 0x5C && VideoBuff2171[1] == 0x36 && VideoBuff2171[2] == 0x54 && VideoBuff2171[3] == (char)0xFF && VideoBuff2171[4] == 0x31)
	//{
	//	//去除链路打包数据，获取光电厂家视频数据
	//	m_pCirCleBuffer->setData((unsigned char*)(VideoBuff2171 + 8), 2160);
	//}

	//VideoData *sendBuf

	VideoData *videoStream = new VideoData;
	videoStream->VideoBufLength = nLength;
	memcpy(videoStream->VideoBuf, VideoBuff2171, nLength);

	return NULL;
}

int VideoDataAnalysis::findMove(int& nIndex, int& nMoveBit)
{
	int index = 0;
	while (1)
	{
		unsigned char cHeadData[4] = { 0 };
		/*if (index > m_pCirCleBuffer->getLength())
		{
			return -1;
		}*/
		/*int nStartPos = m_pCirCleBuffer->getStartPos() + index;
		if (nStartPos >= 1000000)
		{
			nStartPos = nStartPos - 1000000;
		}
		bool bChk = m_pCirCleBuffer->findData(cHeadData, nStartPos, 4);
		if (!bChk)
		{
			return -1;
		}*/
		if (cHeadData == NULL)
		{
			return -1;
		}
		if (cmpData(cHeadData, headFrame1, 4))
		{
			nIndex = index;
			nMoveBit = 1;
			return 0;
		}
		if (cmpData(cHeadData, headFrame2, 4))
		{
			nIndex = index;
			nMoveBit = 2;
			return 0;
		}
		if (cmpData(cHeadData, headFrame3, 4))
		{
			nIndex = index;
			nMoveBit = 3;
			return 0;
		}
		if (cmpData(cHeadData, headFrame4, 4))
		{
			nIndex = index;
			nMoveBit = 4;
			return 0;
		}
		if (cmpData(cHeadData, headFrame5, 4))
		{
			nIndex = index;
			nMoveBit = 5;
			return 0;
		}
		if (cmpData(cHeadData, headFrame6, 4))
		{
			nIndex = index;
			nMoveBit = 6;
			return 0;
		}
		if (cmpData(cHeadData, headFrame7, 4))
		{
			nIndex = index;
			nMoveBit = 7;
			return 0;
		}
		if (cmpData(cHeadData, headFrame8, 4))
		{
			nIndex = index;
			nMoveBit = 0;
			return 0;
		}
		index++;
	}

	return -1;
}

bool VideoDataAnalysis::cmpData(unsigned char* src, unsigned char* dis, int nSize)
{
	for (int i = 0; i < nSize; i++)
	{
		if (src[i] != dis[i])
		{
			return false;
		}
	}

	return true;
}

void VideoDataAnalysis::moveDataBit(unsigned char *buff, unsigned char *processData, int nLength, int nMoveBit)
{
	for (int i = 0; i <= nLength; i++)
	{
		if (i == nLength - 1)
		{
			processData[i] = buff[i] << nMoveBit;
		}
		else
		{
			processData[i] = (buff[i] << nMoveBit) | (buff[i + 1] >> (8 - nMoveBit));
		}
	}
}

void VideoDataAnalysis::sl_ProcessTimer()
{
	unsigned char cMoveBuff[100000] = { 0 }, cTemp[100000] = { 0 }, cVideoData[100000] = { 0 }, cVideoTemp[100000] = { 0 };
	static bool isHead = false;
	int nTmep = 0;
	if (!isHead)
	{

		if (-1 != findMove(m_nIndex, m_nMoveBit))
		{
			isHead = true;
			//m_pCirCleBuffer->getData(cTemp, m_nIndex + 4);
		}
	}
	else
	{
		if (-1 != findMove(m_nIndex, m_nMoveBit))
		{
			//bool bGetOk = m_pCirCleBuffer->getData(cMoveBuff, m_nIndex + 4);
			/*if (!bGetOk)
			{
				return;
			}*/
			moveDataBit(cMoveBuff, cVideoTemp, m_nIndex + 4, m_nMoveBit);
			if (m_nIndex < 168)
			{
				return;
			}
			int nH264Length = cVideoTemp[0] + cVideoTemp[1] * 0x100 + cVideoTemp[2] * 0x10000 + cVideoTemp[3] * 0x1000000;
			/*SendVideoData = new VideoData;
			memcpy(cVideoData, cVideoTemp + 162, nH264Length - 168);
			memcpy(SendVideoData->VideoBuf, cVideoTemp + 162, nH264Length - 168);
			SendVideoData->VideoBufLength = nH264Length - 168;
			memset(cVideoData, 0, 100000);*/

			
			VideoData *videoStream = new VideoData;
			videoStream->VideoBufLength = nH264Length - 168;
			memcpy(videoStream, cVideoTemp + 162, nH264Length - 168);

			//VideoRTMP->

			/*if (!VideoRTMP->isRunning())
			{
				VideoRTMP->start();
			}*/
		}
	}
}
