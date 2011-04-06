#include <QtSql>
#include <QDebug>

//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>

#include "easysyncservice.h"
#include "dbmanager.h"
EasysyncService::EasysyncService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, QString("Easysync-server daemon v%1").arg(VER))
{
    /*!
      adding version information to the service name results in the apearance of 
      strings like 'easysyncserverdaemonv111' in the log file. It is ugle, but 
      allows to view the version information.
    */
    
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
      command is: <daemon> --adduser <username>. If this argument is used, then
      daemon will quit after adding a user.
    */
    QString username = "";

    //! Port number, which is used to listen to new connections. Default is 1984.
    /*!
      Can be specified in configuration file.
    */
    quint16 port = 1984;


    //! Optional path to the configuration file.
    /*!
       It is usefull for systems where $HOME directory may be temporary (emdedded
       devices). Using this parameter you are able to store config file on the
       persistent storage.
    */
    QString config_file = "";

    // Shows version information and quits. This is for future releases. Now it makes little sense
    // because we can only view the version information if start daemon with -e argument.
    //bool show_version = false;

    // possible memory leak. In "A simple Service Controller" example this pointes is also not deleted.
    QCoreApplication *app = application();

    qDebug() << "going to process arguments" << app->argc();
    for (int i = 1; i < app->argc(); i++)
    {
        QString arg = app->argv()[i];
        qDebug() << "Got:" << arg;
        if (arg.contains("adduser", Qt::CaseInsensitive))
        {
            username = QString::fromLocal8Bit(app->argv()[i+1]);
            i++;
        }
        else if (arg.contains("config", Qt::CaseInsensitive))
        {
            config_file = QString::fromLocal8Bit(app->argv()[i+1]);
            i++;
        }
//        else if (arg.contains("version", Qt::CaseInsensitive) || arg.contains("-v", Qt::CaseInsensitive))
//        {
//            show_version = true;
//            qDebug() << "must show version";
//        }
    }

//    if (show_version)
//    {
//        QTextStream cout(stdout);
//        QString info = QString("Easysync Server, v%1, revision %2. Copyright (c) 2011 Vadim Frolov\n\n"
//            "This program comes with ABSOLUTELY NO WARRANTY;\n"
//            "This is free software, and you are welcome to redistribute it"
//            "under certain conditions.\n").arg(VER).arg(REV);
//        cout << info;
//        stop();
//        app->quit();
//        return;
//    }

    if (!username.isEmpty())
    {
        DbManager *dbManager = new DbManager();
        dbManager->initDbPath(config_file);
        dbManager->connect();
        if (!dbManager->createTables())
        {
            logMessage("Failed to create tables in the database");
            qDebug() << "Failed to create tables in the database";

        }
        else if (!dbManager->addUser(username))
        {
            logMessage("Failed to add user to the database");
            qDebug() << "Failed to add user to the database";
        }

        delete dbManager;

        // Quit after adding new user
        stop();
        app->quit();
        return;
    }

    if (!config_file.isEmpty())
    {
        QFileInfo config_info(config_file);
        QSettings settings(config_info.absoluteFilePath(), QSettings::IniFormat);
        port = settings.value("port").toUInt();
    }

    daemon = new EasysyncServer(port, app, config_file);
    if (!daemon->isListening())
    {
        logMessage(QString("Failed to bind to port %1").arg(daemon->serverPort()), QtServiceBase::Error);
        qDebug() << QString("Failed to bind to port %1").arg(daemon->serverPort());

        stop();
        app->quit();
        return;
    }
    if (!daemon->dbManager.isConnected())
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

