#pragma once

#include "CommunicationInterface.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include "communicationinterface_global.h"

class COMMUNICATIONINTERFACE_EXPORT HTTPCommunication : public CommunicationInterface
{
	Q_OBJECT

public:
	HTTPCommunication(QObject *parent=NULL);
	~HTTPCommunication();

	bool openPort() override;
	bool closePort() override;
	bool sendData(QByteArray ary, int nLength) override;
	void setToken(QByteArray aryToken){ HeaderToken = aryToken; }
private slots:
	//接收到http服务器发送的数据
	void sl_receiveHttpData();
	//接收数据进度:currentPos当前下载数据大小，TotalSize总的数据大小
	void sl_DownloadProgress(qint64 currentSize, qint64 TotalSize);
	//数据接收完成
	void sl_DownloadFinish();

	//接收到http服务器发送的数据
	void sl_receiveHttpData(QNetworkReply *reply);
private:
	QNetworkAccessManager *m_pManager;
	QNetworkRequest *m_pRequest;
	QNetworkReply *m_pReply;
	QByteArray HeaderToken;
};
