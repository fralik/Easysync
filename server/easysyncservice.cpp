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

    //! Stores username of a user that should be added.
    /*!
      It is possible to add a user to the database from the command line. The
      command is: <daemon> --adduser <user_name> --quit.
    */
    QString username = "";

    //! Port number, which is used to listen to new connections.
    quint16 port = 1984;


    //! Command line parameter to quit application [optional].
    /*!
      The purpose is to use it in combination with adduser parameter. The
      application will quit after creating a user.
    */
    bool shouldQuit = false;

    //! Optional path to the configuration file.
    /*!
       It is usefull for systems where $HOME directory may be temporary (emdedded
       devices). Using this parameter you are able to store config file on the
       persistent storage.
    */
    QString config_file = "";

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
        else if (arg.contains("config", Qt::CaseInsensitive))
        {
            config_file = QString::fromLocal8Bit(app->argv()[i+1]);
            i++;
        }
    }

    //logMessage("Starting service Easysync Server");
    //qDebug() << "Starting service Easysync Server";

    daemon = new EasysyncServer(port, app, config_file);
    if (!daemon->isListening())
    {
        logMessage(QString("Failed to bind to port %1").arg(daemon->serverPort()), QtServiceBase::Error);
        qDebug() << QString("Failed to bind to port %1").arg(daemon->serverPort());
        app->quit();
    }
    if (!daemon->dbManager.isConnected())
    {
        stop();
        app->quit();
        return;
    }
    if (!username.isEmpty())
    {
        if (!daemon->dbManager.createTables())
        {
            logMessage("Failed to create tables in the database");
            qDebug() << "Failed to create tables in the database";

            stop();
            app->quit();
            return;
        }
        if (!daemon->dbManager.addUser(username))
        {
            logMessage("Failed to add user to the database");
            qDebug() << "Failed to add user to the database";

            stop();
            app->quit();
            return;
        }
    }

    if (shouldQuit)
    {
        stop();
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
