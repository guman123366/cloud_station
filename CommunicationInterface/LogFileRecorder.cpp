#include "LogFileRecorder.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>

namespace {
QString channelKey(LogChannel channel)
{
    return channel == LogChannel::Rx ? "rx" : "tx";
}
}

LogFileRecorder::LogFileRecorder(const LogRuntimeConfig& config)
    : m_config(config)
    , m_rxFile(nullptr)
    , m_txFile(nullptr)
    , m_currentRxFileSize(0)
    , m_currentTxFileSize(0)
{
}

LogFileRecorder::~LogFileRecorder()
{
    QList<LogUploadTask> unusedTasks = shutdown();
    Q_UNUSED(unusedTasks);
}

bool LogFileRecorder::initialize(const LogStartupContext& startupContext)
{
    m_startupContext = startupContext;

    if (!createDataDirectories()) {
        return false;
    }

    return initFiles();
}

QList<LogUploadTask> LogFileRecorder::appendData(LogChannel channel, const QByteArray& data)
{
    QList<LogUploadTask> uploadTasks;

    QFile*& file = fileForChannel(channel);
    QString& currentFileName = fileNameForChannel(channel);
    qint64& currentFileSize = fileSizeForChannel(channel);

    if (!file || !file->isOpen()) {
        qWarning() << (channel == LogChannel::Rx ? "Rx" : "Tx")
                   << "file is not available for writing";
        return uploadTasks;
    }

    const QString oldFileName = currentFileName;
    bool fileSwitched = false;

    const qint64 bytesWritten = file->write(data);
    if (bytesWritten != data.size()) {
        qWarning() << "Failed to write all data, written:" << bytesWritten << "of" << data.size();
    }

    currentFileSize += bytesWritten;

    if (m_config.enableFileSplit && currentFileSize >= m_startupContext.maxFileSizeBytes) {
        qDebug() << (channel == LogChannel::Rx ? "Rx" : "Tx")
                 << "file reached size limit (" << currentFileSize << "bytes), creating new file";
        fileSwitched = true;

        if (!oldFileName.isEmpty() && QFile::exists(oldFileName)) {
            uploadTasks.append(LogUploadTask{ oldFileName, dataTypeForChannel(channel) });
        }

        currentFileName = generateFileName(channel);
        if (openNewFile(file, currentFileName)) {
            file->write(data);
            currentFileSize = data.size();
            qDebug() << "New file created:" << currentFileName;
        }
    }

    if (file) {
        file->flush();
    }

    if (!fileSwitched && currentFileSize >= (m_startupContext.maxFileSizeBytes * 9) / 10) {
        qDebug() << (channel == LogChannel::Rx ? "Rx" : "Tx")
                 << "file approaching size limit:" << currentFileSize
                 << "/" << m_startupContext.maxFileSizeBytes << "bytes";
    }

    return uploadTasks;
}

QList<LogUploadTask> LogFileRecorder::shutdown()
{
    QList<LogUploadTask> uploadTasks;

    const auto finalizeFile = [&uploadTasks](QFile*& file,
                                             const QString& fileName,
                                             const QString& dataType) {
        if (!file) {
            return;
        }

        if (file->isOpen()) {
            file->flush();
            file->close();
        }

        delete file;
        file = nullptr;

        if (!fileName.isEmpty() && QFile::exists(fileName)) {
            QFileInfo fileInfo(fileName);
            if (fileInfo.size() >= 1024) {
                uploadTasks.append(LogUploadTask{ fileName, dataType });
            }
        }
    };

    finalizeFile(m_rxFile, m_currentRxFileName, dataTypeForChannel(LogChannel::Rx));
    finalizeFile(m_txFile, m_currentTxFileName, dataTypeForChannel(LogChannel::Tx));

    return uploadTasks;
}

QString LogFileRecorder::currentFilePath(LogChannel channel) const
{
    return channel == LogChannel::Rx ? m_currentRxFileName : m_currentTxFileName;
}

