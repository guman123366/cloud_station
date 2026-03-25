#include "TD220DataAnalysisCarCommon.h"

const unsigned char START1 = 0x81, START2 = 0x82, START3 = 0x83, START4 = 0x84;
const unsigned char SYNC1 = 0x55, SYNC2 = 0XAA, SYNC3 = 0XEB, SYNC4 = 0X90;

TD220DataAnalysisCarCommon::TD220DataAnalysisCarCommon(QObject *parent)
	: DataAnalysisInterface(parent), ReceiveDataIndex(0), m_pTD220TelemetryDataCar(NULL)
{

}

TD220DataAnalysisCarCommon::~TD220DataAnalysisCarCommon()
{
}

DataDefineInterface* TD220DataAnalysisCarCommon::AnalyseData(QByteArray ary, int nLength)
{
	unsigned char TelemetryData[1024] = { 0 };
	unsigned char ParseData[1024] = { 0 };
	bool bRevFrame1 = 0, bRevFrame2 = 0, bRevFrame3 = 0, bRevFrame4 = 0;
	int TelemetryDataLength = nLength;

	if ((0xEB == ary.at(0)) && (0x90 == ary.at(1)) && (0x81 == ary.at(2)) && (256 == TelemetryDataLength))
	{
		//将0xEB和0x90移到帧尾
		memcpy(TelemetryData, ary.data()+2, TelemetryDataLength-2);
		TelemetryData[254] = 0XEB;
		TelemetryData[255] = 0X90;

		m_pTD220TelemetryDataCar = new TD220TelemetryDataCar;
		
		//读取遥测复合数据帧，飞行遥测数据。遥测数据包含四个副帧
		if (TelemetryData[0] == START1 && TelemetryData[30] == SYNC1 && TelemetryData[31] == SYNC2 && TelemetryData[32] == START1 && TelemetryData[62] == SYNC3 && TelemetryData[63] == SYNC4)
		{
			bRevFrame1 = 1;
		}
		else bRevFrame1 = 0;
		if (TelemetryData[64] == START2 && TelemetryData[94] == SYNC1 && TelemetryData[95] == SYNC2 && TelemetryData[96] == START2 && TelemetryData[126] == SYNC3 && TelemetryData[127] == SYNC4)
		{
			bRevFrame2 = 1;
		}
		else bRevFrame2 = 0;
		if (TelemetryData[128] == START3 && TelemetryData[158] == SYNC1 && TelemetryData[159] == SYNC2 && TelemetryData[160] == START3 && TelemetryData[190] == SYNC3 && TelemetryData[191] == SYNC4)
		{
			bRevFrame3 = 1;
		}
		else bRevFrame3 = 0;
		if (TelemetryData[192] == START4 && TelemetryData[222] == SYNC1 && TelemetryData[223] == SYNC2 && TelemetryData[224] == START4 && TelemetryData[254] == SYNC3 && TelemetryData[255] == SYNC4)
		{
			bRevFrame4 = 1;
		}
		else bRevFrame4 = 0;

		if (bRevFrame1&&bRevFrame2&&bRevFrame3&&bRevFrame4)
		{
			memcpy(ParseData, &TelemetryData[0], 256);
			ParseTelemetryData(ParseData, 256);
			bRevFrame1 = 0;
			bRevFrame2 = 0;
			bRevFrame3 = 0;
			bRevFrame4 = 0;
		}

		ReceiveDataIndex = 0;

		return m_pTD220TelemetryDataCar;
	}
}

