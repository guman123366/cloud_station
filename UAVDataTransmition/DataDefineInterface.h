/*
	数据定义
*/
#pragma once

struct DataDefineInterface
{
public:
	virtual ~DataDefineInterface() = default;  // 必须添加的虚析构函数
	DataDefineInterface()
	{
		DataType = "";
	}
	
	QString DataType;
};