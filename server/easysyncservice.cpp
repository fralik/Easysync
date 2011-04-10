#include "easysyncservice.h"
#include "dbmanager.h"

#include <QtSql>
#include <QDebug>

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
    server = NULL;
}

EasysyncService::~EasysyncService()
{
    if (server)
    {
        delete server;
        server = NULL;
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
       Path to the configuration file is specified by '<daemon> --config <path>'.
       Using non default path is usefull for systems where $HOME directory may 
       be temporary (emdedded devices). Using this parameter it is possible to 
       store config file on the persistent storage.
    */
    QString configPath = "";

    // possible memory leak. In "A simple Service Controller" example this pointes is also not deleted.
    QCoreApplication *app = application();

    qDebug() << "going to process arguments" << app->argc();
    for (int i = 1; i < app->argc(); i++)
    {
        QString arg = app->argv()[i];
        qDebug() << "Got:" << arg;
        if (arg == QLatin1String("--adduser"))
        {
            username = QString::fromLocal8Bit(app->argv()[i+1]);
            i++;
        }
        else if (arg == QLatin1String("--config"))
        {
            configPath = QString::fromLocal8Bit(app->argv()[i+1]);
            i++;
        }
    }

    if (!username.isEmpty())
    {
        DbManager *dbManager = new DbManager();
        dbManager->initDbPath(configPath);
        dbManager->connect();
        if (!dbManager->createTables())
        {
            logMessage("Failed to create tables in the database");
            qDebug() << "Failed to create tables in the database";

        }
        if (!dbManager->addUser(username))
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

    if (!configPath.isEmpty())
    {
        QFileInfo configInfo(configPath);
        QSettings settings(configInfo.absoluteFilePath(), QSettings::IniFormat);
        port = settings.value("port").toUInt();
    }

    server = new EasysyncServer(port, app, configPath);
    if (!server->isListening())
    {
        logMessage(QString("Failed to bind to port %1").arg(port), QtServiceBase::Error);
        qDebug() << QString("Failed to bind to port %1").arg(port);

        stop();
        app->quit();
        return;
    }
    if (!server->dbManager.isConnected())
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
    if (server)
    {
        server->dbManager.disconnect();
        delete server;
        server = NULL;
    }
}

void EasysyncService::pause()
{
    server->pause();
}

void EasysyncService::resume()
{
    server->resume();
}