QString LogFileRecorder::directoryPath(LogChannel channel) const
{
    return channel == LogChannel::Rx ? m_config.rxFileFolder : m_config.txFileFolder;
}

bool LogFileRecorder::createDataDirectories() const
{
    QDir dir;

    const QString dataDir = m_config.appDir + "/" + m_config.dataFolder;
    if (!dir.exists(dataDir) && !dir.mkpath(dataDir)) {
        qWarning() << "Failed to create data directory:" << dataDir;
        return false;
    }

    if (!dir.exists(m_config.rxFileFolder) && !dir.mkpath(m_config.rxFileFolder)) {
        qWarning() << "Failed to create RxFile directory:" << m_config.rxFileFolder;
        return false;
    }

    if (!dir.exists(m_config.txFileFolder) && !dir.mkpath(m_config.txFileFolder)) {
        qWarning() << "Failed to create TxFile directory:" << m_config.txFileFolder;
        return false;
    }

    qDebug() << "Data directories created successfully";
    return true;
}

bool LogFileRecorder::initFiles()
{
    m_currentRxFileName = generateFileName(LogChannel::Rx);
    m_currentTxFileName = generateFileName(LogChannel::Tx);

    if (!openNewFile(m_rxFile, m_currentRxFileName)) {
        qWarning() << "Failed to open Rx file:" << m_currentRxFileName;
        return false;
    }

    if (!openNewFile(m_txFile, m_currentTxFileName)) {
        qWarning() << "Failed to open Tx file:" << m_currentTxFileName;
        return false;
    }

    qDebug() << "Data files initialized:";
    qDebug() << "  Rx file:" << m_currentRxFileName;
    qDebug() << "  Tx file:" << m_currentTxFileName;
    return true;
}

QString LogFileRecorder::generateFileName(LogChannel channel) const
{
    const QDateTime dateTime = QDateTime::currentDateTime();
    const quint64 timestamp = dateTime.currentMSecsSinceEpoch();

    static QMap<QString, int> sequenceMap;
    const QString key = channelKey(channel);
    const int sequence = sequenceMap.value(key, 0) + 1;
    sequenceMap.insert(key, sequence);

    const QString basePrefix =
        QString::fromStdString(m_config.deviceModel) + "_" +
        QString::fromStdString(m_config.deviceSn) + "_" +
        QString::number(m_startupContext.sortie);

    if (channel == LogChannel::Rx) {
        return m_config.rxFileFolder + "/" + basePrefix +
               "_STATION_PARMA_" + QString::number(timestamp) +
               "_" + QString::number(sequence) + ".dat";
    }

    return m_config.txFileFolder + "/" + basePrefix +
           "_STATION_CONTROL_" + QString::number(timestamp) +
           "_" + QString::number(sequence) + ".dat";
}

bool LogFileRecorder::openNewFile(QFile*& file, const QString& fileName)
{
    if (file) {
        file->close();
        delete file;
        file = nullptr;
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << "Failed to open file:" << fileName;
        delete file;
        file = nullptr;
        return false;
    }

    if (fileName.contains("STATION_PARMA")) {
        m_currentRxFileSize = 0;
    }
    else if (fileName.contains("STATION_CONTROL")) {
        m_currentTxFileSize = 0;
    }
    else {
        qDebug() << "No file :" << fileName;
    }

    qDebug() << "New file opened:" << fileName;
    return true;
}

QFile*& LogFileRecorder::fileForChannel(LogChannel channel)
{
    return channel == LogChannel::Rx ? m_rxFile : m_txFile;
}

QString& LogFileRecorder::fileNameForChannel(LogChannel channel)
{
    return channel == LogChannel::Rx ? m_currentRxFileName : m_currentTxFileName;
}

qint64& LogFileRecorder::fileSizeForChannel(LogChannel channel)
{
    return channel == LogChannel::Rx ? m_currentRxFileSize : m_currentTxFileSize;
}

QString LogFileRecorder::dataTypeForChannel(LogChannel channel)
{
    return channel == LogChannel::Rx ? "STATION_PARMA" : "STATION_CONTROL";
}
