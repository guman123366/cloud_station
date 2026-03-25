#include "TD220DataAnalysis.h"
#include "../UAVDataTransmition/TD220TelemetryData.h"

TD220DataAnalysis::TD220DataAnalysis(QObject *parent)
	: DataAnalysisInterface(parent)
{
	memset(m_rxCheckBuf,0 , 2048);
}

TD220DataAnalysis::~TD220DataAnalysis()
{
}

DataDefineInterface* TD220DataAnalysis::AnalyseData(QByteArray ary, int nLength)
{
	static enum DecoderState state = DECODER_STATE_SYNC;
	static int flag55 = FALSE, i = 0;
	static unsigned short len = 0;
	static unsigned char tofrom = 0;
	static unsigned char rxbuf[1024];
	static unsigned short checksum = 0;

	unsigned char m_rebuf[1024] = { 0 };
	memcpy(m_rebuf, ary.data(), nLength);

	unsigned char data = 0;
	
	for (int kk = 0; kk < nLength; kk++)
	{
		data = m_rebuf[kk];
		if (state == DECODER_STATE_SYNC)
		{
			if (data == ZHZ_GLOBAL_IDA5)//如果 接收到的 是 帧头
			{
				state = DECODER_STATE_ADR;
				flag55 = FALSE;
			}
		}
		else
		{
			if (data == ZHZ_GLOBAL_IDA5)
			{//数据中有A5,证明数据中有错误
				state = DECODER_STATE_ADR;
				flag55 = FALSE;
				continue;
			}

			if (!flag55 && (data == ZHZ_GLOBAL_ID55))
			{
				flag55 = TRUE;
			}
			else
			{
				if (flag55)
				{
					if (data == 0x00)
					{
						data = ZHZ_GLOBAL_IDA5;//ID = 0xA5
					}
					flag55 = FALSE;
				}
				if (state == DECODER_STATE_ADR)
				{
					tofrom = data;
					state = DECODER_STATE_LEN;
				}
				else if (state == DECODER_STATE_LEN)
				{
					len = (data & 0xFF);
					i = 0;
					checksum = 0;
					state = DECODER_STATE_PAYLOAD;
				}
				else if (state == DECODER_STATE_PAYLOAD)
				{
					rxbuf[i++] = data;
					if (i >= len)
					{
						state = DECODER_STATE_CRC_1;
					}
				}
				else if (state == DECODER_STATE_CRC_1)
				{
					checksum += data;
					state = DECODER_STATE_CRC_2;
				}
				else if (state == DECODER_STATE_CRC_2)
				{
					checksum += data * 0x100;

					m_rxCheckBuf[0] = tofrom;
					m_rxCheckBuf[1] = char(len & 0xFF);

					memcpy(m_rxCheckBuf + 2, rxbuf, len);
					unsigned short sub = crc16Accumulate((char*)m_rxCheckBuf, len + 2);

					if (sub == checksum)
					{
						//接收到完整的数据 开始解析
						i = 0;
						if (tofrom == 0x00)
						{
							AnalyseDataDefine = new TD220TelemetryData;
							analyseTelemetryData(rxbuf, i, len);
							return AnalyseDataDefine;
						}
					}

					state = DECODER_STATE_SYNC;
					//port->clear(QSerialPort::Input);
				}
				else
				{
					state = DECODER_STATE_SYNC;
				}
			}
		}//

	}

	nLength = 0;
	return NULL;
}

unsigned short TD220DataAnalysis::crc16Accumulate(char * data, unsigned int nLength)
{
	if (nLength == 0)
	{
		return 0xFFFF;
	}

	//
	unsigned short crc = 0xFFFF;

	for (int i = 0; i < nLength; i++)
	{
		crc ^= data[i] << 8;
		crc = crc & 0xFFFF;
		for (int j = 0; j < 8; j++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc = (crc << 1) & 0xFFFF;
			}
		}
	}

	return crc;
}