void TD220DataAnalysisCarCommon::ParseTelemetryData(unsigned char* buf, int nLength)
{
	unsigned char HandleData[1024] = { 0 };
	int HandleDataLength = nLength;
	unsigned char BufTemp[1024] = { 0 };
	unsigned char ch = 0;
	int cnt = 0;
	memcpy(HandleData, buf, HandleDataLength);
	DecoderState decoderState = DECODER_STATE_SYNC0;
	int AnalysisIndex = 0;
	unsigned short DataCrc = 0;
	char crcData[56] = { 0 };
	unsigned short calCrc = 0;
	while (AnalysisIndex < HandleDataLength)
	{
		ch = HandleData[AnalysisIndex];
		if (decoderState == DECODER_STATE_SYNC0)
		{
			if (ch == START1 || ch == START2 || ch == START3 || ch == START4)//帧头是0x81,0x82,0x83,0x84
			{
				cnt = 0;
				BufTemp[cnt++] = ch;
				decoderState = DECODER_STATE_SYNC1;//81,82,83,84
				AnalysisIndex++;
			}
		}
		else if (decoderState == DECODER_STATE_SYNC1)
		{
			//判断如果计数超过协议规定同步字位置，则丢弃该帧
			if (cnt > 0x1F)
			{
				decoderState = DECODER_STATE_SYNC0;
			}
			if (cnt == 0x1F)//根据协议，找到同步字所在位置
			{
				BufTemp[cnt++] = ch;
				AnalysisIndex++;
				if (BufTemp[0x1E] == 0x55 && BufTemp[0x1F] == 0xAA)//同步字匹配
				{
					decoderState = DECODER_STATE_SYNC2;//81
				}
				else
				{
					decoderState = DECODER_STATE_SYNC0;
				}
			}
			else
			{
				BufTemp[cnt++] = ch;
				AnalysisIndex++;
			}
		}
		else if (decoderState == DECODER_STATE_SYNC2)//帧头状态
		{
			if (ch == BufTemp[0])//81,82,83,84
			{
				decoderState = DECODER_STATE_SYNC3;
				BufTemp[cnt++] = ch;
				AnalysisIndex++;
			}
			else
			{
				decoderState = DECODER_STATE_SYNC0;
			}
		}
		else if (decoderState == DECODER_STATE_SYNC3)//同步字状态
		{
			//判断如果计数超过协议规定同步字位置，则丢弃该帧
			if (cnt > 0x3F)
			{
				decoderState = DECODER_STATE_SYNC0;
			}
			if (cnt == 0x3F)
			{
				BufTemp[cnt++] = ch;
				AnalysisIndex++;
				if (BufTemp[0x3E] == SYNC3&&BufTemp[0x3F] == SYNC4)
				{
					if (BufTemp[0] == START1)//第一副帧原始数据
					{
						//载荷数据
						memcpy(m_pTD220TelemetryDataCar->PodData, BufTemp + 12, 8);
						memcpy(m_pTD220TelemetryDataCar->PodData + 8, BufTemp + 44, 8);

						DataCrc = 0;
						DataCrc = BufTemp[0x21] * 0x100 + BufTemp[0x0b];
						memset(crcData, 0, 56);
						memset(&BufTemp[0x0c], 0, 8);
						memset(&BufTemp[0x2c], 0, 8);
						memcpy(crcData, &BufTemp[1], 10);
						memcpy(&crcData[10], &BufTemp[0x0c], 18);
						memcpy(&crcData[28], &BufTemp[34], 28);
						calCrc = cal_crc(crcData, 56);
						if (DataCrc == calCrc)
						{
							m_pTD220TelemetryDataCar->UAVID1 = BufTemp[0x02];
							m_pTD220TelemetryDataCar->Status1 = BufTemp[0x06] * 0x1000000 + BufTemp[0x05] * 0x10000 + BufTemp[0x04] * 0x100 + BufTemp[0x03];
							m_pTD220TelemetryDataCar->PositionN = BufTemp[0x0A] * 0x1000000 + BufTemp[0x09] * 0x10000 + BufTemp[0x08] * 0x100 + BufTemp[0x07];
							m_pTD220TelemetryDataCar->PositionE = BufTemp[0x17] * 0x1000000 + BufTemp[0x16] * 0x10000 + BufTemp[0x15] * 0x100 + BufTemp[0x14];
							m_pTD220TelemetryDataCar->GPSHeight = BufTemp[0x1B] * 0x1000000 + BufTemp[0x1A] * 0x10000 + BufTemp[0x19] * 0x100 + BufTemp[0x18];
							m_pTD220TelemetryDataCar->BaseVal = BufTemp[0x1D] * 0x100 + BufTemp[0x1C];
							m_pTD220TelemetryDataCar->ReBackCMD = BufTemp[0x22];
							m_pTD220TelemetryDataCar->ReBackState = BufTemp[0x23];
							m_pTD220TelemetryDataCar->GuidanceMod = BufTemp[0x24];
							m_pTD220TelemetryDataCar->Status2 = BufTemp[0x28] * 0x1000000 + BufTemp[0x27] * 0x10000 + BufTemp[0x26] * 0x100 + BufTemp[0x25];
							m_pTD220TelemetryDataCar->ServoVal = BufTemp[0x29];
							m_pTD220TelemetryDataCar->CPUtemp = BufTemp[0x2B] * 0x100 + BufTemp[0x2A];
							m_pTD220TelemetryDataCar->Yaw = BufTemp[0x35] * 0x100 + BufTemp[0x34];
							m_pTD220TelemetryDataCar->pitch = BufTemp[0x37] * 0x100 + BufTemp[0x36];
							m_pTD220TelemetryDataCar->roll = BufTemp[0x39] * 0x100 + BufTemp[0x38];
							m_pTD220TelemetryDataCar->PositionD = BufTemp[0x3B] * 0x100 + BufTemp[0x3A];
							m_pTD220TelemetryDataCar->Airspeed = BufTemp[0x3D] * 0x100 + BufTemp[0x3C];
						}
					}
					else if (BufTemp[0] == START2)//第二副帧原始数据
					{
						DataCrc = 0;
						DataCrc = BufTemp[0x35] * 0x100 + BufTemp[0x34];
						memset(crcData, 0, 56);
						memcpy(crcData, &BufTemp[1], 29);
						memcpy(&crcData[29], &BufTemp[33], 19);
						memcpy(&crcData[48], &BufTemp[0x36], 8);
						calCrc = cal_crc(crcData, 56);
						if (DataCrc == calCrc)
						{
							m_pTD220TelemetryDataCar->UAVID2 = BufTemp[0x02];
							m_pTD220TelemetryDataCar->RefSpeedN = BufTemp[0x04] * 0x100 + BufTemp[0x03];
							m_pTD220TelemetryDataCar->RefSpeedE = BufTemp[0x06] * 0x100 + BufTemp[0x05];
							m_pTD220TelemetryDataCar->RefSpeedD = BufTemp[0x08] * 0x100 + BufTemp[0x07];
							m_pTD220TelemetryDataCar->RefHeading_rate = BufTemp[0x0A] * 0x100 + BufTemp[0x09];
							m_pTD220TelemetryDataCar->RefPositionN = BufTemp[0x0E] * 0x1000000 + BufTemp[0x0D] * 0x10000 + BufTemp[0x0C] * 0x100 + BufTemp[0x0B];
							m_pTD220TelemetryDataCar->RefPositionE = BufTemp[0x12] * 0x1000000 + BufTemp[0x11] * 0x10000 + BufTemp[0x10] * 0x100 + BufTemp[0x0F];
							m_pTD220TelemetryDataCar->RefPositionD = BufTemp[0x16] * 0x1000000 + BufTemp[0x15] * 0x10000 + BufTemp[0x14] * 0x100 + BufTemp[0x13];
							m_pTD220TelemetryDataCar->RefPositionYaw = BufTemp[0x18] * 0x100 + BufTemp[0x17];
							m_pTD220TelemetryDataCar->AirSpeedtemp = BufTemp[0x1A] * 0x100 + BufTemp[0x19];
							m_pTD220TelemetryDataCar->AirSpeedHeight = BufTemp[0x1D] * 0x10000 + BufTemp[0x1C] * 0x100 + BufTemp[0x1B];
							m_pTD220TelemetryDataCar->PositionD1 = BufTemp[0x23] * 0x100 + BufTemp[0x22];
							m_pTD220TelemetryDataCar->Throttle = BufTemp[0x24];
							m_pTD220TelemetryDataCar->MotorTemp = BufTemp[0x27] * 0x100 + BufTemp[0x26];
							m_pTD220TelemetryDataCar->ErrorSplitCode = BufTemp[0x28];
							m_pTD220TelemetryDataCar->GeoCourse = BufTemp[0x32] * 0x100 + BufTemp[0x31];
							if (m_pTD220TelemetryDataCar->ErrorSplitCode == 0)
							{
								m_pTD220TelemetryDataCar->ErrorZCode = BufTemp[0x2B] * 0x10000 + BufTemp[0x2A] * 0x100 + BufTemp[0x29];
							}
							else if (m_pTD220TelemetryDataCar->ErrorSplitCode == 1)
							{
								m_pTD220TelemetryDataCar->ErrorFCode = BufTemp[0x2B] * 0x10000 + BufTemp[0x2A] * 0x100 + BufTemp[0x29];
							}
							m_pTD220TelemetryDataCar->SplitCode1 = BufTemp[0x2C];
							if (m_pTD220TelemetryDataCar->SplitCode1 == 0x21)
							{
								m_pTD220TelemetryDataCar->lat = BufTemp[0x30] * 0x1000000 + BufTemp[0x2F] * 0x10000 + BufTemp[0x2E] * 0x100 + BufTemp[0x2D];
								m_pTD220TelemetryDataCar->WaypointNum = BufTemp[0x33];
							}
							else if (m_pTD220TelemetryDataCar->SplitCode1 == 0x22)
							{
								m_pTD220TelemetryDataCar->lon = BufTemp[0x30] * 0x1000000 + BufTemp[0x2F] * 0x10000 + BufTemp[0x2E] * 0x100 + BufTemp[0x2D];
							}
							m_pTD220TelemetryDataCar->Pdop = BufTemp[0x37] * 0x100 + BufTemp[0x36];
							m_pTD220TelemetryDataCar->YawAcc = BufTemp[0x39] * 0x100 + BufTemp[0x38];
							m_pTD220TelemetryDataCar->pitchAcc = BufTemp[0x3B] * 0x100 + BufTemp[0x3A];
							m_pTD220TelemetryDataCar->rollAcc = BufTemp[0x3D] * 0x100 + BufTemp[0x3C];
							m_pTD220TelemetryDataCar->Waypotion = BufTemp[0x25];
						}
					}
					else if (BufTemp[0] == START3)//第三副帧原始数据
					{
						//载荷数据
						memcpy(m_pTD220TelemetryDataCar->PodData + 16, BufTemp + 12, 8);
						memcpy(m_pTD220TelemetryDataCar->PodData + 24, BufTemp + 44, 8);

						DataCrc = 0;
						DataCrc = BufTemp[0x3d] * 0x100 + BufTemp[0x3c];
						memset(crcData, 0, 56);
						memset(&BufTemp[0x0c], 0, 8);
						memset(&BufTemp[0x2c], 0, 8);
						memcpy(crcData, &BufTemp[1], 29);
						memcpy(&crcData[29], &BufTemp[33], 27);
						calCrc = cal_crc(crcData, 56);
						if (DataCrc == calCrc)
						{
							m_pTD220TelemetryDataCar->UAVID3 = BufTemp[0x02];
							m_pTD220TelemetryDataCar->guidanceXAcc = BufTemp[0x04] * 0x100 + BufTemp[0x03];
							m_pTD220TelemetryDataCar->guidanceYAcc = BufTemp[0x06] * 0x100 + BufTemp[0x05];
							m_pTD220TelemetryDataCar->guidanceZAcc = BufTemp[0x08] * 0x100 + BufTemp[0x07];
							m_pTD220TelemetryDataCar->U1ServoPos = BufTemp[0x0A] * 0x100 + BufTemp[0x09];
							m_pTD220TelemetryDataCar->Fan_rpm1 = BufTemp[0x0B];
							m_pTD220TelemetryDataCar->Fan_rpm2 = BufTemp[0x2B];
							m_pTD220TelemetryDataCar->guidanceSpeedD = BufTemp[0x15] * 0x100 + BufTemp[0x14];
							m_pTD220TelemetryDataCar->U2ServoPos = BufTemp[0x17] * 0x100 + BufTemp[0x16];
							m_pTD220TelemetryDataCar->U3ServoPos = BufTemp[0x19] * 0x100 + BufTemp[0x18];
							m_pTD220TelemetryDataCar->D1ServoPos = BufTemp[0x1B] * 0x100 + BufTemp[0x1A];
							m_pTD220TelemetryDataCar->D2ServoPos = BufTemp[0x1D] * 0x100 + BufTemp[0x1C];
							m_pTD220TelemetryDataCar->RPM = BufTemp[0x22] * 0x100 + BufTemp[0x21];
							m_pTD220TelemetryDataCar->D3ServoPos = BufTemp[0x24] * 0x100 + BufTemp[0x23];

							m_pTD220TelemetryDataCar->RecvOil = BufTemp[0x26] * 0x100 + BufTemp[0x25];
							m_pTD220TelemetryDataCar->WaterTemp = BufTemp[0x2A] * 0x100 + BufTemp[0x29];
							m_pTD220TelemetryDataCar->AfterGeo = BufTemp[0x28] * 0x100 + BufTemp[0x27];
							m_pTD220TelemetryDataCar->U0Con = BufTemp[0x35] * 0x100 + BufTemp[0x34];
							m_pTD220TelemetryDataCar->U1Con = BufTemp[0x37] * 0x100 + BufTemp[0x36];
							m_pTD220TelemetryDataCar->U2Con = BufTemp[0x39] * 0x100 + BufTemp[0x38];
							m_pTD220TelemetryDataCar->U3Con = BufTemp[0x3B] * 0x100 + BufTemp[0x3A];
						}
					}
					else if (BufTemp[0] == START4)//第四副帧原始数据
					{
						DataCrc = 0;
						DataCrc = BufTemp[0x3d] * 0x100 + BufTemp[0x3c];
						memset(crcData, 0, 56);
						memcpy(crcData, &BufTemp[1], 29);
						memcpy(&crcData[29], &BufTemp[33], 27);
						calCrc = cal_crc(crcData, 56);
						if (DataCrc == calCrc)
						{
							if (BufTemp[0x3b] == 0x01)
							{
								m_pTD220TelemetryDataCar->UAVID4 = BufTemp[0x02];
								m_pTD220TelemetryDataCar->SpeedN = BufTemp[0x04] * 0x100 + BufTemp[0x03];
								m_pTD220TelemetryDataCar->SpeedE = BufTemp[0x06] * 0x100 + BufTemp[0x05];
								m_pTD220TelemetryDataCar->SpeedD = BufTemp[0x08] * 0x100 + BufTemp[0x07];
								m_pTD220TelemetryDataCar->U1Current = BufTemp[0x0A] * 0x100 + BufTemp[0x09];
								m_pTD220TelemetryDataCar->U2Current = BufTemp[0x0C] * 0x100 + BufTemp[0x0B];
								m_pTD220TelemetryDataCar->U3Current = BufTemp[0x0E] * 0x100 + BufTemp[0x0D];
								m_pTD220TelemetryDataCar->D1Current = BufTemp[0x10] * 0x100 + BufTemp[0x0F];
								m_pTD220TelemetryDataCar->D2Current = BufTemp[0x12] * 0x100 + BufTemp[0x11];
								m_pTD220TelemetryDataCar->D3Current = BufTemp[0x22] * 0x100 + BufTemp[0x21];//修改
								m_pTD220TelemetryDataCar->XAcc = BufTemp[0x24] * 0x100 + BufTemp[0x23];
								m_pTD220TelemetryDataCar->YAcc = BufTemp[0x26] * 0x100 + BufTemp[0x25];
								m_pTD220TelemetryDataCar->ZAcc = BufTemp[0x28] * 0x100 + BufTemp[0x27];
								m_pTD220TelemetryDataCar->GPSsatNum = (BufTemp[0x29]) & 0x0F;//修改
								m_pTD220TelemetryDataCar->BDSatNum = (BufTemp[0x29] >> 4) & 0x0F;//修改
								m_pTD220TelemetryDataCar->DiffState = BufTemp[0x2A] & 0x0F;//修改
								m_pTD220TelemetryDataCar->InertialState = (BufTemp[0x2A] >> 4) & 0x0F;//修改
								m_pTD220TelemetryDataCar->SplitCode2 = BufTemp[0x2C];

								if (m_pTD220TelemetryDataCar->SplitCode2 == 0x41)
								{
									m_pTD220TelemetryDataCar->GPSSpeed = BufTemp[0x2F] * 0x100 + BufTemp[0x2E];
									m_pTD220TelemetryDataCar->WaypointDis = BufTemp[0x33] * 0x1000000 + BufTemp[0x32] * 0x10000 + BufTemp[0x31] * 0x100 + BufTemp[0x30];
								}
								m_pTD220TelemetryDataCar->SplitCode3 = BufTemp[0x34];
								if (m_pTD220TelemetryDataCar->SplitCode3 == 0x00)
								{
									m_pTD220TelemetryDataCar->PlaneType = BufTemp[0x35];
									m_pTD220TelemetryDataCar->PlaneNum = BufTemp[0x36];
								}
							}
							else if (BufTemp[0x3b] == 0x02)
							{
								m_pTD220TelemetryDataCar->BDstate = BufTemp[0x2b];
								m_pTD220TelemetryDataCar->B1SYear = BufTemp[0x2c];
								m_pTD220TelemetryDataCar->B1SMonth = BufTemp[0x2d];
								m_pTD220TelemetryDataCar->B1SDay = BufTemp[0x2e];
								m_pTD220TelemetryDataCar->B1EYear = BufTemp[0x2f];
								m_pTD220TelemetryDataCar->B1EMonth = BufTemp[0x30];
								m_pTD220TelemetryDataCar->B1EDay = BufTemp[0x31];
								m_pTD220TelemetryDataCar->B3SYear = BufTemp[0x32];
								m_pTD220TelemetryDataCar->B3SMonth = BufTemp[0x33];
								m_pTD220TelemetryDataCar->B3SDay = BufTemp[0x34];
								m_pTD220TelemetryDataCar->B3EYear = BufTemp[0x35];
								m_pTD220TelemetryDataCar->B3EMonth = BufTemp[0x36];
								m_pTD220TelemetryDataCar->B3EDay = BufTemp[0x37];
								m_pTD220TelemetryDataCar->ICYear = BufTemp[0x38];
								m_pTD220TelemetryDataCar->ICMonth = BufTemp[0x39];
								m_pTD220TelemetryDataCar->ICDay = BufTemp[0x3a];
							}
							else if (BufTemp[0x3b] == 0x03)
							{
								m_pTD220TelemetryDataCar->Geo0 = BufTemp[0x2c] * 0x100 + BufTemp[0x2b];
								m_pTD220TelemetryDataCar->Geo45 = BufTemp[0x2e] * 0x100 + BufTemp[0x2d];
								m_pTD220TelemetryDataCar->Geo90 = BufTemp[0x30] * 0x100 + BufTemp[0x2f];
								m_pTD220TelemetryDataCar->Geo135 = BufTemp[0x32] * 0x100 + BufTemp[0x31];
								m_pTD220TelemetryDataCar->Geo180 = BufTemp[0x34] * 0x100 + BufTemp[0x33];
								m_pTD220TelemetryDataCar->Geo225 = BufTemp[0x36] * 0x100 + BufTemp[0x35];
								m_pTD220TelemetryDataCar->Geo270 = BufTemp[0x38] * 0x100 + BufTemp[0x37];
								m_pTD220TelemetryDataCar->Geo315 = BufTemp[0x3a] * 0x100 + BufTemp[0x39];
							}
						}
					}
					decoderState = DECODER_STATE_SYNC0;
					memset(BufTemp, 0, 1024);
					cnt = 0;
				}
				else
				{
					decoderState = DECODER_STATE_SYNC0;
				}
			}
			else
			{
				BufTemp[cnt++] = ch;
				AnalysisIndex++;
			}
		}
	}
}

unsigned short TD220DataAnalysisCarCommon::cal_crc(char *data, unsigned int len)
{
	unsigned int i = 0, j = 0;
	unsigned short crc = 0xFFFF;
	for (i = 0; i < len; i++)
	{
		crc ^= data[i] << 8;
		crc = crc & 0xffff;
		for (j = 0; j < 8; j++)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc = (crc << 1) & 0xffff;
			}
		}
	}
	return crc;
}
