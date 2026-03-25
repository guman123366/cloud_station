#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"
#include <QTimer>

class Engineer9XXXProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	Engineer9XXXProtocol(QObject *parent = NULL);
	~Engineer9XXXProtocol();

	int ConstructionData(unsigned char* sendBuf) override;
};
