#include "telogger.h"

#include <QDebug>
#include <QDateTime>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QThread>
#include <QSettings>
#include <QCoreApplication>
#include <QTimer>

Qt::ConnectionType Logger::logConnectionType {Qt::QueuedConnection};
QString Logger::confPath {};
int Logger::maxFnameLen {30};

Logger::Logger() :
    QObject         {nullptr},
    logsDir_        {qApp->applicationDirPath() + "/Logs"},
    logsName_       {"log_%1.log"},
    logsLifeTime_   {30},
    logLevel_       {0},
    dirCreated_     {false}
{
    qRegisterMetaType<QtMsgType>("QtMsgType");

    if (confPath.isEmpty())
    {
#if defined(Q_OS_WIN)
        confPath = qApp->applicationDirPath() + "/log.ini";
#else
        confPath = qApp->applicationDirPath() + "/../log.ini";
#endif
    }
    readConfigs();

    QThread* thread {new QThread {nullptr}};
    connect(qApp, &QCoreApplication::aboutToQuit, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &Logger::deleteLater);
    connect(thread, &QThread::started, this, [this, thread] {
        syncTimer = new QTimer(nullptr);
        connect(thread, &QThread::finished, syncTimer, &QTimer::deleteLater);
        syncTimer->setInterval(30'000);
        syncTimer->callOnTimeout(this, &Logger::readConfigs);
        syncTimer->start();
    });

    this->moveToThread(thread);
    thread->start();

    qInstallMessageHandler(Logger::messageHandler);
}

void Logger::setMaxFnameLen(int newMaxFnameLen)
{
    maxFnameLen = newMaxFnameLen;
}

void Logger::setLogConnectionType(Qt::ConnectionType newLogConnectionType)
{
    logConnectionType = newLogConnectionType;
}

Logger& Logger::instance()
{
    static Logger teLogger_;
    return teLogger_;
}

void Logger::setLogDir(const QString &dirPath)
{
    logsDir_ = dirPath;
    QDir dir {logsDir_};

    if (!dir.mkpath("."))
    {
        qCritical().noquote() << "Не удалось создать папку: " + logsDir_;
        throw;
    }

    dirCreated_ = true;
}

void Logger::setLogFileNamePattern(const QString &fileName)
{
    logsName_ = fileName;
}

QString Logger::getFullLogPath() const
{
    return logsName_;
}

void Logger::setConfPath(QString path)
{
    if (QString absolutePath = QFileInfo(path).absoluteFilePath();
        confPath != absolutePath)
    {
        confPath = absolutePath;
        QMetaObject::invokeMethod(&Logger::instance(), &Logger::deleteLogs, Qt::QueuedConnection);
        QMetaObject::invokeMethod(&Logger::instance(), &Logger::readConfigs, Qt::QueuedConnection);
    }
}

void Logger::deleteLogs()
{
    QDir dir {logsDir_};
    const QFileInfoList files {dir.entryInfoList()};

    qInfo().noquote() << "Отчистка старых логов из: " + logsDir_ + " начата";

    // Перебираем имена файлов в дирректории
    for (const QFileInfo &file : std::as_const(files))
    {
        if (!file.isFile())
            continue;
        const QString fileFullPath {file.absoluteFilePath()};
        // Если последнее изменение было > времени жизни логов, то удаляем этот файл
        if (file.lastModified().daysTo(QDateTime::currentDateTime()) > logsLifeTime_)
        {
            qInfo().noquote() <<  "Удаление файла: " + fileFullPath + (QFile::remove(fileFullPath) ? " успешно" : " не удалось");
        }
    }

    qInfo().noquote() << "Отчистка старых логов из: " + logsDir_ + " завершена";
}

void Logger::setLogsLifeTime(const int &days)
{
    if (days < 1)
    {
        logsLifeTime_ = 1;
        return;
    }

    logsLifeTime_ = days;
}

void Logger::setLogLevel(const int &level)
{
    if (level < 0)
    {
        logLevel_ = 0;
    }
    else if (level > 2)
    {
        logLevel_ = 2;
    }
    else
    {
        logLevel_ = level;
    }
}

void Logger::log(QString message, QtMsgType type)
{
    switch (logLevel_)
    {
    case 2:
    {
        if (type == QtInfoMsg)
        {
            return;
        }
        Q_FALLTHROUGH();
    }
    case 1:
    {
        if (type == QtDebugMsg)
        {
            return;
        }
        break;
    }
    }

    if (!dirCreated_)
    {
        setLogDir(logsDir_);
    }

    const QString curLogPath {(logsDir_ + "/" + logsName_).arg(QDate::currentDate().toString("dd.MM.yyyy"))};
    const bool needToDelete {!QFile::exists(curLogPath)};

    // Если файла не существует, значит либо ещё ни одного нет, либо наступил новый день
    if (needToDelete)
    {
        qInfo() << "Создан новый лог-файл: " + curLogPath;
    }

    QFile logFile {curLogPath};

    if (!logFile.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        qCritical() << "Не удалось открыть лог-файл:" << logFile.fileName() << logFile.errorString();
        return;
    }

    if (needToDelete)
    {
        deleteLogs();
    }

    QString logStr;
    QTextStream fStream {&logStr};

    fStream << (QTime::currentTime().toString("hh:mm:ss.zzz") + " | ");

    switch (type)
    {
    case QtMsgType::QtDebugMsg:
    {
        fStream << "DEBUG    | ";
        break;
    }
    case QtMsgType::QtInfoMsg:
    {
        fStream << "INFO     | ";
        break;
    }
    case QtMsgType::QtWarningMsg:
    {
        fStream << "WARNING  | ";
        break;
    }
    case QtMsgType::QtCriticalMsg:
    {
        fStream << "CRITICAL | ";
        break;
    }
    case QtMsgType::QtFatalMsg:
    {
        fStream << "FATAL    | ";
        break;
    }
    }

    fStream << message << '\n';
    fStream.flush();

    logFile.write(logStr.toUtf8());
    logFile.flush();
    logFile.close();

    emit logWritten(logStr);
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString functName (context.function);
    functName.truncate(functName.indexOf('('));
    if (functName.startsWith("virtual "))
    {
        functName.remove(0, 8);
    }
    functName.remove(0, functName.lastIndexOf(' ') + 1);

    if (functName.isEmpty())
    {
        functName = "...";
    }
    else if (functName.length() > maxFnameLen)
    {
        functName = functName.left(maxFnameLen - 3) + "...";
    }

    QString message =
        functName.leftJustified(maxFnameLen, ' ', true) +
        ", line " +
        QString::number(context.line).rightJustified(4) + " | " +
        msg;

    QTextStream stdStream (type == QtMsgType::QtDebugMsg || type == QtMsgType::QtInfoMsg ? stdout : stderr);

    stdStream << (QTime::currentTime().toString("hh:mm:ss.zzz") + " | ");

    switch (type)
    {
    case QtMsgType::QtDebugMsg:
    {
        stdStream << "\x1b[1;30mDEBUG\x1b[0m    | ";
        break;
    }
    case QtMsgType::QtInfoMsg:
    {
        stdStream << "\x1b[1;34mINFO\x1b[0m     | ";
        break;
    }
    case QtMsgType::QtWarningMsg:
    {
        stdStream << "\x1b[1;33mWARNING\x1b[0m  | ";
        break;
    }
    case QtMsgType::QtCriticalMsg:
    {
        stdStream << "\x1b[1;31mCRITICAL\x1b[0m | ";
        break;
    }
    case QtMsgType::QtFatalMsg:
    {
        stdStream << "\x1b[1;31mFATAL\x1b[0m    | ";
        break;
    }
    }

    stdStream << message << '\n';

#if (QT_VERSION_MAJOR < 6)
    QMetaObject::invokeMethod(&Logger::instance(), "log", logConnectionType,
                              Q_ARG(QString, message),
                              Q_ARG(QtMsgType, type));
#else
    QMetaObject::invokeMethod(&Logger::instance(), &Logger::log, logConnectionType,
                              message, type);
#endif

    // QString {context.function}.split('(').first().split(' ').last()
    // Убираем список аргументов и убираем тип возвращаемого значения
}

void Logger::readConfigs()
{
    QSettings conf {confPath, QSettings::IniFormat};
    // qInfo().noquote() << "Путь к конфигу логгера:" << conf.fileName();

    if (!conf.contains("LOGGER/log_level"))     conf.setValue("LOGGER/log_level", 1);
    if (!conf.contains("LOGGER/lifetime"))     conf.setValue("LOGGER/lifetime", 30);
#if defined(Q_OS_WIN)
    if (!conf.contains("LOGGER/dir"))          conf.setValue("LOGGER/dir",      QCoreApplication::applicationDirPath() + "/Logs/");
#else
    if (!conf.contains("LOGGER/dir"))          conf.setValue("LOGGER/dir",      QCoreApplication::applicationDirPath() + "/../Logs/");
#endif
    if (!conf.contains("LOGGER/name_pattern")) conf.setValue("LOGGER/name_pattern", "log_%1.log");

    setLogLevel(conf.value("LOGGER/log_level").toInt());
    setLogDir(conf.value("LOGGER/dir").toString());
    setLogsLifeTime(conf.value("LOGGER/lifetime").toInt());
    setLogFileNamePattern(conf.value("LOGGER/name_pattern").toString());
}




