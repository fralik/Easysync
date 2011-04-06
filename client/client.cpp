#include "client.h"
#include "ui_client.h"

#include <QtGui>
#include <QSettings>
#include <QMessageBox>
#include <QHostInfo>
#include <QFileDialog>
#include <QDir>
#include <QtDebug>
#include <QProcess>
#include <QtNetwork>
#include <QtNetwork/QNetworkConfigurationManager>

Client::Client(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::Client),
    syncInProgress(false),
    needToSync(false)
{
    socketReady = false;
    keepAliveInterval = 60000; // 60 seconds
    notifyAllAfterSync = Easysync::NotifyNone;

    ui->setupUi(this);

    createActions();
    createTrayIcon();

    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(handleSaveButton()));
    connect(ui->changeButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    networkManager = new QNetworkConfigurationManager(this);
    connect(networkManager, SIGNAL(onlineStateChanged(bool)), this, SLOT(onlineStateChanged(bool)));

    setIcon(Easysync::NoConnectionIcon);
    setWindowIcon(QIcon(":/images/application.svg"));

    unisonProcess = new QProcess(this);
    /*
        The synchronisation is divided into two steps: one starts the synchronisation, another handles all the necessary things
        after synchronisation is done. Two steps are asynchronous. This allows client to send keepAlive messages during the sync
        procedure.
     */
    connect(unisonProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(handleSyncIsFinished(int, QProcess::ExitStatus)));

    trayIcon->show();

    syncTimer = new QTimer(this);
    connect(syncTimer, SIGNAL(timeout()), this, SLOT(handleSyncTimer()));

    serverKeepAliveTimer = new QTimer(this);
    connect(serverKeepAliveTimer, SIGNAL(timeout()), this, SLOT(sendKeepAlive()));

    server = new QTcpSocket(this);
    connect(server, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    connect(server, SIGNAL(connected()), this, SLOT(handleSocketConnected()));
    connect(server, SIGNAL(disconnected()), this, SLOT(handleSocketDisconnected()));

    connect(&watcher, SIGNAL(directoryChanged(QString)), this, SLOT(handleDirectoryChanged(QString)));

    checkSettings();
}

Client::~Client()
{
    server->disconnectFromHost();
    
    delete ui;
}

void Client::connectToServer()
{
    if (socketReady)
    {
        server->abort();
        server->disconnectFromHost();
        server->deleteLater();
    }

    qDebug() << "Trying to connect to the server";
    server->abort();
    server->connectToHost(ui->serverAddressEdit->text(), ui->portEdit->text().toInt());

    QTimer::singleShot(2000, this, SLOT(checkServerConnection()));
}

void Client::checkServerConnection()
{
    if (!socketReady && networkManager->isOnline())
    {
        qDebug() << "no conection, will try to connect in 5 seconds...";
        if (server->state() != QAbstractSocket::ConnectedState)
            QTimer::singleShot(5000, this, SLOT(connectToServer()));
    }
}

void Client::handleSocketConnected()
{
    qDebug() << "Connected to server";

    setIcon(Easysync::ConnectedIcon);

    socketReady = true;
    installWatcher();
    sendGreetings();

    serverKeepAliveTimer->setInterval(keepAliveInterval);
    serverKeepAliveTimer->start();
}

void Client::handleSocketDisconnected()
{
    qDebug() << "Disconnected from server";

    serverKeepAliveTimer->stop();

    setIcon(Easysync::NoConnectionIcon);

    socketReady = false;
    stopWatcher();

    // try to reconnect in case some accidental connection loss
    if (networkManager->isOnline())
        QTimer::singleShot(2000, this, SLOT(connectToServer()));
}

/* 
    Reads messages from the server. For the format of communication "protocol" refer to
    file easysyncserver.cpp in server folder.
 */ 
void Client::readFromServer()
{
    QRegExp searcher;
    quint16 messageId = 0;
    QString username = ui->usernameEdit->text();
    QString sentHostname;

    if (server->canReadLine())
    {
        QString message(server->readLine());
        message.chop(1);
        qDebug() << "Got" << message << "from the server";
        searcher.setPattern("^\\d+\\b");
        int pos = 0;
        if ( (pos = searcher.indexIn(message, 0)) != -1)
        {
            messageId = searcher.cap().toUInt();
        }
        else
        {
            qDebug() << "No ID in the message from server, disconnecting...";
            server->disconnectFromHost();
            return;
        }

        switch (messageId)
        {
            case 2:
                searcher.setPattern("<\\b[^>]*>");
                pos = searcher.indexIn(message, 0);
                if (pos == -1)
                {
                    server->disconnectFromHost();
                    return;
                }
                sentHostname.append(searcher.cap());
                sentHostname.chop(1);
                sentHostname.remove(0, 1);
                if (sentHostname != hostname)
                {
                    qDebug() << "Got wrong hostname" << sentHostname << "from the server, disconnecting";
                    server->disconnectFromHost();
                    return;
                }

                startSynchronisation(Easysync::NotifyNone);

                break;
            default:
                break;
        }
    }
}

void Client::sendKeepAlive()
{
    if (socketReady)
    {
        server->write("pingpong\n");
        server->flush();
    }
}

void Client::handleSyncIsFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "finished to sync";

    setIcon(Easysync::ConnectedIcon);

    installWatcher();

    if (socketReady)
    {
        if (notifyAllAfterSync == Easysync::NotifyAll)
            server->write(QString("3. Robin, notify all. <%1> from <%2>\n").arg(username).arg(hostname).toUtf8());
        else
            server->write(QString("2. Roger\n").toUtf8());
        server->flush();
    }

    syncInProgress = false;
    if (needToSync)
    {
        needToSync = false;
        startSynchronisation(Easysync::NotifyAll);
    }
}

