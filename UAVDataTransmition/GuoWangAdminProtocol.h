#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"

class GuoWangAdminProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	GuoWangAdminProtocol(QString initToken,QObject *parent = NULL);
	~GuoWangAdminProtocol();
	QString calculateMD5(const QString &text);
	QString GetMd5(const QString &value);
	int ConstructionData(unsigned char* sendBuf) override;
	//生成sign字段;
	QString  createSign();
	//生成时间戳;
	QString createTimeSnap();
	QString uavToken = "";
	QString appId = "LH";
	QString appKey = "LH8A5DE4A3588A31E1D490838A6CAF4B";
	QString shijianchuo = "";
};
