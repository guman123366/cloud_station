#pragma once

#include "ConstructionProtocolInterface.h"

class ZhouShanProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	ZhouShanProtocol(QObject *parent = NULL);
	~ZhouShanProtocol();
	int ConstructionData(unsigned char* sendBuf) override;
private:
	//QString QDateTime::currentDateTime().toMSecsSinceEpoch();
};
