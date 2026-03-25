#pragma once

#include <QObject>
#include <QSettings>

class LoadConfigInfo : public QObject
{
	Q_OBJECT

public:
	LoadConfigInfo(QObject *parent=NULL);
	~LoadConfigInfo();
	QSettings* getConfigSettings();
private:
	QSettings* m_pConfigFileSettings;				//╪стьеДжцнд╪Ч
};
