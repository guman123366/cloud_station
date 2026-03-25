#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"

class IntelligentAirSpace550Protocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	IntelligentAirSpace550Protocol(QObject *parent=nullptr);
	~IntelligentAirSpace550Protocol();

	int ConstructionData(unsigned char* sendBuf) override;
};
