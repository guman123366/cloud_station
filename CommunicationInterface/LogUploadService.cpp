#include "LogUploadService.h"

#include "CurlUploader.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <cctype>
#include <utility>

namespace {
QString normalizedAbsolutePath(const QString& filePath)
{
    return QFileInfo(filePath).absoluteFilePath();
}

QString detectDataType(const QString& filePath)
{
    if (filePath.contains("STATION_CONTROL", Qt::CaseInsensitive) ||
        filePath.contains("tx", Qt::CaseInsensitive)) {
        return "STATION_CONTROL";
    }
    return "STATION_PARMA";
}

QString extractSortieFromFilename(const QString& filename)
{
    const QStringList parts = filename.split('_');
    if (parts.size() >= 4) {
        bool ok = false;
        const qint64 sortie = parts[2].toLongLong(&ok);
        if (ok) {
            return QString::number(sortie);
        }
    }

    return "";
}

bool parseFilenameComponents(const QString& filename, QString& timestampStr, int& sequenceNum)
{
    const QString baseName = QFileInfo(filename).fileName();
    QRegularExpression regex(
        R"(^(.*?)_(.*?)_(\d+)_(STATION_PARMA|STATION_CONTROL)_(\d{10,13})_(\d+)\.dat$)");

    const QRegularExpressionMatch match = regex.match(baseName);
    if (!match.hasMatch()) {
        return false;
    }

    timestampStr = match.captured(5);
    sequenceNum = match.captured(6).toInt();
    return true;
}

QString extractTimestampFromFilename(const QString& filename)
{
    QString timestampStr;
    int sequenceNum = 0;
    if (parseFilenameComponents(filename, timestampStr, sequenceNum)) {
        return timestampStr;
    }
    return "";
}

int extractSequenceFromFilename(const QString& filename)
{
    QString timestampStr;
    int sequenceNum = 1;
    if (parseFilenameComponents(filename, timestampStr, sequenceNum)) {
        return sequenceNum;
    }
    return 1;
}

QString formatLogTime(const QString& timestampStr)
{
    if (timestampStr.isEmpty()) {
        return "";
    }

    bool ok = false;
    const qlonglong timestamp = timestampStr.toLongLong(&ok);
    if (!ok) {
        return "";
    }

    QDateTime dateTime;
    if (timestampStr.length() == 13) {
        dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
    }
    else if (timestampStr.length() == 10) {
        dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    }
    else if (timestamp > 1000000000000LL) {
        dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
    }
    else {
        dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
    }

    if (!dateTime.isValid()) {
        return "";
    }

    return dateTime.toUTC().toString("yyyyMMddTHH:mm:ssZ");
}

std::string calculateFileMD5(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for MD5 calculation:" << filename;
        return "";
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    const qint64 bufferSize = 1024 * 16;
    char buffer[bufferSize];
    qint64 bytesRead = 0;

    while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
        hash.addData(buffer, bytesRead);
    }

    file.close();
    return hash.result().toHex().toStdString();
}
}

LogUploadService::LogUploadService(QObject* parent)
    : QObject(parent)
    , m_uploadCheckTimer(nullptr)
    , m_statusCheckTimer(nullptr)
    , m_isUploading(false)
{
}

void LogUploadService::configure(const LogRuntimeConfig& config,
                                 std::function<QString(LogChannel)> currentFileProvider,
                                 std::function<QJsonObject()> metadataProvider)
{
    m_config = config;
    m_currentFileProvider = std::move(currentFileProvider);
    m_metadataProvider = std::move(metadataProvider);
}

LogStartupContext LogUploadService::loadStartupContext()
{
    LogStartupContext startupContext;
    startupContext.maxFileSizeBytes = m_config.defaultMaxFileSizeBytes;
    startupContext.sortie = getSortieWithRetry(10, 1000);

    if (!loadRemoteConfig(startupContext)) {
        qWarning() << "Failed to load log file configuration, using defaults";
    }

    return startupContext;
}