void Client::sendGreetings()
{
    if (!socketReady)
    {
        return ;
    }
    server->write(QString("1. Robin, <%1> from <%2> is on position!\n").arg(username).arg(hostname).toUtf8());
    server->flush();
}

void Client::checkSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Easysync", "Easysync client");
    QString name = settings.value("account/username").toString();
    if (name.isEmpty() || name.isNull())
    {
        ui->hostnameEdit->setText(QHostInfo::localHostName());
        show();
    }
    else
    {
        // we have our settings, do not make additional checks
        ui->usernameEdit->setText(name);
        ui->hostnameEdit->setText(settings.value("account/hostname").toString()); // actually, we can remove this from settings
        ui->serverAddressEdit->setText(settings.value("serverAddress").toString());
        ui->syncdirEdit->setText(settings.value("syncdir").toString());
        ui->unisonProfileEdit->setText(settings.value("unisonProfile").toString());
        ui->portEdit->setText(settings.value("port").toString());

        username = ui->usernameEdit->text();
        hostname = ui->hostnameEdit->text();
        serverAddress = ui->serverAddressEdit->text();
        port = ui->portEdit->text();
        syncDir = ui->syncdirEdit->text();

        connectToServer();
    }
}

/* 
   This function is used because we can not use startSynchronisation with
   signal from timer (arguments mismatch).
 */
void Client::handleSyncTimer()
{
    startSynchronisation();
}

/**
    Performs the synchronisation and sends the message to the server.
    If notifyAll is set to true, then sends a message that server should notify
    all other clients about the synchronisation necessity.
*/
void Client::startSynchronisation(bool notifyAll)
{
    if (syncInProgress)
    {
        needToSync = true; // mark the need of synchronisation and return
        return;
    }
    syncInProgress = true;
    
    notifyAllAfterSync = notifyAll;
    setIcon(Easysync::SyncInProgressIcon);

    stopWatcher();

    QString command(QString("unison %1").arg(ui->unisonProfileEdit->text()));
    qDebug() << "About to invoke" << command;
    unisonProcess->start(command);
}

void Client::handleDirectoryChanged(const QString &path)
{

    qDebug() << "Change in directory" << path;
    // use timer to accumulate several events
    if (!syncTimer->isActive())
    {
        syncTimer->setSingleShot(true);
        syncTimer->start(200);
    }
}


