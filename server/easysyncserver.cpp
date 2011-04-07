#include "easysyncserver.h"
#include "qt/qtservice.h"

#include <QtCore/QCoreApplication>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QDebug>

EasysyncServer::EasysyncServer(quint16 port, QObject *parent, const QString configPath)
    : QTcpServer(parent), disabled(false)
{
    qDebug() << "Starting Easysync Server on port" << QString("%1").arg(port);
    listen(QHostAddress::Any, port);

    connect(this, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

    dbManager.initDbPath(configPath);
    if (dbManager.connect())
    {
        keepAliveTimer = new QTimer(this);
        connect(keepAliveTimer, SIGNAL(timeout()), this, SLOT(checkClientsConnection()));
        keepAliveTimer->setInterval(5000);
    }
}

EasysyncServer::~EasysyncServer()
{
    stop();
}

void EasysyncServer::checkClientsConnection()
{
    QTime currentTime = QTime::currentTime();
    int seconds = 0;
    QList<QTcpSocket *> mustDisconnect;

    for (int i = 0; i < clientLastSeen.length(); i++)
    {
        QTime lastSeen = clientLastSeen.at(i);

        seconds = abs(currentTime.secsTo(lastSeen));

        if (seconds > 60)
            mustDisconnect.append(clientConnections.at(i));
    }
    for (int i = 0; i < mustDisconnect.length(); i++)
    {
        QTcpSocket *client = mustDisconnect.at(i);
        client->disconnectFromHost();
    }
}

void EasysyncServer::handleNewConnection()
{
    if (disabled)
        return;

    while (hasPendingConnections())
    {
        qDebug() << "Client connected";
        QTcpSocket *client = nextPendingConnection();
        connect(client, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
        connect(client, SIGNAL(readyRead()), this, SLOT(readClient()));
    }
}

void EasysyncServer::clientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    QString hostname;
    int socketId;

    if (!client)
        return;

    socketId = clientConnections.indexOf(client);
    if (socketId != -1)
    {
        hostname = peerNames.at(socketId);
        QTime lastSeen = clientLastSeen.at(socketId);

        dbManager.voidConnection(hostname);
        clientConnections.removeAll(client);
        peerNames.removeAll(hostname);
        clientLastSeen.removeAll(lastSeen);
    }

    qDebug() << "Client" << hostname << "disconnected";
    qDebug() << "There are" << clientConnections.length() << "connected clients";

    //QtServiceBase::instance()->logMessage(QString("Client %1 disconnected").arg(hostname));
    //QtServiceBase::instance()->logMessage(QString("There are %1 connected clients").arg(clientConnections.length()));

    if (clientConnections.length() == 0)
        keepAliveTimer->stop();

    client->deleteLater();
}

void EasysyncServer::sendToClient(QTcpSocket *socket, const QString& message)
{
    QTextStream os(socket);
    os.setAutoDetectUnicode(true);
    os << message;
    os.flush();
}

bool EasysyncServer::getUserInfo(const QString& message, QString *username, QString *hostname)
{
    QRegExp searcher;
    int pos = 0;

    if (!username || !hostname)
        return false;

    searcher.setPattern("<\\b[^>]*>");
    pos = 0;
    pos = searcher.indexIn(message, pos);
    if (pos == -1)
    {
        return false;
    }
    username->clear();
    username->append(searcher.cap());
    username->chop(1);
    username->remove(0, 1);

    hostname->clear();
    pos = searcher.indexIn(message, pos+1);
    if (pos == -1)
    {
        return false;
    }
    hostname->append(searcher.cap());
    hostname->chop(1);
    hostname->remove(0, 1);

    return true;
}

void EasysyncServer::readClient()
{
    quint16 messageId = 0;
    QRegExp searcher;
    QString username;
    QString peerHostname;
    int socketId = -1;

    if (disabled)
        return;

    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket->canReadLine())
    {
        QString clientMessage(socket->readLine());
        clientMessage.chop(0);
        qDebug() << "client send us " << clientMessage;

        if (clientMessage.contains("pingpong", Qt::CaseInsensitive))
        {
            socketId = clientConnections.indexOf(socket);
            if (socketId != -1)
                clientLastSeen.replace(socketId, QTime::currentTime());

            return ;
        }

        /* communication "protocol", all message should be lines (finished with '\n')
           <> does not mean "optional" here, but used to find necessary data in the message.
           0.
           pingpong message. It is sent from client to show that he is still alive.

           1.
           client -> 1. Robin, <username> from <peerHost> is on position!
           server -> "Roger." or "Negative."
           This should be the first message from a client. Server checks the username
           and hostname. If username is valid and hostname is not in the DB, then
           server adds hostname to the DB. If user is invalid, then server sends
           "negative" response. If server receives anything expect this message,
           then connection is closed.

           2.
           Message from the server to the client, that client must perform synchronization.
           server -> 2. <hostname>, sync is needed.
           client -> 2. Robin, over
           The answer of a client signals server, that synchronization is over.

           3.
           Something has changed on a client, it synchronizes the folder and sends message
           to the server.
           client -> 3. Robin, notify all. <username> from <peerHost>.
           server ->

           In fact, only the message ID and text inside <> is neede.
        */

        // extract message ID
        searcher.setPattern("^\\d+\\b");
        int pos = 0;
        if ( (pos = searcher.indexIn(clientMessage, 0)) != -1)
        {
            qDebug() << "Got message with ID" << searcher.cap();
            messageId = searcher.cap().toUInt();
        }
        else
        {
            // wrong protocol, terminate
            socket->disconnectFromHost();
            return;
        }

        switch (messageId)
        {
            case 1:
                if (!getUserInfo(clientMessage, &username, &peerHostname))
                {
                    qDebug() << "failed to get username and hostname";
                    socket->disconnectFromHost();
                    return;
                }

                if (!dbManager.isUserValid(username))
                {
                    qDebug() << "invalid user";
                    socket->write("Negative\n");
                    socket->disconnectFromHost();
                    return;
                }

                clientConnections.append(socket);
                peerNames.append(peerHostname);
                clientLastSeen.append(QTime::currentTime());
                socketId = clientConnections.indexOf(socket);

                if (clientConnections.length() == 1)
                    keepAliveTimer->start();

                if (!dbManager.addHostname(username, peerHostname, socketId))
                {
                    socket->disconnectFromHost();
                    return;
                }

                //QtServiceBase::instance()->logMessage(QString("Client %1 is connected").arg(peerHostname));
                qDebug() << QString("Client %1 is connected").arg(peerHostname);

                if (dbManager.isSyncNeeded(peerHostname))
                {
                    qDebug() << "Hostname" << peerHostname << "must sync";
                    notifyClient(socket, peerHostname);
                }
                break;

            case 2:
                socketId = clientConnections.indexOf(socket);
                peerHostname = peerNames.at(socketId);
                dbManager.syncIsDone(peerHostname);
                break;

            case 3:
                if (!getUserInfo(clientMessage, &username, &peerHostname))
                {
                    socket->disconnectFromHost();
                    return;
                }

                if (!dbManager.isUserValid(username))
                {
                    socket->write("Negative\n");
                    socket->disconnectFromHost();
                    return;
                }
                dbManager.setNotifyAllExcept(username, peerHostname);
                notifyClients(username);
                break;

            default:
                socket->disconnectFromHost();
                break;
        }
    }
}

void EasysyncServer::notifyClients(const QString &username)
{
    QList<int> sockets;
    dbManager.clientsToSync(username, sockets);
    qDebug() << "Will notify clients of user" << username;

    foreach (int socketId, sockets)
    {
        if (socketId == -1)
            continue;
        QTcpSocket *client = clientConnections.at(socketId);
        QString peerHostname = peerNames.at(socketId);
        notifyClient(client, peerHostname);
    }
}

// incapsulate notification message
void EasysyncServer::notifyClient(QTcpSocket *socket, const QString &hostname)
{
    qDebug() << hostname << "must sync!";
    sendToClient(socket, QString("2. <%1>, sync is needed.\n").arg(hostname));
}

void EasysyncServer::pause()
{
    disabled = true;
}

void EasysyncServer::resume()
{
    disabled = false;
}

void EasysyncServer::stop()
{
    disabled = true;
    close(); // close all connections
}
