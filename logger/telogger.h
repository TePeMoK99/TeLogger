#pragma once

#include <QObject>

#include "telogger_global.h"

/**
 * Настройка - все настройки меняются в автозаполняемом конфиг-файле log.ini
 * Использование:
 *  хотя бы один раз вызвать метод Logger::instance(), после этого все Qt-шные сообщения будут дублироваться в файл
 */

class QSettings;
class QTimer;

class TELOGGER_EXPORT Logger : public QObject
{
    Q_OBJECT
public:
    static Logger& instance();

    QString getFullLogPath() const;

    static void setConfPath(QString path);
    static void setLogConnectionType(Qt::ConnectionType newLogConnectionType);

public slots:
    void log(QString message, QtMsgType type);

signals:
    void logWritten(QString logMsg);

private slots:
    void deleteLogs();
    void readConfigs();

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void setLogFileNamePattern(const QString &fileName);
    void setLogDir(const QString &dirPath);
    void setLogsLifeTime(const int &days);
    void setLogLevel(const int &level);

    // Singleton
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static QString confPath;

    QString logsDir_;
    QString logsName_;

    int logsLifeTime_;
    int logLevel_;

    bool dirCreated_;
    static Qt::ConnectionType logConnectionType;

    QTimer *syncTimer {nullptr};

    static constexpr int MAX_FNAME_LEN = 30;
};



