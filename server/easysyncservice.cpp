#include "easysyncservice.h"

#include <QtSql>
#include <QDebug>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

EasysyncService::EasysyncService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Easysync Daemon")
{
    setServiceDescription("Maintain client connections for easy synchronization");
    setServiceFlags(QtServiceBase::CanBeSuspended);
    setStartupType(QtServiceController::AutoStartup);
    daemon = NULL;
}

EasysyncService::~EasysyncService()
{
    if (daemon)
    {
        delete daemon;
        daemon = NULL;
    }
}

void EasysyncService::start()
{
    QString username = "";
    quint16 port = 1984;
    bool shouldQuit = false;

    QCoreApplication *app = application();

    /* user can pass arguments: <daemon> -p <port> --<adduser> <username> --quit
       All arguments are optional. Default port is 1984.
       Argument adduser is used to add a user in the sqlite DB.
       Argument quit is used to quit the service. Can be used with adduser
     */

    for (int i = 1; i < app->argc(); i++)
    {
        QString arg = app->argv()[i];
        if (arg == "-p")
        {
            port = QString::fromLocal8Bit(app->argv()[i+1]).toUShort();
            i++;
        }
        else if (arg.contains("adduser", Qt::CaseInsensitive))
        {
            username = QString::fromLocal8Bit(app->argv()[i+1]);
            i++;
        }
        else if (arg.contains("quit", Qt::CaseInsensitive))
        {
            shouldQuit = true;
        }
    }

    //quint16 port = (app->argc() > 1) ?
            //QString::fromLocal8Bit(app->argv()[1]).toUShort() : 1984;


    logMessage("Starting service Easysync Server");
    qDebug() << "Starting service Easysync Server";

    daemon = new EasysyncServer(port, app);
    if (!daemon->isListening())
    {
        logMessage(QString("Failed to bind to port %1").arg(daemon->serverPort()), QtServiceBase::Error);
        qDebug() << QString("Failed to bind to port %1").arg(daemon->serverPort());
        app->quit();
    }
    if (!username.isEmpty())
    {
        if (!daemon->dbManager.createTables())
        {
            logMessage("Failed to create tables in the database");
            qDebug() << "Failed to create tables in the database";
            app->quit();
            return;
        }
        if (!daemon->dbManager.addUser(username))
        {
            logMessage("Failed to add user to the database");
            qDebug() << "Failed to add user to the database";
            app->quit();
            return;
        }
    }

    if (shouldQuit)
    {
        // need to disconnect, otherwise receive "connection is still in use" error
        daemon->dbManager.disconnect();
        app->quit();
        return;
    }

    logMessage("Service have been started");
    qDebug() << "Service have been started";
}

void EasysyncService::stop()
{
    if (daemon)
    {
        daemon->dbManager.disconnect();
        delete daemon;
        daemon = NULL;
    }
}

void EasysyncService::pause()
{
    daemon->pause();
}

void EasysyncService::resume()
{
    daemon->resume();
}
