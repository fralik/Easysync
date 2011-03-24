#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QList>

class DbManager
{
public:
    DbManager();
    ~DbManager();
    bool connect();
    void disconnect();

    bool createTables();
    bool addUser(const QString& username);
    bool isUserValid(const QString& username);
    bool addHostname(const QString& username, const QString& hostname, int socket_id);
    void voidConnection(const QString& hostname);
    bool isSyncNeeded(const QString& hostname);
    void setNotifyAllExcept(const QString& username, const QString& hostname);
    void clientsToSync(const QString& username, QList<int>& sockets);
    void syncIsDone(const QString& hostname);

private:
    // use this function to escape data before SELECT query
    QString sqlQuote(const QString& data)
    {
        QString text = data;
        text.replace("\"","\\\""); /* 228 ``*/
        text.replace("'","`");
        return text;
    }

    QString sqlDeQuote(const QString& data)
    {
        QString text = data;
        text.replace("\\\"","\""); /* 228 ``*/
        text.replace("`","\'");
        return text;
    }

    // DB parameters should be in INI file
    QString dbUser;
    QString dbPasswd;
    QString driver;
    QString dbName;
    QString host;
    QString connectionName;
    QSqlDatabase db;
};

#endif // DBMANAGER_H
