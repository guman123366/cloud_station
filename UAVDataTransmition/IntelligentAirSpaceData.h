/*
智能空域指挥系统遥测数据
*/
#pragma once

#include "DataDefineInterface.h"
#include <QString>

//智能空域指挥系统注册参数
struct IntelligentAirSpaceAdmin :public DataDefineInterface
{
	IntelligentAirSpaceAdmin()
	{
		status = 0; 
		Message = "";
		DepartmentName = "";
		ParentDepName = "";
		OrgAdmin = 0;
		ServicePhone = "";
		DepartmentGeography = "";
		XUN_TE_JING = 0;
		Openid = "";
		RoleId = 0;
		RoleSign = "";
		DepartmentId = 0;
		Rid = 0;
		Type = 0;
		QString Token = "";
		Uid = 0;
		CheckStatus = 0;
		EagleCodeList = 0;
		SystemCode = "";
		Phone = "";
		Permissions = 0;
		OrgCode = "";
		Appid = 0;
		Name = "";
		FullSign = "";
		CardStatus = 0;
		UserName = "";
	}
	unsigned char status;
	QString Message;
	QString DepartmentName;
	QString ParentDepName;
	unsigned char OrgAdmin;
	QString ServicePhone;
	QString DepartmentGeography;
	unsigned char XUN_TE_JING;
	QString Openid;
	unsigned char RoleId;
	QString RoleSign;
	unsigned char DepartmentId;
	unsigned char Rid;
	unsigned char Type;
	QString Token;
	unsigned char Uid;
	unsigned char CheckStatus;
	unsigned char EagleCodeList;
	QString SystemCode;
	QString Phone;
	unsigned char Permissions;
	QString OrgCode;
	int Appid;
	QString Name;
	QString FullSign;
	unsigned char CardStatus;
	QString UserName;
};