#include "IntelligentASAdminProtocol.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

IntelligentASAdminProtocol::IntelligentASAdminProtocol(QObject *parent)
	: ConstructionProtocolInterface(parent)
{
}

IntelligentASAdminProtocol::~IntelligentASAdminProtocol()
{
}

int IntelligentASAdminProtocol::ConstructionData(unsigned char* sendBuf)
{
	QJsonObject RootJasonObject;
	RootJasonObject.insert("username", "gudonggang");
	RootJasonObject.insert("password", "Wmh6MTIzNEdkZw");
	RootJasonObject.insert("type", 2);
	RootJasonObject.insert("deviceHardId", "2763H7M001N342");

	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());
	return byte_array.size();
}
