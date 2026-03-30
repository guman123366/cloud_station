#pragma once

#include "LogReportTypes.h"

#include <QJsonObject>
#include <QList>
#include <QMutex>
#include <QObject>
#include <functional>

class QTimer;

class LogUploadService : public QObject
{
    Q_OBJECT

public:
    explicit LogUploadService(QObject* parent = nullptr);

    void configure(const LogRuntimeConfig& config,
                   std::function<QString(LogChannel)> currentFileProvider,
                   std::function<QJsonObject()> metadataProvider);

    LogStartupContext loadStartupContext();
    void start();
    void enqueueTask(const LogUploadTask& task);
    void enqueueTasks(const QList<LogUploadTask>& tasks);
    void shutdown(const QList<LogUploadTask>& finalTasks);

private slots:
    void checkAndUploadCompletedFiles();
    void processUploadQueue();

private:
    void uploadAllEligibleFiles();
    void scheduleEligibleFilesFromDirectory(const QString& directoryPath,
                                            const QString& currentFilePath);
    bool shouldScheduleFileForUpload(const QString& filePath,
                                     const QString& currentFilePath) const;
    bool isFileReadyForUpload(const QString& filePath) const;
    void sortQueueByTimestampDescending();
    bool uploadFileInternal(const LogUploadTask& task);
    qint64 getSortieWithRetry(int maxRetries, int retryDelayMs) const;
    bool loadRemoteConfig(LogStartupContext& startupContext) const;
    qint64 resolveMaxFileSizeBytes(int td550FileSizeMB,
                                   int t1400FileSizeMB,
                                   int r6000FileSizeMB) const;
    int pendingTaskCount() const;

private:
    LogRuntimeConfig m_config;
    std::function<QString(LogChannel)> m_currentFileProvider;
    std::function<QJsonObject()> m_metadataProvider;

    QTimer* m_uploadCheckTimer;
    QTimer* m_statusCheckTimer;
    QList<LogUploadTask> m_pendingUploadTasks;
    mutable QMutex m_uploadMutex;
    bool m_isUploading;
};