void LogUploadService::start()
{
    if (!m_uploadCheckTimer) {
        m_uploadCheckTimer = new QTimer(this);
        m_uploadCheckTimer->setInterval(3000);
        m_uploadCheckTimer->setSingleShot(false);
        connect(m_uploadCheckTimer, &QTimer::timeout,
                this, &LogUploadService::checkAndUploadCompletedFiles);
    }

    if (!m_statusCheckTimer) {
        m_statusCheckTimer = new QTimer(this);
        m_statusCheckTimer->setInterval(60000);
        m_statusCheckTimer->setSingleShot(false);
        connect(m_statusCheckTimer, &QTimer::timeout, this, [this]() {
            QMutexLocker locker(&m_uploadMutex);
            if (m_isUploading) {
                qWarning() << "Upload status has been 'uploading' for more than 60 seconds, resetting...";
                m_isUploading = false;
                QTimer::singleShot(0, this, &LogUploadService::processUploadQueue);
            }
        });
    }

    m_uploadCheckTimer->start();
    m_statusCheckTimer->start();

    qDebug() << "Upload check timer started, interval: 3 seconds";
    uploadAllEligibleFiles();
}

void LogUploadService::enqueueTask(const LogUploadTask& task)
{
    if (!task.isValid()) {
        qWarning() << "Invalid upload task provided";
        return;
    }

    const QString filePath = normalizedAbsolutePath(task.filePath);
    if (!QFile::exists(filePath)) {
        qWarning() << "File does not exist for scheduling:" << filePath;
        return;
    }

    bool shouldScheduleProcessing = false;
    {
        QMutexLocker locker(&m_uploadMutex);
        for (const LogUploadTask& existingTask : m_pendingUploadTasks) {
            if (normalizedAbsolutePath(existingTask.filePath) == filePath) {
                qDebug() << "File already in upload queue:" << filePath;
                return;
            }
        }

        LogUploadTask normalizedTask = task;
        normalizedTask.filePath = filePath;
        if (normalizedTask.dataType.isEmpty()) {
            normalizedTask.dataType = detectDataType(filePath);
        }

        m_pendingUploadTasks.append(normalizedTask);
        sortQueueByTimestampDescending();
        qDebug() << "File scheduled for upload:" << normalizedTask.filePath
                 << "Queue size:" << m_pendingUploadTasks.size();

        shouldScheduleProcessing = !m_isUploading;
    }

    if (shouldScheduleProcessing) {
        QTimer::singleShot(0, this, &LogUploadService::processUploadQueue);
    }
}

void LogUploadService::enqueueTasks(const QList<LogUploadTask>& tasks)
{
    for (const LogUploadTask& task : tasks) {
        enqueueTask(task);
    }
}

void LogUploadService::shutdown(const QList<LogUploadTask>& finalTasks)
{
    qInfo() << "Uploading all remaining files before shutdown...";

    if (m_uploadCheckTimer) {
        m_uploadCheckTimer->stop();
    }

    if (m_statusCheckTimer) {
        m_statusCheckTimer->stop();
    }

    enqueueTasks(finalTasks);
    uploadAllEligibleFiles();

    if (pendingTaskCount() > 0) {
        QTimer::singleShot(0, this, &LogUploadService::processUploadQueue);
    }

    QElapsedTimer timer;
    timer.start();
    while (pendingTaskCount() > 0 && timer.elapsed() < 10000) {
        QCoreApplication::processEvents();
        QThread::msleep(100);
    }

    qInfo() << "Remaining files upload completed, pending:" << pendingTaskCount();
}

void LogUploadService::checkAndUploadCompletedFiles()
{
    qDebug() << "Checking for completed files to upload...";
    uploadAllEligibleFiles();
    qDebug() << "Completed file check, pending uploads:" << pendingTaskCount();
}

