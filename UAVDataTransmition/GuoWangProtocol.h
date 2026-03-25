#pragma once

#include "ConstructionProtocolInterface.h"

class GuoWangProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	GuoWangProtocol(QObject *parent = NULL);
	~GuoWangProtocol();
	int ConstructionData(unsigned char* sendBuf) override;
private:
	////QString QDateTime::currentDateTime().toMSecsSinceEpoch();
};