void Client::handleSaveButton()
{
    QMessageBox msgBox;

    if (ui->usernameEdit->text().isEmpty())
    {
        msgBox.setText(tr("You should enter the username."));
        ui->usernameEdit->setFocus(Qt::OtherFocusReason);
        msgBox.exec();
        return ;
    }
    if (ui->serverAddressEdit->text().isEmpty())
    {
        msgBox.setText(tr("You should enter the server address."));
        ui->serverAddressEdit->setFocus(Qt::OtherFocusReason);
        msgBox.exec();
        return ;
    }
    if (ui->syncdirEdit->text().isEmpty())
    {
        msgBox.setText(tr("You should specify the directory for the synchronisation."));
        ui->changeButton->setFocus(Qt::OtherFocusReason);
        msgBox.exec();
        return ;
    }
    if (ui->unisonProfileEdit->text().isEmpty())
    {
        msgBox.setText(tr("You should specify the unison profile."));
        ui->unisonProfileEdit->setFocus(Qt::OtherFocusReason);
        msgBox.exec();
        return ;
    }

    stopWatcher();

    writeSettings();

    if (!socketReady)
        connectToServer();
    else
        installWatcher();

    close();
}

void Client::writeSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Easysync", "Easysync client");
    settings.setValue("account/username", ui->usernameEdit->text());
    settings.setValue("account/hostname", ui->hostnameEdit->text());
    settings.setValue("serverAddress", ui->serverAddressEdit->text());
    settings.setValue("port", ui->portEdit->text());
    settings.setValue("syncdir", ui->syncdirEdit->text());
    settings.setValue("unisonProfile", ui->unisonProfileEdit->text());

    username = ui->usernameEdit->text();
    hostname = ui->hostnameEdit->text();
    serverAddress = ui->serverAddressEdit->text();
    port = ui->portEdit->text();
    syncDir = ui->syncdirEdit->text();
}

void Client::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this);
    if (!dir.isEmpty())
    {
        ui->syncdirEdit->setText(dir);
    }
}

void Client::showAbout()
{
    QMessageBox msgBox;
    QString msg = tr("Easysync-client, v%1, revision %2. Copyright (c) 2011 Vadim Frolov\n\n"
        "This program comes with ABSOLUTELY NO WARRANTY;\n"
        "This is free software, and you are welcome to redistribute it "
        "under certain conditions.").arg(VER).arg(REV);

    msgBox.about(this, tr("About Easysync-client"), msg);
}

// actions for the tray menu
void Client::createActions()
{
    preferencesAction = new QAction(tr("&Preferences..."), this);
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(show()));

    aboutAction = new QAction(tr("&About..."), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Client::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(preferencesAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void Client::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
            show();
            break;
        default:
            ;
    }
}

void Client::setIcon(Easysync::AvailableIcons icon)
{
    QIcon iconFile;
    QString tip;
    switch (icon)
    {
        // PNG icons are used along wuth SVG icon becuase SVG looks blurry when scaled
        // to small sizes
        case Easysync::SyncInProgressIcon:
            iconFile = QIcon(":/images/sync_16x16.png");
            tip = tr("Synchronization is running...");
            break;
        case Easysync::NoConnectionIcon:
            iconFile = QIcon(":/images/disconnected_16x16.png");
            tip = tr("No internet or server connection");
            break;
        case Easysync::ConnectedIcon:
            iconFile = QIcon(":/images/application.svg");
            tip = tr("Running normally");
            break;
    }
    if (!iconFile.isNull())
    {
        trayIcon->setIcon(iconFile);
        trayIcon->setToolTip(tip);
    }
}

void Client::installWatcher()
{
#if defined(Q_OS_WIN)
    watcher.addPath(syncDir);
#else
    watcher.addPaths(subFolders(syncDir));
#endif
}

void Client::stopWatcher()
{
#if defined(Q_OS_WIN)
    watcher.removePath(syncDir);
#else
    watcher.removePaths(subFolders(syncDir));
#endif
}

void Client::onlineStateChanged(bool isOnline)
{
    if (!isOnline)
    {
        qDebug() << "Connection status changed to OFFLINE";
        setIcon(Easysync::NoConnectionIcon);
        stopWatcher();
    }
    else
    {
        connectToServer();
    }
}

QStringList Client::subFolders(const QString &folder)
{
    QDir dir(folder);
    QStringList dirList;

    dirList << folder;

    foreach (QString subdir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString fullPath(folder + QDir::separator() + subdir);
        dirList << subFolders(fullPath);
    }
    return dirList;
}