void LogUploadService::processUploadQueue()
{
    const int maxRetry = 3;
    LogUploadTask currentTask;

    {
        QMutexLocker locker(&m_uploadMutex);
        if (m_isUploading || m_pendingUploadTasks.isEmpty()) {
            return;
        }

        m_isUploading = true;
        currentTask = m_pendingUploadTasks.first();
    }

    qDebug() << "Processing upload for file:" << currentTask.filePath;

    bool success = false;
    int retryCount = 0;
    while (retryCount < maxRetry && !success) {
        if (retryCount > 0) {
            qDebug() << "Retry" << retryCount << "for file:" << currentTask.filePath;
            QThread::msleep(1000);
        }

        try {
            success = uploadFileInternal(currentTask);
        }
        catch (...) {
            qWarning() << "Exception during upload";
            success = false;
        }

        ++retryCount;
    }

    bool shouldScheduleNext = false;
    const QString normalizedFilePath = normalizedAbsolutePath(currentTask.filePath);
    {
        QMutexLocker locker(&m_uploadMutex);
        for (int index = m_pendingUploadTasks.size() - 1; index >= 0; --index) {
            if (normalizedAbsolutePath(m_pendingUploadTasks.at(index).filePath) == normalizedFilePath) {
                m_pendingUploadTasks.removeAt(index);
            }
        }

        if (success) {
            if (QFile::exists(normalizedFilePath) && !QFile::remove(normalizedFilePath)) {
                qWarning() << "Failed to delete uploaded file:" << normalizedFilePath;
            }
            else {
                qInfo() << "File uploaded successfully:" << normalizedFilePath;
            }
        }
        else if (QFile::exists(normalizedFilePath)) {
            m_pendingUploadTasks.append(LogUploadTask{ normalizedFilePath, currentTask.dataType });
            sortQueueByTimestampDescending();
            qWarning() << "File failed after" << maxRetry
                       << "attempts, moved to queue:" << normalizedFilePath;
        }
        else {
            qWarning() << "File no longer exists, not re-adding to queue:" << normalizedFilePath;
        }

        m_isUploading = false;
        shouldScheduleNext = !m_pendingUploadTasks.isEmpty();
    }

    if (shouldScheduleNext) {
        QTimer::singleShot(0, this, &LogUploadService::processUploadQueue);
    }
}

void LogUploadService::uploadAllEligibleFiles()
{
    const QString currentRxFilePath =
        m_currentFileProvider ? m_currentFileProvider(LogChannel::Rx) : QString();
    const QString currentTxFilePath =
        m_currentFileProvider ? m_currentFileProvider(LogChannel::Tx) : QString();

    scheduleEligibleFilesFromDirectory(m_config.rxFileFolder, currentRxFilePath);
    scheduleEligibleFilesFromDirectory(m_config.txFileFolder, currentTxFilePath);
}

void LogUploadService::scheduleEligibleFilesFromDirectory(const QString& directoryPath,
                                                          const QString& currentFilePath)
{
    QDir dir(directoryPath);
    const QStringList files = dir.entryList(QStringList() << "*.dat", QDir::Files);

    for (const QString& fileName : files) {
        const QString filePath = dir.absoluteFilePath(fileName);
        if (shouldScheduleFileForUpload(filePath, currentFilePath)) {
            enqueueTask(LogUploadTask{ filePath, detectDataType(filePath) });
        }
    }
}

bool LogUploadService::shouldScheduleFileForUpload(const QString& filePath,
                                                   const QString& currentFilePath) const
{
    if (filePath.isEmpty()) {
        return false;
    }

    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    if (!currentFilePath.isEmpty() &&
        fileInfo.absoluteFilePath() == QFileInfo(currentFilePath).absoluteFilePath()) {
        return false;
    }

    return isFileReadyForUpload(fileInfo.absoluteFilePath());
}

bool LogUploadService::isFileReadyForUpload(const QString& filePath) const
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    const QDateTime lastModified = fileInfo.lastModified();
    const QDateTime now = QDateTime::currentDateTime();
    if (lastModified.secsTo(now) < 2) {
        return false;
    }

    if (fileInfo.size() < 1024) {
        return false;
    }

    return true;
}

