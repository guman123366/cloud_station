#pragma once

#include "LogReportTypes.h"

#include <QByteArray>
#include <QFile>
#include <QList>

class LogFileRecorder
{
public:
    explicit LogFileRecorder(const LogRuntimeConfig& config);
    ~LogFileRecorder();

    bool initialize(const LogStartupContext& startupContext);
    QList<LogUploadTask> appendData(LogChannel channel, const QByteArray& data);
    QList<LogUploadTask> shutdown();

    QString currentFilePath(LogChannel channel) const;
    QString directoryPath(LogChannel channel) const;

private:
    bool createDataDirectories() const;
    bool initFiles();
    QString generateFileName(LogChannel channel) const;
    bool openNewFile(QFile*& file, const QString& fileName);

    QFile*& fileForChannel(LogChannel channel);
    QString& fileNameForChannel(LogChannel channel);
    qint64& fileSizeForChannel(LogChannel channel);
    static QString dataTypeForChannel(LogChannel channel);

private:
    LogRuntimeConfig m_config;
    LogStartupContext m_startupContext;

    QFile* m_rxFile;
    QFile* m_txFile;
    QString m_currentRxFileName;
    QString m_currentTxFileName;
    qint64 m_currentRxFileSize;
    qint64 m_currentTxFileSize;
};
