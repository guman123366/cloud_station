#pragma once

#include "circlebuffer_global.h"

class CIRCLEBUFFER_EXPORT CircleBuffer
{
public:
	CircleBuffer();

	bool isFull(int nLength);
	bool isEmpty(int nLength);
	void setData(unsigned char* data, int nLength);
	bool getData(unsigned char* buffer, int nLength);
	bool findData(unsigned char* cBuff, int nBegin, int nLength);
	int getStartPos(){ return m_nStartPos; }
	int getLength();
	bool dddd(int nSize);
private:
	unsigned char *m_BufferData;
	int m_nStartPos;
	int m_nEndPos;
};
