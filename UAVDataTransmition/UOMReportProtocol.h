#pragma once

#include "ConstructionProtocolInterface.h"

class UOMReportProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	UOMReportProtocol(QObject *parent = NULL);
	~UOMReportProtocol();
	int ConstructionData(unsigned char* sendBuf) override;
private:
	////QString QDateTime::currentDateTime().toMSecsSinceEpoch();
};
