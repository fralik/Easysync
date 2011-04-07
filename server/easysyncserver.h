#ifndef EASYSYNCSERVER_H
#define EASYSYNCSERVER_H

#include "dbmanager.h"

#include <QtNetwork/QTcpServer>
#include <QtCore/QList>
#include <QTimer>
#include <QTime>

class EasysyncServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit EasysyncServer(quint16 port, QObject *parent = 0, const QString configPath = "");
    ~EasysyncServer();

    void pause();
    void resume();
    void stop();

    DbManager dbManager;

protected Q_SLOTS:
    void handleNewConnection();
    void clientDisconnected();
    void readClient();
    void checkClientsConnection();

private:
    void sendToClient(QTcpSocket *socket, const QString &message);
    bool getUserInfo(const QString &message, QString *username, QString *hostname);
    void notifyClients(const QString &username);
    void notifyClient(QTcpSocket *socket, const QString &hostname);

    bool disabled;
    QList<QTcpSocket *> clientConnections;
    QList<QString > peerNames;
    QList<QTime > clientLastSeen;
    QTimer *keepAliveTimer;

};
#endif // EASYSYNCSERVER_H
