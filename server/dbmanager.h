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

    //! Return status of the connection to the database
    bool isConnected();
    bool createTables();
    bool addUser(const QString& username);
    bool isUserValid(const QString& username);
    bool addHostname(const QString& username, const QString& hostname, int socket_id);
    void voidConnection(const QString& hostname);
    bool isSyncNeeded(const QString& hostname);
    void setNotifyAllExcept(const QString& username, const QString& hostname);
    void clientsToSync(const QString& username, QList<int>& sockets);
    void syncIsDone(const QString& hostname);
    //! Initialize path to the database.
    /*!
      There are two options to specify path to the database. First one is to
      use an INI file with key dbPath. If this options is used, then you should
      provide path to the INI file in @param config_file. The second options is
      used if @param config_file is empty. Then the location for database is set
      to $HOME/.easyssync/easysync.sqlite
      \param config_file Path to the INI file with configuration parameters.
    */
    void initDbPath(QString config_file = "");

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
    QString dbPath;
    QString host;
    QString connectionName;
    QSqlDatabase db;
};

#endif // DBMANAGER_H