void LogUploadService::sortQueueByTimestampDescending()
{
    std::sort(m_pendingUploadTasks.begin(), m_pendingUploadTasks.end(),
              [](const LogUploadTask& left, const LogUploadTask& right) {
                  const QString timeStrLeft = extractTimestampFromFilename(left.filePath);
                  const QString timeStrRight = extractTimestampFromFilename(right.filePath);
                  const int seqLeft = extractSequenceFromFilename(left.filePath);
                  const int seqRight = extractSequenceFromFilename(right.filePath);

                  bool okLeft = false;
                  bool okRight = false;
                  const qint64 timeLeft = timeStrLeft.toLongLong(&okLeft);
                  const qint64 timeRight = timeStrRight.toLongLong(&okRight);

                  if (okLeft && okRight) {
                      if (timeLeft != timeRight) {
                          return timeLeft > timeRight;
                      }
                      if (seqLeft != seqRight) {
                          return seqLeft > seqRight;
                      }
                      return left.filePath < right.filePath;
                  }

                  if (okLeft != okRight) {
                      return okLeft;
                  }

                  if (seqLeft != seqRight) {
                      return seqLeft > seqRight;
                  }

                  return left.filePath < right.filePath;
              });
}

bool LogUploadService::uploadFileInternal(const LogUploadTask& task)
{
    QFileInfo fileInfo(task.filePath);
    const QString dataType = task.dataType.isEmpty() ? detectDataType(task.filePath) : task.dataType;

    if (!fileInfo.exists()) {
        qWarning() << "[" << dataType << "] File does not exist:" << task.filePath;
        return false;
    }

    const QString filename = fileInfo.fileName();
    const qint64 fileSize = fileInfo.size();
    const QString sortie = extractSortieFromFilename(filename);

    CurlUploader curlUploader;
    curlUploader.setLogRequestUrl(m_config.uploadUrl + m_config.apiPath);
    curlUploader.setAppIdSecret(m_config.appId, m_config.appSecret);
    curlUploader.setLogParam(m_config.deviceModel,
                             m_config.deviceSn,
                             sortie.toStdString(),
                             dataType.toStdString(),
                             "GROUND_STATION");

    const std::string uploadUrl = curlUploader.requestLogUrl(filename.toStdString(), fileSize);
    if (uploadUrl.empty()) {
        qWarning() << "[" << dataType << "] Failed to get upload URL for file:" << filename;
        return false;
    }

    qInfo() << "[" << dataType << "] Upload URL:" << QString::fromStdString(uploadUrl);

    QFile file(task.filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[" << dataType << "] Failed to open file:" << task.filePath;
        return false;
    }

    const QByteArray content = file.readAll();
    file.close();

    if (content.size() != fileSize) {
        qWarning() << "[" << dataType << "] Failed to read complete file content:" << task.filePath;
        return false;
    }

    curlUploader.setMethod(CurlUploader::Method::PUT);
    curlUploader.setVerifySSL(false);

    if (!curlUploader.uploadData(uploadUrl, content.toStdString(), filename.toStdString())) {
        qWarning() << "[" << dataType << "] Upload failed for file:" << filename;
        return false;
    }

    std::string calculatedMd5 = calculateFileMD5(task.filePath);
    std::transform(calculatedMd5.begin(), calculatedMd5.end(), calculatedMd5.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });

    const std::string responseMd5 = curlUploader.getResponseMD5();
    qInfo() << "[" << dataType << "] Local MD5:" << QString::fromStdString(calculatedMd5);
    qInfo() << "[" << dataType << "] Server MD5:" << QString::fromStdString(responseMd5);

    if (responseMd5 != calculatedMd5) {
        qWarning() << "[" << dataType << "] MD5 verification failed for file:" << filename;
        qWarning() << "[" << dataType << "] Local:" << QString::fromStdString(calculatedMd5)
                   << "Server:" << QString::fromStdString(responseMd5);
        return false;
    }

    QString timestampStr;
    int uploadSequence = 0;
    if (!parseFilenameComponents(filename, timestampStr, uploadSequence)) {
        qWarning() << "[" << dataType << "] Failed to parse filename components:" << filename;
        return true;
    }

    const QString logTime = formatLogTime(timestampStr);
    const QJsonObject metadataObject = m_metadataProvider ? m_metadataProvider() : QJsonObject();
    const QJsonDocument metadataDocument(metadataObject);
    const std::string metadata =
        QString::fromUtf8(metadataDocument.toJson(QJsonDocument::Compact)).toStdString();

    if (!curlUploader.uploadSuccessReport(m_config.successReportUrl,
                                          logTime.toStdString(),
                                          uploadSequence,
                                          metadata)) {
        qWarning() << "[" << dataType << "] Failed to report upload success for file:" << filename;
    }
    else {
        qDebug() << "[" << dataType << "] Upload success report sent successfully";
    }

    return true;
}

