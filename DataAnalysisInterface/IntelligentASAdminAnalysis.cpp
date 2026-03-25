#include "IntelligentASAdminAnalysis.h"
#include "../UAVDataTransmition/IntelligentAirSpaceData.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

IntelligentASAdminAnalysis::IntelligentASAdminAnalysis(QObject *parent)
	: DataAnalysisInterface(parent)
{
}

IntelligentASAdminAnalysis::~IntelligentASAdminAnalysis()
{
}

DataDefineInterface* IntelligentASAdminAnalysis::AnalyseData(QByteArray ary, int nLength)
{
	if (nLength == 0)
		return NULL;

	IntelligentAirSpaceAdmin *AirSpaceAdmin = new IntelligentAirSpaceAdmin;
	QJsonDocument JsonDoc(QJsonDocument::fromJson(ary));
	QJsonObject RootObject = JsonDoc.object();
	//AirSpaceAdmin->status = RootObject.value("status").toInt();
	//AirSpaceAdmin->Message = RootObject.value("message").toString();
	QJsonObject DataObject = RootObject.value("data").toObject();
	/*AirSpaceAdmin->DepartmentName = DataObject.value("departmentName").toString();
	AirSpaceAdmin->ParentDepName = DataObject.value("parentDepName").toString();
	AirSpaceAdmin->OrgAdmin = DataObject.value("orgAdmin").toInt();
	AirSpaceAdmin->ServicePhone = DataObject.value("servicePhone").toString();
	AirSpaceAdmin->DepartmentGeography = DataObject.value("departmentGeography").toString();
	AirSpaceAdmin->XUN_TE_JING = DataObject.value("XUN_TE_JING_ORGANIZATION").toInt();
	AirSpaceAdmin->Openid = DataObject.value("openid").toString();
	QJsonArray RolesArray = DataObject.value("roles").toArray();
	QJsonObject RolesObject = RolesArray.at(0).toObject();
	AirSpaceAdmin->RoleId = RolesObject.value("roleId").toInt();
	AirSpaceAdmin->RoleSign = RolesObject.value("roleSign").toString();
	AirSpaceAdmin->DepartmentId = DataObject.value("departmentId").toInt();
	AirSpaceAdmin->Rid = DataObject.value("rid").toArray().at(0).toInt();
	AirSpaceAdmin->Type = DataObject.value("type").toInt();*/
	AirSpaceAdmin->Token = DataObject.value("token").toString();
	/*AirSpaceAdmin->Uid = DataObject.value("uid").toInt();
	AirSpaceAdmin->CheckStatus = DataObject.value("checkStatus").toInt();
	AirSpaceAdmin->SystemCode = DataObject.value("systemCode").toString();
	AirSpaceAdmin->Phone = DataObject.value("phone").toString();
	AirSpaceAdmin->OrgCode = DataObject.value("orgCode").toString();
	AirSpaceAdmin->Appid = DataObject.value("appid").toInt();
	AirSpaceAdmin->Name = DataObject.value("name").toString();
	AirSpaceAdmin->FullSign = DataObject.value("fullSign").toString();
	AirSpaceAdmin->CardStatus = DataObject.value("cardStatus").toInt();
	AirSpaceAdmin->UserName = DataObject.value("username").toString();
	AirSpaceAdmin->DataType = "IntelligentASAdmin";*/

	return AirSpaceAdmin;
}
