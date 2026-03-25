#include "GuoWangAdminProtocol.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QDateTime>
#include <QCryptographicHash>
#include <QString>
#include <QByteArray>
#include <QDebug>
GuoWangAdminProtocol::GuoWangAdminProtocol(QString initToken, QObject *parent)
	: ConstructionProtocolInterface(parent)
{
	uavToken = initToken;
}

GuoWangAdminProtocol::~GuoWangAdminProtocol()
{
}
//生成sign字段;
QString  GuoWangAdminProtocol::createSign()
{
	QString text = "appId="+appId+"&appKey="+appKey+"&timestamp="+createTimeSnap()+"&uavToken="+uavToken+"&";
	//QString text = "appId=LH&appKey=LH8A5DE4A3588A31E1D490838A6CAF4B&timestamp=1742950758157&uavToken=QK822HH4yX72IL76&";
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(text.toUtf8()); // 将QString转换为UTF-8编码的QByteArray，然后添加到hash中
	QByteArray result = hash.result(); // 获取最终的hash结果
	QString aaa = result.toHex().toUpper();
	return aaa;
	//int a = 0;


	//QString originSign = QString("appId=%1&appKey=%2&timestamp=%3&uavToken=%4&").arg(appId).arg(appKey).arg(createTimeSnap()).arg(uavToken);
	//QString md5Sign = calculateMD5(originSign);
	//QString md5Sign = GetMd5(originSign);
	//return md5Sign;
}
//生成毫秒级时间戳;
QString GuoWangAdminProtocol::createTimeSnap()
{
	QDateTime currentDateTime = QDateTime::currentDateTime();
	qint64 millisecondsTimestamp = currentDateTime.toMSecsSinceEpoch();
	QString result = QString::number(millisecondsTimestamp);
	shijianchuo = result;
	return result;
}
int GuoWangAdminProtocol::ConstructionData(unsigned char* sendBuf)
{ 
	QJsonObject RootJasonObject;
	RootJasonObject.insert("appId", appId);
	RootJasonObject.insert("uavToken", uavToken);
	RootJasonObject.insert("sign", createSign());
	RootJasonObject.insert("timestamp", shijianchuo);

	QByteArray byte_array = QJsonDocument(RootJasonObject).toJson();
	char* buff = byte_array.data();
	memcpy(sendBuf, (unsigned char*)buff, byte_array.size());
	return byte_array.size();
}

QString GuoWangAdminProtocol::calculateMD5(const QString &text) {
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(text.toUtf8()); // 将QString转换为UTF-8编码的QByteArray，然后添加到hash中
	QByteArray result = hash.result(); // 获取最终的hash结果
	return result.toHex(); // 将结果转换为十六进制字符串
}


//MD5加密设置
QString GuoWangAdminProtocol::GetMd5(const QString &value)
{
	QString md5;
	QByteArray bb;//相当于是QChar的一个vector<>
	bb = QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Md5);
	md5.append(bb.toHex());
	return md5;
}