#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include <telogger.h>

int main(int argc, char *argv[])
{
    QCoreApplication a (argc, argv);
    Logger::instance();
    qInfo().noquote()       << "Hello logger";
    qDebug().noquote()      << "It's debug message";
    qWarning().noquote()    << "It's warning message";
    qCritical().noquote()   << "It's critical message";
    qInfo().noquote()       << "It's multiline message:" << Qt::endl << "line 1" << Qt::endl << "line 2";

    return a.exec();
}
