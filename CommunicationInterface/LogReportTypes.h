#pragma once

#include <QtGlobal>
#include <QString>
#include <string>

enum class LogChannel {
    Rx,
    Tx
};

struct LogUploadTask {
    QString filePath;
    QString dataType;

    bool isValid() const
    {
        return !filePath.isEmpty() && !dataType.isEmpty();
    }
};

struct LogStartupContext {
    qint64 sortie = 0;
    qint64 maxFileSizeBytes = 5120000;
    int td550FileSizeMB = 1;
    int t1400FileSizeMB = 1;
    int r6000FileSizeMB = 1;
    int fileMergeTimeMinutes = 30;
};

struct LogRuntimeConfig {
    QString appDir;
    QString dataFolder;
    QString rxFileFolder;
    QString txFileFolder;
    qint64 defaultMaxFileSizeBytes = 5120000;
    bool enableFileSplit = true;
    QString filePrefix = "fileData";
    QString defaultLogType = "STATION_PARMA";
    QString appVersion = "V1.00.00";

    std::string uploadUrl = "https://cloud-test.uatair.com/lfy-api";
    std::string apiPath = "/manage/logFile/getUploadUrl";
    std::string apiSuccessPath = "/manage/logFile/record";
    std::string successReportUrl;
    std::string logFileConfigPath = "/manage/logFileConfig/list";
    std::string logFileConfigUrl;
    std::string sortiePath = "/log/logFileConfig/getSortie";
    std::string sortieUrl;

    std::string deviceModel = "TD550";
    std::string deviceSn = "6722390TD550";
    std::string appId = "f0fo5to874q6ciwanzfbsrk05v9p1vcm";
    std::string appSecret = "u63kg92fsoyo273zlv39gbnoqk7usa2n";

    static LogRuntimeConfig fromIni(const QString& appDir);
};
