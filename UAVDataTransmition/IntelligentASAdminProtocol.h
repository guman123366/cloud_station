#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"

class IntelligentASAdminProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	IntelligentASAdminProtocol(QObject *parent=NULL);
	~IntelligentASAdminProtocol();

	int ConstructionData(unsigned char* sendBuf) override;
};
