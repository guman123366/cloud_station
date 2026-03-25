#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"
#include <QTimer>

class SanYaProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	SanYaProtocol(QObject *parent = NULL);
	~SanYaProtocol();

	int ConstructionData(unsigned char* sendBuf) override;
};