DataDefineInterface* TD220DataAnalysis::analyseTelemetryData(unsigned char* rxbuf, int i, const int len)
{
	//解析Telemetry数据
	while (i<len)
	{
		int a = rxbuf[i++];
		switch (a)
		{
		case TelemetryDataID_POSNAVREL://97	0x61 IMU相对位置
		{
			AnalyseRelPosition(rxbuf, i);
			i += (4 * 3);
			break;
		}
		case TelemetryDataID_VELNAV://98	0x62
		{
			AnalyseVelnav(rxbuf, i);
			i += (4 * 3);
			break;
		}
		case TelemetryDataID_POSGPSABS://104	0x68 GPS绝对位置
		{
			double dAbsLat = (double)(GetInt4Low(rxbuf, i)) / 1.0e8; i += 4;
			double dAbsLon = (double)(GetInt4Low(rxbuf, i)) / 1.0e8; i += 4;
			double dAbsAlt = (double)(GetInt4Low(rxbuf, i)) / 1.0e3; i += 4;

			AnalyseDataDefine->UAVAbsPosition.m_dLat = QString::number(dAbsLat, 'f', 8).toDouble();
			AnalyseDataDefine->UAVAbsPosition.m_dLon = QString::number(dAbsLon, 'f', 8).toDouble();
			AnalyseDataDefine->UAVAbsPosition.m_dAlt = QString::number(dAbsAlt, 'f', 1).toDouble();
			break;
		}
		case TelemetryDataID_AIRCRAFT:		//106	0x6A
		{
			AnalyseDataDefine->UAVAircraft.m_dBatServos = (double)(GetInt2Low(rxbuf, i)) / 100.0; i += 2;
			AnalyseDataDefine->UAVAircraft.m_dBatFCS = (double)(GetInt2Low(rxbuf, i)) / 100.0; i += 2;
			//加0.5是为了四舍五入
			int rotorRPM = int(0.5 + (GetInt2Low(rxbuf, i)) / 10.0); i += 2;
			AnalyseDataDefine->UAVAircraft.m_nEngineTemp = (GetInt2Low(rxbuf, i)) / 10.0; i += 2;
			AnalyseDataDefine->UAVAircraft.m_nRotorRPM = rotorRPM;

			break;
		}
		case TelemetryDataID_STATUS:		//107	0x6B
		{
			int status[2] = { 0 };
			status[0] = GetInt4Low(rxbuf, i); i += 4;
			status[1] = GetInt4Low(rxbuf, i); i += 4;

			AnalyseStatusAlarmInfo(status[0]);
			AnalyseStatusServoLinkInfo(status[1]);

			AnalyModeInfo(status[0]);

			int configDataValid = (status[1] >> 19) & 0x01;
			int magDeclValid = (status[1] >> 20) & 0x01;

			break;

		}
		case TelemetryDataID_GUIDANCE:		//108	0x6C
		{
			int guidance[2] = { 0 };

			guidance[0] = GetInt2Low(rxbuf, i); i += 2;
			guidance[1] = GetInt2Low(rxbuf, i); i += 2;

			AnalyseDataDefine->WayPointInfo.m_nWPNo = (guidance[0] >> 8) & 0xFF;//下一个航点的编号
			AnalyseDataDefine->WayPointInfo.m_dDist = (double)(guidance[1]) / 6.5535;//到下一个航点的横向距离

			if (guidance[0] & 0x40)//航线 列表 是否为空
			{
				AnalyseDataDefine->WayPointInfo.m_nIsValid = 1;
			}
			else
			{
				AnalyseDataDefine->WayPointInfo.m_nIsValid = 0;
			}
			break;
		}
		case TelemetryDataID_ATTITUDE:		//150	0x96
		{
			AnalyseAttitude(rxbuf, i);
			i += (2 * 3);
			break;
		}
		case TelemetryDataID_CONTROL:		//159	0x9F 解析遥控器手柄数据
		{
			AnalyseControl(rxbuf, i);
			i += (2 * 5);
			break;
		}
		case TelemetryDataID_REFERENCE:		//161	0xA1
		{
			AnalyseReference(rxbuf, i);
			i += (4 * 8);
			break;
		}
		case TelemetryDataID_ServoCurrent:	//175	0xAF	舵机位置 舵机电流
		{
			AnalyseServoPosition(rxbuf, i);
			i += (2 * 6);
			break;
		}
		case TelemetryDataID_ADValue:		//176 0xB0
		{
			AnalyseADValue(rxbuf, i);
			i += (2 * 6);
			break;
		}
		case TelemetryDataID_AirSpeedVal://177
		{
			i += (10 * 4);
			break;
		}
		case TelemetryDataID_GPS_BD_Status://178 0xB2 GPS/BD STATUS
		{
			i += 4;
			break;
		}
		default:
			break;
		}
	}

	return AnalyseDataDefine;
}

