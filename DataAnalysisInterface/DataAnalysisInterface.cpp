#include "DataAnalysisInterface.h"

DataAnalysisInterface::DataAnalysisInterface(QObject *parent)
	: QObject(parent)
{
}

DataAnalysisInterface::~DataAnalysisInterface()
{
}

int DataAnalysisInterface::GetInt4Low(unsigned char *buf, int nIndex)
{
	int x = 0;
	x = buf[nIndex++];
	x += buf[nIndex++] * 0x100;
	x += buf[nIndex++] * 0x10000;
	x += buf[nIndex] * 0x1000000;
	if (x > 0x7FFFFFFF) x -= 0x100000000;
	return x;
}

int DataAnalysisInterface::GetInt2Low(unsigned char *buf, int nIndex)
{
	int x = 0;
	x = buf[nIndex++];
	x += buf[nIndex++] * 0x100;
	if (x > 0x7FFF) x -= 0x10000;
	return x;
}

int DataAnalysisInterface::GetUInt4Low(unsigned char *buf, int nIndex)
{
	int x = 0;
	x = buf[nIndex++];
	x += buf[nIndex++] * 0x100;
	x += buf[nIndex++] * 0x10000;
	x += buf[nIndex] * 0x1000000;
	return x;
}

int DataAnalysisInterface::GetUInt2Low(unsigned char *buf, int nIndex)
{
	int x = 0;
	x = buf[nIndex++];
	x += buf[nIndex++] * 0x100;
	return x;
}
