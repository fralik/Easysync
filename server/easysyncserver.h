#ifndef EASYSYNCSERVER_H
#define EASYSYNCSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <QTime>

#include "dbmanager.h"

class EasysyncServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit EasysyncServer(quint16 port, QObject* parent = 0, QString config_file = "");
    ~EasysyncServer();

    void pause();
    void resume();
    void stop();

    DbManager dbManager;

protected slots:
    void handleNewConnection();
    void clientDisconnected();
    void readClient();
    void checkClientsConnection();

private:
    void sendToClient(QTcpSocket *socket, const QString& message);
    bool getUserInfo(const QString& message, QString *username, QString *hostname);
    void notifyClients(const QString& username);
    void notifyClient(QTcpSocket *socket, const QString& hostname);

    bool disabled;
    QList<QTcpSocket *> clientConnections;
    QList<QString > peerNames;
    QList<QTime > clientLastSeen;
    QTimer *timer;

};
#endif // EASYSYNCSERVER_H