qint64 LogUploadService::getSortieWithRetry(int maxRetries, int retryDelayMs) const
{
    CurlUploader curlUploader;
    curlUploader.setLogRequestUrl(m_config.sortieUrl);
    curlUploader.setAppIdSecret(m_config.appId, m_config.appSecret);

    for (int retryCount = 0; retryCount < maxRetries; ++retryCount) {
        if (retryCount > 0) {
            qDebug() << "Retry attempt" << retryCount + 1 << "of" << maxRetries << "to get sortie";
            QThread::msleep(retryDelayMs);
        }

        const std::string sortieStr = curlUploader.getSortie(m_config.sortieUrl, m_config.deviceSn);
        bool ok = false;
        const qint64 sortie = QString::fromStdString(sortieStr).toLongLong(&ok);
        if (ok && !sortieStr.empty()) {
            qDebug() << "Successfully obtained sortie on attempt" << retryCount + 1;
            return sortie;
        }
    }

    const qint64 timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qDebug() << "Failed to get sortie after" << maxRetries
             << "attempts, using timestamp:" << timestamp;
    return timestamp;
}

bool LogUploadService::loadRemoteConfig(LogStartupContext& startupContext) const
{
    CurlUploader curlUploader;
    curlUploader.setLogConfigUrl(m_config.logFileConfigUrl);
    curlUploader.setAppIdSecret(m_config.appId, m_config.appSecret);

    if (!curlUploader.getLogFileConfig(1, 1)) {
        qWarning() << "Failed to get log file config";
        return false;
    }

    startupContext.td550FileSizeMB = curlUploader.getTd550FileSize();
    startupContext.t1400FileSizeMB = curlUploader.getT1400FileSize();
    startupContext.r6000FileSizeMB = curlUploader.getR6000FileSize();

    const std::vector<LogFileConfig> logConfigs = curlUploader.getLogConfigs();
    if (!logConfigs.empty()) {
        startupContext.fileMergeTimeMinutes = logConfigs.front().fileMergeTime;
    }

    startupContext.maxFileSizeBytes = resolveMaxFileSizeBytes(startupContext.td550FileSizeMB,
                                                              startupContext.t1400FileSizeMB,
                                                              startupContext.r6000FileSizeMB);

    qInfo() << "=== Log File Configuration ===";
    qInfo() << "TD550 File Size:" << startupContext.td550FileSizeMB << "MB";
    qInfo() << "T1400 File Size:" << startupContext.t1400FileSizeMB << "MB";
    qInfo() << "R6000 File Size:" << startupContext.r6000FileSizeMB << "MB";
    qInfo() << "File Merge Time:" << startupContext.fileMergeTimeMinutes << "minutes";
    qInfo() << "===============================";
    qInfo() << "Resolved max file size:" << startupContext.maxFileSizeBytes / 1024 / 1024 << "MB";

    return true;
}

qint64 LogUploadService::resolveMaxFileSizeBytes(int td550FileSizeMB,
                                                 int t1400FileSizeMB,
                                                 int r6000FileSizeMB) const
{
    if (m_config.deviceModel == "TD550") {
        return static_cast<qint64>(td550FileSizeMB) * 1024 * 1024;
    }
    if (m_config.deviceModel == "T1400") {
        return static_cast<qint64>(t1400FileSizeMB) * 1024 * 1024;
    }
    if (m_config.deviceModel == "R6000") {
        return static_cast<qint64>(r6000FileSizeMB) * 1024 * 1024;
    }
    return m_config.defaultMaxFileSizeBytes;
}

int LogUploadService::pendingTaskCount() const
{
    QMutexLocker locker(&m_uploadMutex);
    return m_pendingUploadTasks.size();
}
