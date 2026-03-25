#include "LoadConfigInfo.h"
#include <QCoreApplication>

LoadConfigInfo::LoadConfigInfo(QObject *parent)
	: QObject(parent)
{
	QString strConfigFilePath = QCoreApplication::applicationDirPath() + "/init/Config.ini";
	m_pConfigFileSettings = new QSettings(strConfigFilePath, QSettings::IniFormat);
}

LoadConfigInfo::~LoadConfigInfo()
{
}

QSettings* LoadConfigInfo::getConfigSettings()
{
	if (!m_pConfigFileSettings)
		return NULL;

	return m_pConfigFileSettings;
}
