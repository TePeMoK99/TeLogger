#include "logger.h"

Logger::Logger() :
    QObject         {nullptr},
    logsDir_        {qApp->applicationDirPath() + "/Logs"},
    logsName_       {"log_%1.log"},
    logsLifeTime_   {30},
    logLevel_       {0},
    dirCreated_     {false}
{
    qRegisterMetaType<QtMsgType>("QtMsgType");

    readConfigs();

    QThread* thread {new QThread {nullptr}};
    connect(thread, &QThread::finished, this, &Logger::deleteLater);
    this->moveToThread(thread);
    thread->start();

    qInstallMessageHandler(Logger::messageHandler);
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
        qCritical() << "Не удалось создать папку: " + logsDir_;
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

void Logger::deleteLogs()
{
    QDir dir {logsDir_};
    const QStringList files {dir.entryList()};

    qInfo() << "Отчистка старых логов из: " + logsDir_ + " начата";

    // Перебираем имена файлов в дирректории
    for (const auto &fileName : files)
    {
        const QString fileFullPath {logsDir_ + "/" + fileName};
        // Если последнее изменение было > времени жизни логов, то удаляем этот файл
        if (QFileInfo {fileFullPath}.lastModified().daysTo(QDateTime::currentDateTime()) > logsLifeTime_)
        {
            qInfo() <<  "Удаление файла: " + fileFullPath + (QFile::remove(fileFullPath) ? " успешно" : " не удалось");
        }
    }

    qInfo() << "Отчистка старых логов из: " + logsDir_ + " завершена";
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
    logLevel_ = level;
}

void Logger::log(QString message, QtMsgType type)
{
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
    QTextStream stdStream {stdout};

    fStream << (QTime::currentTime().toString("hh:mm:ss.zzz") + " | ");
    stdStream << (QTime::currentTime().toString("hh:mm:ss.zzz") + " | ");

    switch (type)
    {
    case QtMsgType::QtDebugMsg:
    {
        fStream << "DEBUG    | ";
        stdStream << "\x1b[1;30mDEBUG\x1b[0m    | ";
        break;
    }
    case QtMsgType::QtInfoMsg:
    {
        fStream << "INFO     | ";
        stdStream << "\x1b[1;34mINFO\x1b[0m     | ";
        break;
    }
    case QtMsgType::QtWarningMsg:
    {
        fStream << "WARNING  | ";
        stdStream << "\x1b[1;33mWARNING\x1b[0m  | ";
        break;
    }
    case QtMsgType::QtCriticalMsg:
    {
        fStream << "CRITICAL | ";
        stdStream << "\x1b[1;31mCRITICAL\x1b[0m | ";
        break;
    }
    case QtMsgType::QtFatalMsg:
    {
        fStream << "FATAL    | ";
        stdStream << "\x1b[1;31mFATAL\x1b[0m    | ";
        break;
    }
    }

    fStream << message;
    stdStream << message;

    fStream << '\n';
    stdStream << '\n';

    fStream.flush();
    stdStream.flush();

    if (!(logLevel_ != 0 && type == QtMsgType::QtDebugMsg))
    {
        logFile.write(logStr.toUtf8());
        logFile.flush();
        logFile.close();
    }

    emit logWritten(logStr);
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logStr;
    QString functName {QString {context.function}.split('(').first().split(' ').last()};

    if (functName.isEmpty())
    {
        functName = "...";
    }

    if (functName.length() > MAX_FNAME_LEN)
    {
        functName = functName.mid(0, MAX_FNAME_LEN - 3) + "...";
    }


    logStr += functName.leftJustified(MAX_FNAME_LEN, ' ', true);
    logStr += QString {", line "};
    logStr += QString::number(context.line).rightJustified(4) + " | ";
    logStr += msg;

    QMetaObject::invokeMethod(&Logger::instance(), "log", Qt::QueuedConnection,
                              Q_ARG(QString, logStr),
                              Q_ARG(QtMsgType, type));

    // QString {context.function}.split('(').first().split(' ').last()
    // Убираем список аргументов и убираем тип возвращаемого значения
}

void Logger::readConfigs()
{
    QSettings conf {qApp->applicationDirPath() + "/../log.ini", QSettings::IniFormat};
    qInfo().noquote() << "Путь к конфигу логгера:" << conf.fileName();

    if (!conf.contains("LOGGER/debug"))        conf.setValue("LOGGER/debug",    false);
    if (!conf.contains("LOGGER/lifetime"))     conf.setValue("LOGGER/lifetime", 30);
    if (!conf.contains("LOGGER/dir"))          conf.setValue("LOGGER/dir",      QCoreApplication::applicationDirPath() + "/../Logs/");
    if (!conf.contains("LOGGER/name_pattern")) conf.setValue("LOGGER/name_pattern", "log_%1.log");

    setLogLevel(conf.value("LOGGER/debug").toBool() ? 0 : 1);
    setLogDir(conf.value("LOGGER/dir").toString());
    setLogsLifeTime(conf.value("LOGGER/lifetime").toInt());
    setLogFileNamePattern(conf.value("LOGGER/name_pattern").toString());
}