void TD220DataAnalysis::AnalyseRelPosition(unsigned char* rxbuf, const int & nDataStartIndex)
{
	int i = nDataStartIndex;
	AnalyseDataDefine->UAVRelPosition.m_dNorth = (double)(GetInt4Low(rxbuf, i)) / 6.5535;
	i += 4;//IMU 相对位置 北
	AnalyseDataDefine->UAVRelPosition.m_dEast = (double)(GetInt4Low(rxbuf, i)) / 6.5535;
	i += 4;//IMU 相对位置 东
	AnalyseDataDefine->UAVRelPosition.m_dDown = (double)(GetInt4Low(rxbuf, i)) / 6.5535;
	i += 4;//IMU 相对位置 下
}

void TD220DataAnalysis::AnalyseVelnav(unsigned char* rxbuf, const int & nDataStartIndex /*= 0*/)
{
	int i = nDataStartIndex;
	AnalyseDataDefine->UAVVelocity.m_dNorth = (double)(GetInt4Low(rxbuf, i)) / 655.47;
	i += 4;//IMU 相对速度 北
	AnalyseDataDefine->UAVVelocity.m_dEast = (double)(GetInt4Low(rxbuf, i)) / 655.47;
	i += 4;//IMU 相对速度 东
	AnalyseDataDefine->UAVVelocity.m_dDown = (double)(GetInt4Low(rxbuf, i)) / 655.47;
	i += 4;//IMU 相对速度 下
}

void TD220DataAnalysis::AnalyseReference(unsigned char* rxbuf, const int & nDataStartIndex /*= 0*/)
{
	int i = nDataStartIndex;
	int bytes = 4;

	AnalyseDataDefine->UAVRelVelocity.m_dVel_north = (double)(GetInt4Low(rxbuf, i)) / 655.47;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dVel_east = (double)(GetInt4Low(rxbuf, i)) / 655.47;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dVel_down = (double)(GetInt4Low(rxbuf, i)) / 655.47;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dHeading = (double)(GetInt4Low(rxbuf, i)) / 10000.0;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dPos_north = (double)(GetInt4Low(rxbuf, i)) / 6.5535;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dPos_east = (double)(GetInt4Low(rxbuf, i)) / 6.5535;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dPos_down = (double)(GetInt4Low(rxbuf, i)) / 6.5535;
	i += bytes;
	AnalyseDataDefine->UAVRelVelocity.m_dHeading_rate = (double)(GetInt4Low(rxbuf, i)) / 10000.0;
}

void TD220DataAnalysis::AnalyseAttitude(unsigned char* szData, const int & nDataStartIndex /*= 0*/)
{
	int i = nDataStartIndex;
	AnalyseDataDefine->UAVAttitude.roll = (double)(GetInt2Low(szData, i)) / 10000.0;
	i += 2;
	AnalyseDataDefine->UAVAttitude.pitch = (double)(GetInt2Low(szData, i)) / 10000.0;
	i += 2;
	AnalyseDataDefine->UAVAttitude.yaw = (double)(GetInt2Low(szData, i)) / 10000.0;
}

