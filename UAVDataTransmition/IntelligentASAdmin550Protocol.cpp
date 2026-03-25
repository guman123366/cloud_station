#include "IntelligentASAdmin550Protocol.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

IntelligentASAdmin550Protocol::IntelligentASAdmin550Protocol(QObject *parent)
	: ConstructionProtocolInterface(parent)
{
}

IntelligentASAdmin550Protocol::~IntelligentASAdmin550Protocol()
{
}

int IntelligentASAdmin550Protocol::ConstructionData(unsigned char* sendBuf)
{
	QJsonObject RootJasonObject;
	RootJasonObject.insert("username", "gudonggang");
	RootJasonObject.insert("password", "Wmh6MTIzNEdkZw==");
	RootJasonObject.insert("type", 2);
	RootJasonObject.insert("deviceHardId", "ZHZ301R22208032");

	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());
	return byte_array.size();
}
