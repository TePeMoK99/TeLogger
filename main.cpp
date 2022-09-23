#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include "logger/logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Logger::instance();

    return a.exec();
}
