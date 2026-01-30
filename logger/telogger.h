#pragma once

#include <QObject>

/**
 * Настройка - все настройки меняются в автозаполняемом конфиг-файле log.ini
 * Использование:
 *  хотя бы один раз вызвать метод Logger::instance(), после этого все Qt-шные сообщения будут дублироваться в файл
 */

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger& instance();

    void setLogFileNamePattern(const QString &fileName);
    void setLogDir(const QString &dirPath);
    QString getFullLogPath() const;
    void deleteLogs();
    void setLogsLifeTime(const int &days);
    void setLogLevel(const int &level);

public slots:
    void log(QString message, QtMsgType type);

signals:
    void logWritten(QString logMsg);

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void readConfigs();

    // Singleton
    Logger();
    Logger(const Logger&);
    Logger& operator=(const Logger&);

    QString logsDir_;
    QString logsName_;

    int logsLifeTime_;
    int logLevel_;

    bool dirCreated_;

    static constexpr int MAX_FNAME_LEN = 30;
};



