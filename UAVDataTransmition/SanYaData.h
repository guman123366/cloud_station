/*
TD220便携控制软件遥测数据
*/
#pragma once

#include "DataDefineInterface.h"

struct SanYaData :public DataDefineInterface
{
	SanYaData()
	{
		m_dLon = 0;
		m_dLat = 0;
		m_dAlt = 0;
	}
	double m_dLon;		//经度
	double m_dLat;		//纬度
	double m_dAlt;		//高度
};