void TD220DataAnalysis::AnalyseControl(unsigned char* szData, const int & nDataStartIndex /*= 0*/)
{
	int nDataIndex = nDataStartIndex;

	AnalyseDataDefine->ControlSignals.m_dA1s = (double)(GetInt2Low(szData, nDataIndex)) / 100.0;
	nDataIndex += 2;
	AnalyseDataDefine->ControlSignals.m_dB1s = (double)(GetInt2Low(szData, nDataIndex)) / 100.0;
	nDataIndex += 2;
	AnalyseDataDefine->ControlSignals.m_dAm = (double)(GetInt2Low(szData, nDataIndex)) / 100.0;
	nDataIndex += 2;
	AnalyseDataDefine->ControlSignals.m_dAt = (double)(GetInt2Low(szData, nDataIndex)) / 100.0;
	nDataIndex += 2;
	AnalyseDataDefine->ControlSignals.m_nTh = int(0.5 + (GetInt2Low(szData, nDataIndex)) / 100.0);
}

void TD220DataAnalysis::AnalyseServoPosition(unsigned char* rxbuf, const int & nDataStartIndex /*= 0*/)
{
	int nDataIndex = nDataStartIndex;

	AnalyseDataDefine->ServoPostion.pitch = GetInt2Low(rxbuf, nDataIndex) / 1000.0;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->ServoPostion.roll = GetInt2Low(rxbuf, nDataIndex) / 1000.0;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->ServoPostion.collection = GetInt2Low(rxbuf, nDataIndex) / 1000.0;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->ServoPostion.throttle = GetInt2Low(rxbuf, nDataIndex) / 1000.0;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->ServoPostion.heading = GetInt2Low(rxbuf, nDataIndex) / 1000.0;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->ServoPostion.rudder = GetInt2Low(rxbuf, nDataIndex) / 1000.0;
}

void TD220DataAnalysis::AnalyseADValue(unsigned char* szData, const int & nDataStartIndex /*= 0*/)
{
	int nDataIndex = nDataStartIndex;

	AnalyseDataDefine->UAVADValue.m_dAirTemp1 = GetInt2Low(szData, nDataIndex)*1000.0 / 4096.0 - 100;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->UAVADValue.m_dAirTemp2 = GetInt2Low(szData, nDataIndex)*1000.0 / 4096.0 - 100;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->UAVADValue.m_dAirSpeed = GetInt2Low(szData, nDataIndex)*1000.0 / 4096.0 - 100;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->UAVADValue.m_dWaterTemp = GetInt2Low(szData, nDataIndex)*1000.0 / 4096.0 - 100;
	nDataIndex = nDataIndex + 2;
	AnalyseDataDefine->UAVADValue.m_dCpuTemp = GetInt2Low(szData, nDataIndex)*1000.0 / 4096.0 - 100;
	nDataIndex = nDataIndex + 2;

	double dLastOil = GetInt2Low(szData, nDataIndex)*1000.0 / 4096.0 - 100;
	AnalyseDataDefine->UAVADValue.m_dLastOil = dLastOil / 10;
}

