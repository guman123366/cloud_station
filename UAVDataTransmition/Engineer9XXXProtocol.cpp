#include "Engineer9XXXProtocol.h"
#include "TD220TelemetryDataCar.h"
#include <qmath.h>

Engineer9XXXProtocol::Engineer9XXXProtocol(QObject *parent)
	: ConstructionProtocolInterface(parent)
{
	
}

Engineer9XXXProtocol::~Engineer9XXXProtocol()
{
}

int Engineer9XXXProtocol::ConstructionData(unsigned char* sendBuf)
{
	if (!ReceiveUAVData)
		return 0;

	TD220TelemetryDataCar* ProtocolData = (TD220TelemetryDataCar*)ReceiveUAVData;
	if (!ProtocolData)
		return 0;

	unsigned char UAVData[1024] = { 0 };
	unsigned int SendDataIndex = 0;
	static unsigned int ProtocolCounts = 0;
	unsigned int TempData = 0;

	if (ProtocolData->UAVID2 == 0)
	{
		return 0;
	}

	//帧头
	UAVData[SendDataIndex++] = 0xEB;
	UAVData[SendDataIndex++] = 0x90;
	//数据长度
	SendDataIndex += 4;
	//飞机号ID
	UAVData[SendDataIndex++] = ProtocolData->UAVID2;
	//计数
	ProtocolCounts++;
	memcpy(UAVData + SendDataIndex, &ProtocolCounts, 4);
	SendDataIndex += 4;
	//无人机纬度
	TempData = (ProtocolData->lat / 3600000.0) * 10000000;
	memcpy(UAVData + SendDataIndex, &TempData, 4);
	SendDataIndex += 4;
	//无人机经度
	TempData = (ProtocolData->lon / 3600000.0) * 10000000;
	memcpy(UAVData + SendDataIndex, &TempData, 4);
	SendDataIndex += 4;
	//无人机海拔高度
	TempData = (ProtocolData->GPSHeight*0.001 - 300) * 100;
	memcpy(UAVData + SendDataIndex, &TempData, 4);
	SendDataIndex += 4;
	//无人机北向速度
	TempData = (ProtocolData->SpeedN*0.01 - 50) * 100;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机东向速度
	TempData = (ProtocolData->SpeedE*0.01 - 50) * 100;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机天向速度
	TempData = -(ProtocolData->SpeedD*0.01 - 50) * 100;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机偏航角
	TempData = ProtocolData->Yaw;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机俯仰角
	TempData = (ProtocolData->pitch*0.0380859375 - 78.0) * 10;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机横滚角
	TempData = (ProtocolData->roll*0.0380859375 - 78.0) * 10;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机剩余油量
	TempData = ProtocolData->RecvOil;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//无人机发动机转速
	TempData = ProtocolData->RPM;
	memcpy(UAVData + SendDataIndex, &TempData, 2);
	SendDataIndex += 2;
	//控制车纬度
	TempData = ProtocolData->StationLat * 10000000;
	memcpy(UAVData + SendDataIndex, &TempData, 4);
	SendDataIndex += 4;
	//控制车经度
	TempData = ProtocolData->StationLon * 10000000;
	memcpy(UAVData + SendDataIndex, &TempData, 4);
	SendDataIndex += 4;
	//控制车海拔高度
	TempData = ProtocolData->StationAlt * 100;
	memcpy(UAVData + SendDataIndex, &TempData, 4);
	SendDataIndex += 4;

	int DataLength = SendDataIndex + 1;
	memcpy(UAVData + 2, &DataLength, 4);

	//和校验
	unsigned char SumChk = 0;
	for (int i = 0; i < SendDataIndex; i++)
	{
		SumChk += UAVData[i];
	}
	UAVData[SendDataIndex++] = SumChk;

	memcpy(sendBuf, UAVData, SendDataIndex);
	return SendDataIndex;
}
