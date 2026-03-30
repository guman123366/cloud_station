#pragma once

#include "LogReportTypes.h"

#include <QByteArray>
#include <QJsonObject>
#include <QObject>

class LogFileRecorder;
class LogUploadService;

class LogReportCoordinator : public QObject
{
    Q_OBJECT

public:
    explicit LogReportCoordinator(const QString& appDir, QObject* parent = nullptr);
    ~LogReportCoordinator() override;

    bool initialize();
    void shutdown();
    void appendData(LogChannel channel, const QByteArray& data);
    void updateFccVersion(unsigned short version);

private:
    QJsonObject buildMetadata() const;
    static QString formatFccVersion(unsigned short version);

private:
    QString m_appDir;
    LogRuntimeConfig m_runtimeConfig;
    LogFileRecorder* m_fileRecorder;
    LogUploadService* m_uploadService;
    unsigned short m_fccVersion;
    bool m_initialized;
};
