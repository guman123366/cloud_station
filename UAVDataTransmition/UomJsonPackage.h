 


#include "UomDataDefine.h"

#include <string>

 
////产生随机值的开关;
//bool static m_flag = true;
////用来记录上一次的随机值;
//std::string static m_temp_random = "00000000";
//用来记录上一次状态;
std::string static m_last_FlightState = "TakeOff";

class UomJsonPackage
{
public:
	UomJsonPackage();
	~UomJsonPackage();

public:
	std::string  GenerateJsonString(const WayPoint& point, const UAVPose& pose, const FlightState& state);
	std::string  GenerateJsonString(const UomUAVSate& var);
	bool         ParseTokenStatus(const std::string&  json, std::string& strToken);
	bool         ParseUomStatus(const std::string& json);

private:
	std::string FlightModeConvertDesc(std::string mode);

private:
	int           m_nAappId; //appId
	std::string   m_strSystemCode;//systemCode
	int           m_nDeviceType; //deviceType
	
	std::string  m_strUavType;//无人机类型
	std::string  m_strDevId;//设备id
	std::string  m_strUavSn;//厂商的无人机生产序列号
	std::string  m_strManufacturerID;//生产厂商信用代码
	
	int          m_nPlatformType;
	int          m_nUnmannedId;
	
	

	
	std::string m_temp_FlightState = "TakeOff";



	
};
