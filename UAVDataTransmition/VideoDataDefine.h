/*
	สำฦตสýพÝ
*/
#pragma once

#include "DataDefineInterface.h"
#include <iostream>

struct VideoData :public DataDefineInterface
{
	VideoData()
	{
		VideoBuf = new unsigned char[100000];
		memset(VideoBuf, 0, 100000);
		VideoBufLength = 0;
	}
	unsigned char *VideoBuf;
	int VideoBufLength;
};