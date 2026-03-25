#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"

class IntelligentASAdmin550Protocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	IntelligentASAdmin550Protocol(QObject *parent = NULL);
	~IntelligentASAdmin550Protocol();

	int ConstructionData(unsigned char* sendBuf) override;
};
