#include "LogReportCoordinator.h"

#include "LogFileRecorder.h"
#include "LogUploadService.h"

#include <QDebug>
#include <QSettings>

LogRuntimeConfig LogRuntimeConfig::fromIni(const QString& appDir)
{
    LogRuntimeConfig config;
    config.appDir = appDir;

    const QString configPath = appDir + "/init/Config.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    config.uploadUrl = settings.value("FileUpload/UploadUrl", QString::fromStdString(config.uploadUrl))
                           .toString()
                           .toStdString();
    config.apiPath = settings.value("FileUpload/ApiPath", QString::fromStdString(config.apiPath))
                         .toString()
                         .toStdString();
    config.apiSuccessPath =
        settings.value("FileUpload/SucessApiPath", QString::fromStdString(config.apiSuccessPath))
            .toString()
            .toStdString();
    config.logFileConfigPath =
        settings.value("FileUpload/logFileConfig", QString::fromStdString(config.logFileConfigPath))
            .toString()
            .toStdString();
    config.sortiePath =
        settings.value("FileUpload/SortiePath", QString::fromStdString(config.sortiePath))
            .toString()
            .toStdString();

    config.successReportUrl = config.uploadUrl + config.apiSuccessPath;
    config.logFileConfigUrl = config.uploadUrl + config.logFileConfigPath;
    config.sortieUrl = config.uploadUrl + config.sortiePath;

    config.dataFolder = settings.value("DataStorage/DataFolder", "data").toString();
    config.defaultMaxFileSizeBytes = settings.value("DataStorage/MaxFileSize", 5120000).toLongLong();
    config.enableFileSplit = settings.value("DataStorage/EnableFileSplit", true).toBool();
    config.filePrefix = settings.value("DataStorage/FilePrefix", "fileData").toString();
    config.rxFileFolder =
        appDir + "/" + settings.value("DataStorage/RxFileFolder", "data/RxFile").toString();
    config.txFileFolder =
        appDir + "/" + settings.value("DataStorage/TxFileFolder", "data/TxFile").toString();

    config.deviceModel = settings.value("Device/Model", "TD550").toString().toStdString();
    config.deviceSn = settings.value("Device/sn", "6722390TD550").toString().toStdString();
    config.appId =
        settings.value("Device/AppId", QString::fromStdString(config.appId)).toString().toStdString();
    config.appSecret = settings.value("Device/AppKey", QString::fromStdString(config.appSecret))
                           .toString()
                           .toStdString();

    config.defaultLogType = settings.value("LogSettings/DefaultLogType", "STATION_PARMA").toString();
    config.appVersion = settings.value("Device/AppVersion", "V1.00.00").toString();

    return config;
}

LogReportCoordinator::LogReportCoordinator(const QString& appDir, QObject* parent)
    : QObject(parent)
    , m_appDir(appDir)
    , m_fileRecorder(nullptr)
    , m_uploadService(nullptr)
    , m_fccVersion(0)
    , m_initialized(false)
{
}

LogReportCoordinator::~LogReportCoordinator()
{
    shutdown();
}

bool LogReportCoordinator::initialize()
{
    if (m_initialized) {
        return true;
    }

    m_runtimeConfig = LogRuntimeConfig::fromIni(m_appDir);

    if (!m_uploadService) {
        m_uploadService = new LogUploadService(this);
    }

    if (m_fileRecorder) {
        delete m_fileRecorder;
        m_fileRecorder = nullptr;
    }

    m_fileRecorder = new LogFileRecorder(m_runtimeConfig);
    m_uploadService->configure(
        m_runtimeConfig,
        [this](LogChannel channel) {
            return m_fileRecorder ? m_fileRecorder->currentFilePath(channel) : QString();
        },
        [this]() { return buildMetadata(); });

    const LogStartupContext startupContext = m_uploadService->loadStartupContext();
    if (!m_fileRecorder->initialize(startupContext)) {
        qWarning() << "Failed to initialize log file recorder";
        delete m_fileRecorder;
        m_fileRecorder = nullptr;
        return false;
    }

    m_uploadService->start();
    m_initialized = true;
    qDebug() << "LogReportCoordinator initialized successfully";
    return true;
}

void LogReportCoordinator::shutdown()
{
    if (!m_fileRecorder && !m_uploadService) {
        m_initialized = false;
        return;
    }

    QList<LogUploadTask> finalTasks;
    if (m_fileRecorder) {
        finalTasks = m_fileRecorder->shutdown();
    }

    if (m_uploadService) {
        m_uploadService->shutdown(finalTasks);
    }

    delete m_fileRecorder;
    m_fileRecorder = nullptr;
    delete m_uploadService;
    m_uploadService = nullptr;
    m_initialized = false;
}

void LogReportCoordinator::appendData(LogChannel channel, const QByteArray& data)
{
    if (!m_fileRecorder) {
        return;
    }

    const QList<LogUploadTask> uploadTasks = m_fileRecorder->appendData(channel, data);
    if (m_uploadService && !uploadTasks.isEmpty()) {
        m_uploadService->enqueueTasks(uploadTasks);
    }
}

void LogReportCoordinator::updateFccVersion(unsigned short version)
{
    m_fccVersion = version;
}

QJsonObject LogReportCoordinator::buildMetadata() const
{
    QJsonObject metadataObject;
    metadataObject["appVersion"] = m_runtimeConfig.appVersion;
    metadataObject["fcVersion"] = formatFccVersion(m_fccVersion);
    return metadataObject;
}

QString LogReportCoordinator::formatFccVersion(unsigned short version)
{
    const int version1 = version % 100;
    const int version3 = version / 10000;
    const int version2 = (version - (version3 * 10000)) / 100;

    const QString part1 = version1 >= 10 ? QString::number(version1)
                                         : QString("0%1").arg(version1);
    const QString part2 = version2 >= 10 ? QString::number(version2)
                                         : QString("0%1").arg(version2);

    return QString("%1.%2.%3").arg(version3).arg(part2).arg(part1);
}
