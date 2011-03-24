#ifndef EASYSYNCSERVICE_H
#define EASYSYNCSERVICE_H

#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QSettings>

#include "qt/qtservice.h"
#include "easysyncserver.h"

class EasysyncService : public QtService<QCoreApplication>
{
public:
    EasysyncService(int argc, char **argv);
    virtual ~EasysyncService();

protected:
    void start();
    void stop();
    void pause();
    void resume();

private:
    EasysyncServer *daemon;
};

#endif // EASYSYNCSERVICE_H
