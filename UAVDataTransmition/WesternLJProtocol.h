#pragma once

#include <QObject>
#include "ConstructionProtocolInterface.h"
#include <QTimer>

class WesternLJProtocol : public ConstructionProtocolInterface
{
	Q_OBJECT

public:
	WesternLJProtocol(QObject *parent=NULL);
	~WesternLJProtocol();

	void ContructionProtocol();

};