void TD220DataAnalysis::AnalyModeInfo(const int szData)
{
	//转速传感器1状态
	int RpmState1 = szData & 0x010000;
	if (RpmState1 == 0)
	{
		AnalyseDataDefine->UAVModeInfo.m_RpmState1 = std::string("有效");
	}
	else
	{
		AnalyseDataDefine->UAVModeInfo.m_RpmState1 = std::string("失效");
	}
	//转速传感器2状态
	int RpmState2 = szData & 0x020000;
	if (RpmState2 == 0)
	{
		AnalyseDataDefine->UAVModeInfo.m_RpmState2 = std::string("有效");
	}
	else
	{
		AnalyseDataDefine->UAVModeInfo.m_RpmState2 = std::string("失效");
	}

	//卫星定位模式
	int SatelliteMode = szData & 0x0C0000;
	if (SatelliteMode == 0)
	{
		AnalyseDataDefine->UAVModeInfo.m_SatelliteMode = std::string("未定位");
	}
	else if (SatelliteMode == 0x040000)
	{
		AnalyseDataDefine->UAVModeInfo.m_SatelliteMode = std::string("单点");
	}
	else if (SatelliteMode == 0x080000)
	{
		AnalyseDataDefine->UAVModeInfo.m_SatelliteMode = std::string("伪距");
	}
	else
	{
		AnalyseDataDefine->UAVModeInfo.m_SatelliteMode = std::string("差分");
	}

	//导航模式
	int NavigationMode = szData & 0x0F00000;
	if (NavigationMode == 0)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("纯惯性导航");
	}
	else if (NavigationMode == 0x0100000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("惯导+卫导");
	}
	else if (NavigationMode == 0x0200000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("惯导+气压");
	}
	else if (NavigationMode == 0x0300000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("惯导+无线电+气压");
	}
	else if (NavigationMode == 0x0400000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("惯导+卫导+气压");
	}
	else if (NavigationMode == 0x0500000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("待机模式");
	}
	else if (NavigationMode == 0x0600000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("快速自对准");
	}
	else if (NavigationMode == 0x0700000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("正常自对准");
	}
	else if (NavigationMode == 0x0800000)
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("空中自对准");
	}
	else
	{
		AnalyseDataDefine->UAVModeInfo.m_NavigationMode = std::string("IMU+磁力计+卫导");
	}
}

void TD220DataAnalysis::AnalySensorInfo(const int szData)
{

}

void TD220DataAnalysis::AnalyseStatusAlarmInfo(const int szData)
{
	AnalyseDataDefine->UAVAircraft.m_nSat = (szData >> 8) & 0x0F;

	if (szData & 0x1000)
	{
		AnalyseDataDefine->UAVAircraft.GPSStatus = UAV_GPS_NOTVALID;
	}
	else if (szData & 0x2000)
	{
		AnalyseDataDefine->UAVAircraft.GPSStatus = UAV_GPS_LOST;
	}
	else if (szData & 0x4000)
	{
		AnalyseDataDefine->UAVAircraft.GPSStatus = UAV_GPS_VALID_DGPS;
	}
	else
	{
		AnalyseDataDefine->UAVAircraft.GPSStatus = UAV_GPS_VALID;
	}

	//遥控器连接状态
	if ((szData & 0x18000000) == 0x18000000)
	{
		AnalyseDataDefine->UAVAircraft.RCStatus = UAV_RC_DISABLED;
	}
	else if ((szData & 0x18000000) == 0x10000000)
	{
		AnalyseDataDefine->UAVAircraft.RCStatus = UAV_RC_LOST;
	}
	else
	{
		AnalyseDataDefine->UAVAircraft.RCStatus = UAV_RC_OK;
	}

	//燃料槽水平
	if ((szData & 0xC0000000) == 0xC0000000)
	{
		AnalyseDataDefine->UAVAircraft.m_nFuelLevel = 0;
	}
	else if ((szData & 0xC0000000) == 0x80000000)
	{
		AnalyseDataDefine->UAVAircraft.m_nFuelLevel = 0;
	}
	else
	{
		AnalyseDataDefine->UAVAircraft.m_nFuelLevel = 100;
	}
}

void TD220DataAnalysis::AnalyseStatusServoLinkInfo(const int szData)
{
	AnalyseDataDefine->ServoLinkInfo.m_nServolinkNOK = (szData >> 22) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nServolinkLost = (szData >> 23) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nWatchdogReset = (szData >> 24) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nSoftwareReset = (szData >> 25) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nPoweronReset = (szData >> 26) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nAp2servolinkNOK = (szData >> 27) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nAp2servoChkErr = (szData >> 28) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nWatchdogResetFlag = (szData >> 29) & 1;
	AnalyseDataDefine->ServoLinkInfo.m_nSoftwareResetFlag = (szData >> 30) & 1;
}
