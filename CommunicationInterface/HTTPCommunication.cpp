#include "HTTPCommunication.h"

HTTPCommunication::HTTPCommunication(QObject *parent)
	: CommunicationInterface(parent), m_pReply(NULL)
{
	m_pManager = new QNetworkAccessManager();
	m_pRequest = new QNetworkRequest();
	QSslConfiguration sslConfig = m_pRequest->sslConfiguration();
	sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 禁用SSL证书验证
	m_pRequest->setSslConfiguration(sslConfig);


	//QSslConfiguration sslConfig(QSslConfiguration::defaultConfiguration());
	//sslConfig.setProtocol(QSsl::AnyProtocol); // 这会禁用 SSL/TLS
	//m_pRequest->setAttribute(QNetworkRequest::postcl, QVariant::fromValue(sslConfig));

	m_pRequest->setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
	//m_pRequest->setRawHeader("Content_Type", "application/json");

	connect(m_pManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(sl_receiveHttpData(QNetworkReply *)));
}

HTTPCommunication::~HTTPCommunication()
{
}

bool HTTPCommunication::openPort()
{
	m_pReply=m_pManager->get(QNetworkRequest(QUrl(ListenAddress)));
	connect(m_pReply, SIGNAL(readyRead()), this, SLOT(sl_receiveHttpData()));
	connect(m_pReply, SIGNAL(downloadProgress()), this, SLOT(sl_DownloadProgress()));
	connect(m_pReply, SIGNAL(finished()), this, SLOT(sl_DownloadFinish()));
	setConnectSuccess(true);
	return true;
}

bool HTTPCommunication::closePort()
{
	if (m_pReply)
		return false;

	disconnect(m_pReply, SIGNAL(readyRead()), this, SLOT(sl_receiveHttpData()));
	disconnect(m_pReply, SIGNAL(downloadProgress()), this, SLOT(sl_DownloadProgress()));
	disconnect(m_pReply, SIGNAL(finished()), this, SLOT(sl_DownloadFinish()));
	m_pReply->deleteLater();
	m_pReply = NULL;
	setConnectSuccess(false);

	return true;
}

bool HTTPCommunication::sendData(QByteArray ary, int nLength)
{
	//去除空格
	SendAddress.remove(QRegExp("\\s"));
	m_pRequest->setUrl(SendAddress);
	m_pRequest->setRawHeader("Content-Type", "application/json;charset=utf-8");
	m_pRequest->setRawHeader("x-uav-token", HeaderToken);
	m_pReply = m_pManager->post(*m_pRequest, ary);
	if (m_pReply->error() != QNetworkReply::NoError)
	{
		qDebug() << "reply error:"<<m_pReply->error();
	}
	return true;
}

void HTTPCommunication::sl_receiveHttpData()
{
	QByteArray replyData = m_pReply->readAll();
	QJsonDocument json_recv = QJsonDocument::fromJson(replyData);//解析json对
	if (!json_recv.isNull())
	{
		QJsonObject object = json_recv.object();
		if (object.contains("code"))
		{
			int value = object.value("code").toInt();  // 获取指定 key 对应的 value
			if (value == 0)
			{
				//qDebug() << "数据发送成功";
			}
		}
	}
	emit si_ReceiveData(replyData, replyData.size());
}

void HTTPCommunication::sl_receiveHttpData(QNetworkReply *reply)
{
	QByteArray replyData = m_pReply->readAll();
	QJsonDocument json_recv = QJsonDocument::fromJson(replyData);//解析json对
	if (!json_recv.isNull())
	{
		QJsonObject object = json_recv.object();
		if (object.contains("code"))
		{
			int value = object.value("code").toInt();  // 获取指定 key 对应的 value
			//qDebug() << "code:" << replyData;
		}
	}
	emit si_ReceiveData(replyData, replyData.size());
}

void HTTPCommunication::sl_DownloadProgress(qint64 currentSize, qint64 TotalSize)
{

}

void HTTPCommunication::sl_DownloadFinish()
{

}
