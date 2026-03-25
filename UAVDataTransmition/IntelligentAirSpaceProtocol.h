#pragma once
#include <QObject>
#include "ConstructionProtocolInterface.h"

class IntelligentAirSpaceProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	IntelligentAirSpaceProtocol(QObject *parent=NULL);
	~IntelligentAirSpaceProtocol();

	int ConstructionData(unsigned char* sendBuf) override;
};
