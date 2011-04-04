#include "dbmanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QtSql>
#include <QtServiceBase>
#include <QDir>

DbManager::DbManager()
{
    dbUser = ""; /*!< Not used in SQLITE */
    dbPasswd = ""; /*!< Not used in SQLITE */
    driver = "QSQLITE";
    dbPath = "";
    host = "localhost";
    connectionName = "easysync-connection";
}

DbManager::~DbManager()
{
    disconnect();
}

bool DbManager::isConnected()
{
    return db.isOpen();
}

bool DbManager::connect()
{
    QSqlError err;

    db = QSqlDatabase::addDatabase(driver, connectionName);
    db.setDatabaseName(dbPath);
    db.setHostName(host);
    db.setPort(-1);
    //qDebug() << "Trying to open DB" << dbPath;
    if (!db.open(dbUser, dbPasswd))
    {
        err = db.lastError();
        disconnect();
        QtServiceBase::instance()->logMessage(QString("Failed to open DB, error: %1").arg(err.text()));
        qDebug() << QString("Failed to open DB, error: %1").arg(err.text());

        return false;
    }
    return true;
}


void DbManager::disconnect()
{
    if (db.isOpen())
        db.close();

    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

// create table users (id integer unique primary key,username varchar(255));
// create table hosts (id integer unique primary key, user_id integer not null, must_sync numeric default 0, hostname varchar(255) not null, foreign key(user_id) references users(id) ON DELETE CASCADE );
bool DbManager::createTables()
{
    if (!db.isOpen())
        return false;

    QSqlQuery query(db);

    // check that table exists
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='users';"))
    {
        QSqlError err = query.lastError();
        QtServiceBase::instance()->logMessage(QString("Failed to open perform query, error: %1").arg(err.text()));
        qDebug() << "Failed to perfrom query in createTables";

        disconnect();
        return false;
    }
    if (query.size() < 1)
    {
        // table to store users
        query.exec("create table users ("
                "id integer unique primary key,"
                "username varchar(255)"
                ");");

        // table to store user's hotnames
        query.exec("drop table hosts;");
        query.exec("create table hosts ("
                "id integer unique primary key,"
                "user_id integer not null,"
                "must_sync numeric default 0,"
                "hostname varchar(255) not null,"
                "socket_id integer default -1,"
                "foreign key(user_id) references users(id) ON DELETE CASCADE"
                ");");
    }

    return true;
}

void DbManager::voidConnection(const QString &hostname)
{
    if (!db.isOpen())
        return;

    QSqlQuery query(db);
    query.prepare("UPDATE hosts SET socket_id = -1 WHERE hostname = :hostname;");
    query.bindValue(":hostname", hostname);
    query.exec();
}

/*
    sets 'must_sync' to 1 for all hostnames, except given hostname
*/
void DbManager::setNotifyAllExcept(const QString& username, const QString& hostname)
{
    if (!db.isOpen())
        return;

    QSqlQuery query(db);
    QString safeUsername = sqlQuote(username);
    QString sQuery = QString(" AND user_id = (SELECT id from users WHERE username = '%1');").arg(safeUsername);

    QString completeQuery("UPDATE hosts SET must_sync = 1 WHERE hostname != :hostname");
    completeQuery += sQuery;

    query.prepare(completeQuery);
    query.bindValue(":hostname", hostname);
    query.exec();
}

bool DbManager::addUser(const QString& username)
{
    if (!db.isOpen())
        return false;

    if (isUserValid(username))
        return true;

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (id, username) VALUES(null, :username);");
    query.bindValue(":username", username);
    query.exec();
    qDebug() << "Added user" << username << "to the DB";
    return true;
}

bool DbManager::isUserValid(const QString& username)
{
    if (!db.isOpen())
        return false;

    QString safeUsername = sqlQuote(username);
    QSqlQuery query(db);
    QString sQuery(QString("SELECT username FROM users WHERE username='%1';").arg(safeUsername));
    if (!query.exec(sQuery))
    {
        qDebug() << "failed to perfrom query isUserValid";
        return false;
    }

    if (query.next())
    {
        QString dbUsername = query.value(0).toString();
        if (dbUsername == username)
        {
            qDebug() << "User" << username << "is valid";
            return true;
        }
    }
    qDebug() << "User" << username << "is NOT valid";
    return false;
}

void DbManager::clientsToSync(const QString &username, QList<int> &sockets)
{
    if (!db.isOpen())
        return ;

    QString safeUsername = sqlQuote(username);
    QSqlQuery query(db);

    query.exec(QString("SELECT socket_id, hostname FROM hosts, users WHERE must_sync = '1' "
        "AND socket_id != '-1' AND users.id = (SELECT id FROM users WHERE username = '%1');").arg(safeUsername));

    sockets.clear();
    while (query.next())
    {
        int socket_id = query.value(0).toInt();
        sockets.append(socket_id);
    }
}

void DbManager::syncIsDone(const QString &hostname)
{
    if (!db.isOpen())
        return ;

    QSqlQuery query(db);

    query.prepare("UPDATE hosts SET must_sync = 0 WHERE hostname = :hostname;");
    query.bindValue(":hostname", hostname);
    query.exec();
    qDebug() << "Set must_sync = 0 for hostname" << hostname;
}

bool DbManager::isSyncNeeded(const QString& hostname)
{
    if (!db.isOpen())
        return false;

    QString safeHostname = sqlQuote(hostname);
    QSqlQuery query(db);
    query.exec(QString("SELECT must_sync FROM hosts WHERE hostname = '%1' AND must_sync = '1'").arg(safeHostname));
    query.exec();
    if (query.next())
        return true;

    return false;
}

bool DbManager::addHostname(const QString& username, const QString& hostname, int socket_id)
{
    if (!db.isOpen())
        return false;

    QString safeUsername = sqlQuote(username);
    QString safeHostname = sqlQuote(hostname);
    QSqlQuery query(db);
    query.exec(QString("SELECT users.username FROM users, hosts WHERE username= '%1' AND hostname = '%2'").arg(
        safeUsername).arg(safeHostname));

    if (!query.exec())
    {
        qDebug() << "failed to perfrom query addHostname";
        return false;
    }

    if (!query.next())
    {
        query.prepare("INSERT INTO hosts (id, user_id, hostname, must_sync) VALUES(null,"
            "(SELECT id FROM users WHERE username = :username), :hostname, 1);");
        query.bindValue(":username", username);
        query.bindValue(":hostname", hostname);
        if (!query.exec())
        {
            qDebug() << "failed to insert hostname" << hostname << "for user" << username;
            return false;
        }
        qDebug() << "Added hostname" << hostname;
    }

    query.prepare("UPDATE hosts SET socket_id = :socket_id WHERE hostname = :hostname;");
    query.bindValue(":socket_id", socket_id);
    query.bindValue(":hostname", hostname);
    query.exec();
    qDebug() << "Updated hostname" << hostname;

    return true;
}


void DbManager::initDbPath(QString config_file)
{
    QDir dir;
    QString settings_dir = "";

    QFileInfo config_info(config_file);

    //qDebug() << "Got" << config_info.absoluteFilePath();

    if (!config_file.isEmpty() && config_info.exists())
    {
        QSettings settings(config_info.absoluteFilePath(), QSettings::IniFormat);
        dbPath = settings.value("dbPath").toString();
        if (dbPath.isEmpty() || dbPath.isNull())
        {
            QtServiceBase::instance()->logMessage(QString("Failed to read dbPath from config file"));
            settings_dir = QDir::homePath() + QDir::separator() + ".easysync";
            dbPath = settings_dir + QDir::separator() + "easysync.sqlite";
        }
        else
        {
            settings_dir = QFileInfo(dbPath).absolutePath();
        }

        //qDebug() << "creating" << settings_dir;
        dir.mkpath(settings_dir);
    }
    else
    {
        settings_dir = QDir::homePath() + QDir::separator() + ".easysync";
        dir.mkpath(settings_dir);

        dbPath = settings_dir + QDir::separator() + "easysync.sqlite";
    }
    qDebug() << "Set dbPath to" << dbPath;
}
