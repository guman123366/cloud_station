#include "TD550DataAnalysis.h"
#include "../UAVDataTransmition/TD550TelemetryData.h"

TD550DataAnalysis::TD550DataAnalysis(QObject* parent)
	: DataAnalysisInterface(parent)
{
	m_nDataSize = 0;
	m_nBufSize = 4096 * 5;
	m_pBuf = new unsigned char[m_nBufSize];
	m_pBufSwap = new unsigned char[m_nBufSize];
	m_nFlag = 0;
	memset(m_arrDecodeData, 0, 256);

	m_TD550TelemetryData = new TD550TelemetryData;
}

TD550DataAnalysis::~TD550DataAnalysis()
{
	if (m_TD550TelemetryData)
		delete m_TD550TelemetryData;
}

unsigned short TD550DataAnalysis::getSendFCCVersion()
{
	return m_FCCVersion;
}

DataDefineInterface* TD550DataAnalysis::AnalyseData(QByteArray ary, int nLength)
{
	if ((nLength > m_nBufSize) || (nLength < 0))
		return nullptr;

	static Decode450State decodeState = Decode_Head81;
	static unsigned char decodeData[256] = { 0 };
	static unsigned int decodeDataLength = 0;
	memcpy(m_pBuf, (unsigned char*)ary.data(), nLength);
	m_nDataSize = nLength;

	int nIndex = 0;
	while (nIndex < m_nDataSize)
	{
		//若decodeData数组已满，仍未找到一帧完整的数据，应将所有数据和状态清空
		if (decodeDataLength >= 256)
		{
			decodeDataLength = 0;
			decodeState = Decode_Head81;
			memset(decodeData, 0, 256);
		}

		if (Decode_Head81 == decodeState)
		{
			//寻找第一副帧帧头
			if (HEAD81 == m_pBuf[nIndex])
			{
				decodeData[decodeDataLength++] = m_pBuf[nIndex];
				decodeState = Decode_Head81_NUM;
			}
		}
		else if (Decode_Head81_NUM == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第一副祯编号
			if (2 == decodeDataLength)
			{
				if ((HEAD81_1 == decodeData[decodeDataLength - 1]) || (HEAD81_2 == decodeData[decodeDataLength - 1]) ||
					(HEAD81_3 == decodeData[decodeDataLength - 1]))
				{
					decodeState = Decode_Frame81_EB;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame81_EB == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第一副祯同步字1
			if (63 == decodeDataLength)
			{
				if (TAILEB == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Frame81_90;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame81_90 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第一副祯同步字2
			if (64 == decodeDataLength)
			{
				if (TAIL90 == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Head82;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Head82 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第二副祯标识
			if (65 == decodeDataLength)
			{
				if (HEAD82 == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Head82_NUM;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Head82_NUM == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第二副祯编号
			if (66 == decodeDataLength)
			{
				if ((HEAD82_1 == decodeData[decodeDataLength - 1]) || (HEAD82_2 == decodeData[decodeDataLength - 1]) ||
					(HEAD82_3 == decodeData[decodeDataLength - 1]))
				{
					decodeState = Decode_Frame82_EB;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame82_EB == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第二副祯同步字1
			if (127 == decodeDataLength)
			{
				if (TAILEB == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Frame82_90;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame82_90 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第二副祯同步字2
			if (128 == decodeDataLength)
			{
				if (TAIL90 == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Head83;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Head83 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第三副祯标识
			if (129 == decodeDataLength)
			{
				if (HEAD83 == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Head83_NUM;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Head83_NUM == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第三副祯编号
			if (130 == decodeDataLength)
			{
				if ((HEAD83_1 == decodeData[decodeDataLength - 1]) || (HEAD83_2 == decodeData[decodeDataLength - 1]) ||
					(HEAD83_3 == decodeData[decodeDataLength - 1]))
				{
					decodeState = Decode_Frame83_EB;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame83_EB == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第三副祯同步字1
			if (191 == decodeDataLength)
			{
				if (TAILEB == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Frame83_90;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame83_90 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第三副祯同步字2
			if (192 == decodeDataLength)
			{
				if (TAIL90 == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Head84;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Head84 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第四副祯标识
			if (193 == decodeDataLength)
			{
				if (HEAD84 == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Head84_NUM;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Head84_NUM == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第四副祯编号
			if (194 == decodeDataLength)
			{
				if ((HEAD84_1 == decodeData[decodeDataLength - 1]) || (HEAD84_2 == decodeData[decodeDataLength - 1]) ||
					(HEAD84_3 == decodeData[decodeDataLength - 1]))
				{
					decodeState = Decode_Frame84_EB;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame84_EB == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第四副祯同步字1
			if (255 == decodeDataLength)
			{
				if (TAILEB == decodeData[decodeDataLength - 1])
				{
					decodeState = Decode_Frame84_90;
				}
				else
				{
					decodeState = Decode_Head81;
					decodeDataLength = 0;
					memset(decodeData, 0, 256);
				}
			}
		}
		else if (Decode_Frame84_90 == decodeState)
		{
			decodeData[decodeDataLength++] = m_pBuf[nIndex];
			//寻找第四副祯同步字2
			if (256 == decodeDataLength)
			{
				if (TAIL90 == decodeData[decodeDataLength - 1])
				{
					memcpy(m_arrDecodeData, decodeData, 256);
					handleData();
				}

				decodeState = Decode_Head81;
				decodeDataLength = 0;
				memset(decodeData, 0, 256);
			}
		}

		nIndex++;
	}
	return m_TD550TelemetryData;
}

void TD550DataAnalysis::handleData()
{
	unsigned char cCRCData[201];
	int nIndex = 0;
	memset(cCRCData, 0, 201);
	for (int i = 1; i <= 241; i++)
	{
		if ((i > 11 && i < 20) || (i > 43 && i < 52) || (i > 107 && i < 116) || (i > 139 && i < 148) || (i > 171 && i < 180))
		{
			continue;
		}
		else
			cCRCData[nIndex++] = m_arrDecodeData[i];
	}

	unsigned short crc;
	crc = CalCRC16_CCITT(cCRCData, nIndex);
	if (crc == (m_arrDecodeData[242] + m_arrDecodeData[243] * 0x100))
	{
		decode550(m_arrDecodeData, 256);
	}
	memset(m_arrDecodeData, 0, 256);
}
unsigned short CRC16_CCIT_table[16] = { 0x0, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
0xc18c, 0xd1ad, 0xe1ce, 0xf1ef };
unsigned short TD550DataAnalysis::CalCRC16_CCITT(unsigned char* chkbuf, int len)
{
	unsigned char byte_up = 0;
	unsigned char byte_temp = 0;
	unsigned short CRC_temp = 0;
	unsigned short CRC_code = 0xffff;
	unsigned short tempL = 0;

	for (int i = 0; i < len; i++)
	{
		tempL = chkbuf[i];

		byte_up = (unsigned char)(CRC_code >> 12);

		byte_temp = byte_up ^ (tempL >> 4);

		CRC_temp = CRC16_CCIT_table[byte_temp];

		CRC_code = ((CRC_code << 4) ^ CRC_temp) & 0xffff;

		byte_up = (unsigned char)(CRC_code >> 12);

		byte_temp = byte_up ^ (tempL & 0xf);

		CRC_temp = CRC16_CCIT_table[byte_temp];

		CRC_code = ((CRC_code << 4) ^ CRC_temp);
	}
	return CRC_code;
}

void TD550DataAnalysis::decode550(unsigned char* m_rebuf, int m_rxlen)
{
	//创建飞机对象
	int bufIndex = 0;
	unsigned char headType = 0;
	unsigned char subFrameData[64] = { 0 };
	while (bufIndex < m_rxlen)
	{
		headType = m_rebuf[bufIndex];
		if (headType == START1)//第一副帧原始数据，分为A机、B机
		{
			memcpy(subFrameData, m_rebuf + bufIndex, 64);
			bufIndex += 64;
			if (subFrameData[1] == 0x11)
			{
				FirstsubOneframe& firstOneFrame = m_TD550TelemetryData->m_FirstSubOneFrame;
				m_YCMutex.lock();
				firstOneFrame.SubCounts = (subFrameData[7] * 0x1000000 + subFrameData[6] * 0x10000 + subFrameData[5] * 0x100 + subFrameData[4]) * 1.0 * 0.02;
				firstOneFrame.FCVersion = subFrameData[9] * 0x100 + subFrameData[8];			//飞控版本
				firstOneFrame.CalibAirSpeed = (*(short*)&subFrameData[10]) * 1.0 * 0.00153;//校准空速
				firstOneFrame.TureAirSpeed = (*(short*)&subFrameData[20]) * 1.0 * 0.00153;//真空速
				firstOneFrame.AirPressHeight = *(short*)&subFrameData[22] * 1.0 * 0.3357;//气压高
				firstOneFrame.StaticTemp = *(short*)&subFrameData[28] * 1.0 * 0.00214;//大气静温
				firstOneFrame.TotalTemp = *(short*)&subFrameData[33] * 1.0 * 0.00214;//大气总温
				firstOneFrame.SmoothAirSpeed = (*(short*)&subFrameData[35]) * 1.0 * 0.00153;//校准空速（平滑处理）
				firstOneFrame.AvoidNorthPos1 = (*(short*)&subFrameData[37]);
				firstOneFrame.AvoidEastPos1 = (*(short*)&subFrameData[39]);
				firstOneFrame.AvoidNorthPos2 = (*(short*)&subFrameData[41]);
				firstOneFrame.NaviState = subFrameData[43];
				firstOneFrame.AvoidEastPos2 = (*(short*)&subFrameData[58]);
				firstOneFrame.AvoidNorthPos3 = (*(short*)&subFrameData[60]);
				m_FCCVersion = firstOneFrame.FCVersion;
				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x21)
			{
				SecondsubOneframe& secondOneFrame = m_TD550TelemetryData->m_SecondSubOneFrame;
				m_YCMutex.lock();
				secondOneFrame.InertialState = subFrameData[8];
				secondOneFrame.BDPdop = (*(short*)&subFrameData[9]) * 1.0 * 0.01;
				secondOneFrame.MEMSNum = subFrameData[11];
				secondOneFrame.UTCTime = (subFrameData[23] * 0x1000000 + subFrameData[22] * 0x10000 + subFrameData[21] * 0x100 + subFrameData[20]) * 1.0 * 0.001;
				secondOneFrame.FlightState = subFrameData[28];
				secondOneFrame.RouteNum = subFrameData[29];
				secondOneFrame.WayPointNum = subFrameData[33];
				secondOneFrame.WayPointState = subFrameData[34];
				secondOneFrame.NextPointNum = subFrameData[35];
				secondOneFrame.NextPointState = subFrameData[36];
				secondOneFrame.HeadingControl = (*(short*)&subFrameData[37]) * 1.0 * 0.01;
				secondOneFrame.TrackError = (*(short*)&subFrameData[39]) * 1.0 * 0.01;
				secondOneFrame.SetoverDis = *(short*)&subFrameData[41];
				secondOneFrame.MainNavNum = subFrameData[43];
				secondOneFrame.FlushingDis = *(short*)&subFrameData[58];
				secondOneFrame.FlushingTime = subFrameData[61] * 0x100 + subFrameData[60];

				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x31)
			{

				ThirdsubOneframe& thirdOneFrame = m_TD550TelemetryData->m_ThirdSubOneFrame;
				m_YCMutex.lock();
				thirdOneFrame.DisperseIn1 = subFrameData[8];
				//thirdOneFrame.DisperseOut1 = subFrameData[9];
				thirdOneFrame.DisperseOut2 = subFrameData[11] * 0x100 + subFrameData[10];
				//coreLog.info("离散输出指令" + QString::number(thirdOneFrame.DisperseOut2));
				thirdOneFrame.DisperseOutRe2 = subFrameData[21] * 0x100 + subFrameData[20];

				thirdOneFrame.Battery = subFrameData[22];
				thirdOneFrame.OpenK2 = (*(short*)&subFrameData[28]) * 1.0 * 0.1;//前旋翼转速;
				double aaa = (*(short*)&subFrameData[33]);
				thirdOneFrame.OilPressure = (*(short*)&subFrameData[33]) * 1.0 * 0.001;//前发燃油压力;
				thirdOneFrame.OilPressure2 = (*(short*)&subFrameData[35]) * 1.0 * 0.001;//后发燃油压力;
				thirdOneFrame.OilVolume = (*(short*)&subFrameData[37]) * 1.0 * 0.001;//下油量电压;


				//thirdOneFrame.OilVolume2 = (*(short*)&subFrameData[77])* 1.0 * 0.001;//下油量电压;
				//thirdOneFrame.OilRemianing = (*(short*)&subFrameData[79])* 1.0 * 0.001;//剩余燃油;
				//thirdOneFrame.RetarderTemperature = (*(short*)&subFrameData[73])* 1.0 * 0.001;//前减速器滑油温度;
				//thirdOneFrame.RetarderTemperature2 = (*(short*)&subFrameData[75])* 1.0 * 0.001;//后减速器滑油温度;
				//thirdOneFrame.ThrottleOpening = (*(short*)&subFrameData[85])* 1.0 * 0.001;//前发油门开度;
				//thirdOneFrame.ThrottleOpening2 = (*(short*)&subFrameData[133])* 1.0 * 0.001;//后发油门开度;


				//thirdOneFrame.OpenK4 = (*(short*)&subFrameData[37])* 1.0 * 0.001;
				thirdOneFrame.RetarderPressure = (*(short*)&subFrameData[39]) * 1.0 * 0.1;
				thirdOneFrame.RetarderTemp = (*(short*)&subFrameData[41]) * 1.0 * 0.1;
				thirdOneFrame.OpenK1 = (*(short*)&subFrameData[58]) * 1.0;//离合状态;
				thirdOneFrame.Tension = (*(short*)&subFrameData[60]) * 1.0 * 0.001;//离合反馈电压;
				m_YCMutex.unlock();
			}
		}
		else if (headType == START2)//第二副帧原始数据，分为A机、B机
		{
			memcpy(subFrameData, m_rebuf + bufIndex, 64);
			bufIndex += 64;
			if (subFrameData[1] == 0x12)
			{
				FirstsubTwoframe &firstTwoFrame = m_TD550TelemetryData->m_FirstSubTwoFrame;
				m_YCMutex.lock();
				firstTwoFrame.D1ServoControl = (*(short*)&subFrameData[3]) * 1.0 * 0.002;//D1舵机位置控制律指令
				firstTwoFrame.D2ServoControl = (*(short*)&subFrameData[5]) * 1.0 * 0.002;//D2舵机位置控制律指令
				firstTwoFrame.D3ServoControl = (*(short*)&subFrameData[7]) * 1.0 * 0.002;//D3舵机位置控制律指令
				firstTwoFrame.U1ServoControl = (*(short*)&subFrameData[9]) * 1.0 * 0.002;//U1舵机位置控制律指令
				firstTwoFrame.U2ServoControl = (*(short*)&subFrameData[11]) * 1.0 * 0.002;//U2舵机位置控制律指令
				firstTwoFrame.U3ServoControl = (*(short*)&subFrameData[13]) * 1.0 * 0.002;//U3舵机位置控制律指令
				firstTwoFrame.DamperServoControl = (*(short*)&subFrameData[15]) * 1.0 * 0.01;//风门舵机位置控制律指令
				firstTwoFrame.DirServoControl = (*(short*)&subFrameData[17]) * 1.0 * 0.01;//方向舵机位置控制律指令
				firstTwoFrame.D1ServoPostion = (*(short*)&subFrameData[19]) * 1.0 * 0.001;//D1舵机位置反馈
				firstTwoFrame.D2ServoPostion = (*(short*)&subFrameData[21]) * 1.0 * 0.001;//D2舵机位置反馈
				firstTwoFrame.D3ServoPostion = (*(short*)&subFrameData[23]) * 1.0 * 0.001;//D3舵机位置反馈
				firstTwoFrame.U1ServoPostion = (*(short*)&subFrameData[25]) * 1.0 * 0.001;//u1舵机位置反馈
				firstTwoFrame.U2ServoPostion = (*(short*)&subFrameData[36]) * 1.0 * 0.001;//u2舵机位置反馈
				firstTwoFrame.U3ServoPostion = (*(short*)&subFrameData[38]) * 1.0 * 0.001;//u3舵机位置反馈
				firstTwoFrame.QXYZJJJJ = (*(short*)&subFrameData[40]) * 1.0 * 0.001;//前旋翼总距桨距角
				firstTwoFrame.QXYFYJJJ = (*(short*)&subFrameData[42]) * 1.0 * 0.001;//前旋翼俯仰桨距角
				firstTwoFrame.lat = (*(int*)&subFrameData[52]) * 1.0 * 0.0000001;//纬度
				firstTwoFrame.lon = (*(int*)&subFrameData[56]) * 1.0 * 0.0000001;//经度
				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x22)
			{
				SecondsubTwoframe& secondTwoFrame = m_TD550TelemetryData->m_SecondSubTwoFrame;
				m_YCMutex.lock();
				secondTwoFrame.GSB = (*(short*)&subFrameData[3]) * 1.0 * 0.01;
				secondTwoFrame.AvoidTimeIndex = subFrameData[6] * 0x100 + subFrameData[5];
				secondTwoFrame.AvoidTaskState = subFrameData[7];
				secondTwoFrame.AvoidNorthSpeed = (*(short*)&subFrameData[8]) * 1.0 * 0.01;
				secondTwoFrame.AvoidEastSpeed = (*(short*)&subFrameData[10]) * 1.0 * 0.01;
				secondTwoFrame.AcoidFCState = subFrameData[12];
				secondTwoFrame.AvoidFCOrder = subFrameData[13];
				secondTwoFrame.ctc = (*(short*)&subFrameData[15]) * 1.0 * 0.01;
				secondTwoFrame.b1c = (*(short*)&subFrameData[17]) * 1.0 * 0.01;
				secondTwoFrame.a1c = (*(short*)&subFrameData[19]) * 1.0 * 0.01;
				secondTwoFrame.dtc = (*(short*)&subFrameData[21]) * 1.0 * 0.01;
				secondTwoFrame.HZJJJJ = (*(short*)&subFrameData[23]) * 1.0 * 0.001;
				secondTwoFrame.RouteSpeed = (*(short*)&subFrameData[25]) * 1.0 * 0.01;
				secondTwoFrame.RouteVPostion = (*(short*)&subFrameData[36]) * 1.0 * 0.01;
				secondTwoFrame.RouteHPostion = (*(short*)&subFrameData[38]) * 1.0 * 0.01;
				secondTwoFrame.RouteYPostion = (*(short*)&subFrameData[40]) * 1.0 * 0.01;
				secondTwoFrame.RouteHeight = (*(short*)&subFrameData[42]) * 1.0 * 0.2;
				secondTwoFrame.RouteVSpeed = (*(short*)&subFrameData[52]) * 1.0 * 0.01;
				secondTwoFrame.RouteSideSpeed = (*(short*)&subFrameData[54]) * 1.0 * 0.01;
				secondTwoFrame.NaviState = subFrameData[59];
				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x32)
			{

				ThirdsubTwoframe& thirdTwoFrame = m_TD550TelemetryData->m_ThirdSubTwoFrame;
				m_YCMutex.lock();
				thirdTwoFrame.OpenK3 = (*(short*)&subFrameData[3]) * 1.0 * 0.1;
				thirdTwoFrame.Motortemp1 = (*(unsigned short*)&subFrameData[5]) * 1.0 * 0.01;
				thirdTwoFrame.Motortemp2 = (*(unsigned short*)&subFrameData[7]) * 1.0 * 0.01;
				thirdTwoFrame.ROiltemp = (*(short*)&subFrameData[9]) * 1.0 * 0.1;
				thirdTwoFrame.ROilPress = (*(short*)&subFrameData[11]) * 1.0 * 0.1;
				thirdTwoFrame.OilPress = (*(short*)&subFrameData[13]) * 1.0 * 0.001;
				thirdTwoFrame.Oil = (*(short*)&subFrameData[15]) * 1.0 * 0.01;
				thirdTwoFrame.EngineRPM145 = (*(short*)&subFrameData[19]) * 1.0 * 0.2;
				thirdTwoFrame.ThrottlePos145 = (*(short*)&subFrameData[21]) * 1.0 * 0.05;
				thirdTwoFrame.EngineOilPre145 = (*(unsigned short*)&subFrameData[23]) * 0.01;
				thirdTwoFrame.EngineOilTemp145 = (*(unsigned short*)&subFrameData[25]) * 1.0 * 0.01;
				//thirdTwoFrame.ExhaustTemp1 = (*(short*)&subFrameData[36])* 1.0 * 0.1;
				//thirdTwoFrame.ExhaustTemp2 = (*(short*)&subFrameData[38])* 1.0 * 0.1;
				thirdTwoFrame.ExhaustTemp1 = ((short)(subFrameData[37] * 0x100 + (subFrameData[36]))) * 1.0 * 0.1;
				thirdTwoFrame.ExhaustTemp2 = ((short)(subFrameData[39] * 0x100 + (subFrameData[38]))) * 1.0 * 0.1;
				if (thirdTwoFrame.ExhaustTemp1 < 0)
				{
					printf("-\n");;
				}

				thirdTwoFrame.ExhaustTemp3 = (*(short*)&subFrameData[40]) * 1.0 * 0.1;
				thirdTwoFrame.ExhaustTemp4 = (*(short*)&subFrameData[42]) * 1.0 * 0.1;
				thirdTwoFrame.IntakePressure = (*(unsigned short*)&subFrameData[52]) * 1.0 * 0.1;
				thirdTwoFrame.TurboPressure = (*(unsigned short*)&subFrameData[54]) * 1.0 * 0.1;
				thirdTwoFrame.CoolantTemp = (*(unsigned short*)&subFrameData[56]) * 1.0 * 0.1;
				thirdTwoFrame.EngineOilPre = ((*(short*)&subFrameData[58]) * 1.0) * 0.01;
				m_YCMutex.unlock();
			}
		}
		else if (headType == START3)//第三副帧原始数据，分为A机、B机
		{
			memcpy(subFrameData, m_rebuf + bufIndex, 64);
			bufIndex += 64;
			if (subFrameData[1] == 0x13)
			{
				FirstsubThreeframe& firstThreeFrame = m_TD550TelemetryData->m_FirstSubThreeFrame;
				m_YCMutex.lock();
				firstThreeFrame.AbsolutelyHeight = (*(short*)&subFrameData[3]) * 1.0 * 0.5;
				firstThreeFrame.XSpeed = (*(short*)&subFrameData[5]) * 1.0 * 0.05;
				firstThreeFrame.YSpeed = (*(short*)&subFrameData[7]) * 1.0 * 0.05;
				firstThreeFrame.ZSpeed = (*(short*)&subFrameData[9]) * 1.0 * 0.05;
			//	firstThreeFrame.RemoteAuthorizationStatus = subFrameData[15];//遥控器是否授权状态;
				firstThreeFrame.XaSpeed = (*(short*)&subFrameData[20]) * 1.0 * 0.00245;
				firstThreeFrame.YaSpeed = (*(short*)&subFrameData[22]) * 1.0 * 0.00245;
				firstThreeFrame.ZaSpeed = (*(short*)&subFrameData[24]) * 1.0 * 0.00245;
				firstThreeFrame.RollAngVelocity = (*(short*)&subFrameData[26]) * 1.0 * 0.02;
				firstThreeFrame.PitchAngVelocity = (*(short*)&subFrameData[28]) * 1.0 * 0.02;
				firstThreeFrame.YawAngVelocity = (*(short*)&subFrameData[33]) * 1.0 * 0.02;
				firstThreeFrame.Yaw = (*(short*)&subFrameData[35]) * 1.0 * 0.01;
				firstThreeFrame.Pitch = (*(short*)&subFrameData[37]) * 1.0 * 0.01;
				firstThreeFrame.Roll = (*(short*)&subFrameData[39]) * 1.0 * 0.01;
				firstThreeFrame.EastSpeed = (*(short*)&subFrameData[41]) * 1.0 * 0.05;
				firstThreeFrame.NothSpeed = (*(short*)&subFrameData[52]) * 1.0 * 0.05;
				firstThreeFrame.RelHeight = (*(short*)&subFrameData[54]) * 1.0 * 0.2;
				firstThreeFrame.AYB = (*(short*)&subFrameData[56]) * 1.0 * 0.00245;
				firstThreeFrame.EngineR = (subFrameData[59] * 0x100 + subFrameData[58]) * 1.0 */* 0.2*/0.5;
				//firstThreeFrame.EngineR = (*(short*)&subFrameData[24])* 1.0 * 0.5;
				firstThreeFrame.MainRotor = (subFrameData[61] * 0x100 + subFrameData[60]) * 1.0 * 0.1;


				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x23)
			{

				SecondsubThreeframe& secondThreeFrame = m_TD550TelemetryData->m_SecondSubThreeFrame;
				m_YCMutex.lock();
				secondThreeFrame.FCCPower = (*(short*)&subFrameData[3]) * 1.0 * 100 / 32767;
				secondThreeFrame.FCCState = subFrameData[6] * 0x100 + subFrameData[5];
				secondThreeFrame.FCCTemp = (*(short*)&subFrameData[7]) * 1.0 * 0.0625;
				secondThreeFrame.ServoTemp = subFrameData[9];
				secondThreeFrame.FCCError = (subFrameData[11] * 0x100 + subFrameData[10]);
				secondThreeFrame.FCCLevel = subFrameData[20];
				secondThreeFrame.ErrorID_qian = subFrameData[21];
				secondThreeFrame.ErrorNumber_qian = (*(short*)&subFrameData[22]);
				secondThreeFrame.ErrorID_hou = subFrameData[24];
				secondThreeFrame.ErrorNumber_hou = (*(short*)&subFrameData[25]);
				//secondThreeFrame.EngineError = (*(short*)&subFrameData[22]);
				//secondThreeFrame.EngineErrorBack = (*(short*)&subFrameData[25]);


				secondThreeFrame.CollectionBoxState = subFrameData[28] * 0x100 + subFrameData[27];
				secondThreeFrame.ForceLandPointIndex = subFrameData[29];
				secondThreeFrame.ControlMode = subFrameData[33];
				secondThreeFrame.ModeUsed = subFrameData[35] * 0x100 + subFrameData[34];
				secondThreeFrame.ManeuMode = subFrameData[37] * 0x100 + subFrameData[36];
				secondThreeFrame.EngineMode = subFrameData[38];
				secondThreeFrame.Startup = subFrameData[39];
				secondThreeFrame.Closedown = subFrameData[40];
				secondThreeFrame.Reserved = subFrameData[41] * 1.0 * 0.1;;
				secondThreeFrame.PSI_DELTA = (*(short*)&subFrameData[42]) * 1.0 * 0.01;
				secondThreeFrame.DIS_XY = (subFrameData[55] * 0x1000000 + subFrameData[54] * 0x10000 + subFrameData[53] * 0x100 + subFrameData[52]) * 1.0 * 0.1;
				secondThreeFrame.DIS_X = (subFrameData[59] * 0x1000000 + subFrameData[58] * 0x10000 + subFrameData[57] * 0x100 + subFrameData[56]) * 1.0 * 0.1;


				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x33)
			{
				ThirdsubThreeframe& thirdThreeFrame = m_TD550TelemetryData->m_ThirdSubThreeFrame;
				m_YCMutex.lock();
				thirdThreeFrame.EngineOilTemp = (*(short*)&subFrameData[3]) * 1.0 * 0.01;
				thirdThreeFrame.DamperPostion = (*(short*)&subFrameData[5]) * 1.0 * 0.05;
				thirdThreeFrame.InairTemp = (*(short*)&subFrameData[7]) * 1.0 * 0.1;
				thirdThreeFrame.InairPre = (*(short*)&subFrameData[9]) * 1.0 * 0.1;

				thirdThreeFrame.FrontEngineIdleMarking = subFrameData[20] * 1.0;
				thirdThreeFrame.BehindEngineIdleMarking = subFrameData[21] * 1.0;
				thirdThreeFrame.OutAirPostion = (*(unsigned short*)&subFrameData[22]) * 1.0 * 0.01;
				thirdThreeFrame.EngineRAM = (*(short*)&subFrameData[24]) * 1.0 * 0.5;
				thirdThreeFrame.EnineInAirTemp = (*(unsigned short*)&subFrameData[26]) * 0.01;
				thirdThreeFrame.EcuAbus = (*(short*)&subFrameData[28]) * 1.0 * 0.1;
				thirdThreeFrame.EcuBbus = (*(short*)&subFrameData[33]) * 1.0 * 0.1;
				thirdThreeFrame.EngineTime = (*(short*)&subFrameData[35]) * 20;
				thirdThreeFrame.OilPreDiff = (*(short*)&subFrameData[38]) * 1.0 * 0.0001;//前发风扇pwm
				thirdThreeFrame.StableBoxPre = (*(short*)&subFrameData[40]) * 1.0 * 0.1;//后发风扇pwm
				thirdThreeFrame.StableBoxGoalPre = (*(short*)&subFrameData[42]) * 1.0 * 0.1;
				thirdThreeFrame.StableBoxTemp = (*(short*)&subFrameData[52]) * 1.0 * 0.1;
				thirdThreeFrame.CoolingTempMax = (*(short*)&subFrameData[54]) * 1.0 * 0.1;
				thirdThreeFrame.CoolingTempMin = (*(short*)&subFrameData[56]) * 1.0 * 0.1;
				thirdThreeFrame.ExhaustTempmax = (*(short*)&subFrameData[58]) * 1.0 * 0.1;
				thirdThreeFrame.ExhaustTempmin = (*(short*)&subFrameData[60]) * 1.0 * 0.1;


				m_YCMutex.unlock();
			}
		}
		else if (headType == START4)//第四副帧原始数据，分为A机、B机
		{
			memcpy(subFrameData, m_rebuf + bufIndex, 64);
			bufIndex += 64;
			if (subFrameData[1] == 0x14)
			{

				FirstsubFourframe& firstFourFrame = m_TD550TelemetryData->m_FirstSubFourFrame;
				m_YCMutex.lock();
				firstFourFrame.SignalSource1 = subFrameData[3];
				firstFourFrame.SignalSource2 = subFrameData[4];
				firstFourFrame.SignalSource3 = subFrameData[5];
				firstFourFrame.SignalSource4 = subFrameData[6];
				firstFourFrame.SignalSource5 = subFrameData[7];
				firstFourFrame.FaultCode = subFrameData[12] * 0x1000000 + subFrameData[11] * 0x10000 + subFrameData[10] * 0x100 + subFrameData[9];
				firstFourFrame.AvoidEastPos3 = (*(short*)&subFrameData[13]);
				firstFourFrame.D1ServoMFault = subFrameData[15];
				firstFourFrame.D1ServoBFault = subFrameData[16];
				firstFourFrame.D2ServoMFault = subFrameData[17];
				firstFourFrame.D2ServoBFault = subFrameData[18];
				firstFourFrame.D3ServoMFault = subFrameData[19];
				firstFourFrame.D3ServoBFault = subFrameData[20];
				firstFourFrame.U1ServoMFault = subFrameData[21];
				firstFourFrame.U1ServoBFault = subFrameData[22];
				firstFourFrame.U2ServoMFault = subFrameData[23];
				firstFourFrame.U2ServoBFault = subFrameData[24];
				firstFourFrame.U3ServoMFault = subFrameData[25];
				firstFourFrame.U3ServoBFault = subFrameData[26];
				//firstFourFrame.DamperServoMFault = subFrameData[27];
				firstFourFrame.QXYHGJJJ = (*(short*)&subFrameData[27]) * 1.0 * 0.001;
				//firstFourFrame.DamperServoBFault = subFrameData[28];
				firstFourFrame.DirServoMFault = subFrameData[29];
				firstFourFrame.DirServoBFault = subFrameData[33];
				firstFourFrame.D1ServoCurrent = (*(short*)&subFrameData[34]) * 1.0 * 0.01;
				firstFourFrame.D2ServoCurrent = (*(short*)&subFrameData[36]) * 1.0 * 0.01;
				firstFourFrame.D3ServoCurrent = (*(short*)&subFrameData[38]) * 1.0 * 0.01;
				firstFourFrame.U1ServoCurrent = (*(short*)&subFrameData[40]) * 1.0 * 0.01;
				firstFourFrame.U2ServoCurrent = (*(short*)&subFrameData[42]) * 1.0 * 0.01;
				firstFourFrame.U3ServoCurrent = (*(short*)&subFrameData[44]) * 1.0 * 0.01;
				firstFourFrame.HXYFYJJJ = (*(short*)&subFrameData[46]) * 1.0 * 0.001;//后旋翼俯仰桨距角;
				firstFourFrame.HXYHGJJJ = (*(short*)&subFrameData[48]) * 1.0 * 0.001;//后旋翼横滚桨距角;


				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x24)
			{

				SecondsubFourframe& secondFourFrame = m_TD550TelemetryData->m_SecondSubFourFrame;

				m_YCMutex.lock();
				secondFourFrame.DIS_Y = (subFrameData[6] * 0x1000000 + subFrameData[5] * 0x10000 + subFrameData[4] * 0x100 + subFrameData[3]) * 1.0 * 0.1;
				secondFourFrame.takeoffHeight = (*(short*)&subFrameData[7]) * 1.0 * 0.2;
				secondFourFrame.takeoffWeight = (subFrameData[10] * 0x100 + subFrameData[9]) * 1.0 * 0.1;
				secondFourFrame.NowWeight = (subFrameData[12] * 0x100 + subFrameData[11]) * 1.0 * 0.1;
				secondFourFrame.AvoidPosX = (*(short*)&subFrameData[13]);
				secondFourFrame.AvoidPosY = (*(short*)&subFrameData[15]);
				secondFourFrame.EquipmentState = subFrameData[28] * 0x100 + subFrameData[17];
				secondFourFrame.EquipmentAlarm1 = subFrameData[18];
				secondFourFrame.EquipmentAlarm2 = subFrameData[19];
				secondFourFrame.EquipmentAlarm3 = subFrameData[20];
				secondFourFrame.EquipmentAlarm4 = subFrameData[21];
				secondFourFrame.FlightTime = subFrameData[23] * 0x100 + subFrameData[22];
				secondFourFrame.FlightSurTime = subFrameData[25] * 0x100 + subFrameData[24];
				secondFourFrame.WaterFan = subFrameData[26];
				secondFourFrame.MiddleFan = subFrameData[27];
				//secondFourFrame.RadarHeight = (subFrameData[38] * 0x100 + subFrameData[37])* 1.0 * 0.0006714;
				//secondFourFrame.LandHeight = (*(short*)&subFrameData[39])* 1.0 * 0.01;
				secondFourFrame.MidFanCurrent = (*(short*)&subFrameData[33]) * 1.0 * 0.01;//前发风扇电流;
				secondFourFrame.WaterFacCurrent = (*(short*)&subFrameData[35]) * 1.0 * 0.01;//后发风扇电流;
				secondFourFrame.GeneratorCurrent = (*(short*)&subFrameData[37]) * 1.0 * 0.01;
				secondFourFrame.GeneratorCurrentBack = (*(short*)&subFrameData[39]) * 1.0 * 0.01;
				secondFourFrame.RadioHeight = (*(short*)&subFrameData[41]) * 1.0 * 0.1;
				secondFourFrame.RadioSmoothAlt = (*(short*)&subFrameData[43]) * 1.0 * 0.1;
				secondFourFrame.Power74V = subFrameData[46] * 1.0 * 0.1;
				//secondFourFrame.Power12V = (*(short*)&subFrameData[46])* 1.0 * 0.01;
				secondFourFrame.Power12V = subFrameData[47] * 1.0 * 0.1;
				secondFourFrame.Power24V = (*(short*)&subFrameData[48]) * 1.0 * 0.01;


				m_YCMutex.unlock();
			}

			if (subFrameData[1] == 0x34)
			{


				ThirdsubFourframe& thirdFourFrame = m_TD550TelemetryData->m_ThirdSubFourFrame;

				m_YCMutex.lock();
				thirdFourFrame.KgReply = subFrameData[4] * 0x100 + subFrameData[3];
				thirdFourFrame.YtReply = subFrameData[5];
				thirdFourFrame.YtHdReply = subFrameData[6];
				thirdFourFrame.B2YReply = (*(char*)&subFrameData[7]) * 100.0 / 127;
				thirdFourFrame.B2XReply = (*(char*)&subFrameData[8]) * 100.0 / 127;
				thirdFourFrame.B1YReply = (*(char*)&subFrameData[9]) * 100.0 / 127;
				thirdFourFrame.B1XReply = (*(char*)&subFrameData[10]) * 100.0 / 127;
				thirdFourFrame.DamperReply = (*(char*)&subFrameData[11]) * 100.0 / 127;
				thirdFourFrame.byte11Reply = *(char*)&subFrameData[13];
				thirdFourFrame.byte12Reply = *(char*)&subFrameData[14];
				thirdFourFrame.byte13Reply = *(char*)&subFrameData[15];
				thirdFourFrame.byte14Reply = *(char*)&subFrameData[16];
				thirdFourFrame.byte15Reply = *(char*)&subFrameData[17];
				thirdFourFrame.byte16Reply = *(char*)&subFrameData[18];
				thirdFourFrame.byte17Reply = *(char*)&subFrameData[19];
				thirdFourFrame.byte18Reply = *(char*)&subFrameData[20];
				thirdFourFrame.byte19Reply = *(char*)&subFrameData[21];
				thirdFourFrame.byte20Reply = *(char*)&subFrameData[22];
				thirdFourFrame.byte21Reply = *(char*)&subFrameData[23];
				thirdFourFrame.byte22Reply = *(char*)&subFrameData[24];
				thirdFourFrame.byte23Reply = *(char*)&subFrameData[25];
				thirdFourFrame.byte24Reply = *(char*)&subFrameData[26];
				thirdFourFrame.byte25Reply = *(char*)&subFrameData[27];
				thirdFourFrame.byte26Reply = *(char*)&subFrameData[28];
				thirdFourFrame.byte27Reply = *(char*)&subFrameData[29];
				thirdFourFrame.LinkTest = subFrameData[33];
				thirdFourFrame.EngineAirPre = (*(unsigned short*)&subFrameData[35]) * 1.0 * 0.1;
				thirdFourFrame.TCUPower = (*(short*)&subFrameData[37]) * 1.0 * 0.01;
				thirdFourFrame.ServoPos = (*(short*)&subFrameData[39]) * 1.0 * 0.01;
				thirdFourFrame.ServoGoalPos = (*(short*)&subFrameData[41]) * 1.0 * 0.01;
				thirdFourFrame.NaviPDOP = (*(unsigned short*)&subFrameData[43]) * 1.0 * 0.01;
				thirdFourFrame.B2YControl = (*(char*)&subFrameData[45]) * 1.0 / 127;
				thirdFourFrame.B2XControl = (*(char*)&subFrameData[46]) * 1.0 / 127;
				thirdFourFrame.B1YControl = (*(char*)&subFrameData[47]) * 1.0 / 127;
				thirdFourFrame.B1XControl = (*(char*)&subFrameData[48]) * 1.0 / 127;
				thirdFourFrame.FUTABAState = subFrameData[49];
				m_YCMutex.unlock();
			}
		}
	}
}
