#ifndef CLIENT_H
#define CLIENT_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QTcpSocket>
#include <QTimer>
#include <QProcess>
#include <QMutex>

#if defined(Q_OS_WIN)
    #include "qt/mfilesystemwatcher.h"
#else
    #include <QFileSystemWatcher>
#endif

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QTcpSocket;
class QTimer;
class QNetworkConfigurationManager;
class QProcess;
class QMutex;
QT_END_NAMESPACE

namespace Easysync
{
    enum AvailableIcons {SyncInProgressIcon, NoConnectionIcon, ConnectedIcon};
    /* this enumeration is used in call of startSynchronisation function */
    enum SyncNotifications { NotifyAll = true, NotifyNone = false};
};

namespace Ui {
    class Client;
}

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

private slots:
    void connectToServer();
    void checkServerConnection();
    void handleSocketConnected();
    void handleSocketDisconnected();
    void readFromServer();
    void handleSyncTimer();
    /* sends "pingpong" message to the server */
    void sendKeepAlive();
    void handleSyncIsFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void handleSaveButton();
    void selectDirectory();
    void handleDirectoryChanged(const QString &path);
    void onlineStateChanged(bool isOnline);
    void showAbout();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void setIcon(Easysync::AvailableIcons icon);

private:
    void checkSettings();
    /* creates action for the systray menu */
    void createActions();
    void createTrayIcon();
    void writeSettings();

    /* sends initial message to the server */
    void sendGreetings();
    void startSynchronisation(bool notifyAll = Easysync::NotifyAll);

    void installWatcher();
    void stopWatcher();

    /* returns list of all subfolders of a given folder */
    QStringList subFolders(const QString &folder);

    Ui::Client *ui;

    QAction *preferencesAction;
    QAction *quitAction;
    QAction *aboutAction;

    QTimer *syncTimer;
    QTimer *serverKeepAliveTimer;
    int keepAliveInterval;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
#if defined(Q_OS_WIN)
    MFileSystemWatcher watcher;
#else
    QFileSystemWatcher watcher;
#endif

    QString username;
    QString hostname;
    QString syncDir;
    QString serverAddress;
    QString port;
    
    bool notifyAllAfterSync;
    QProcess *unisonProcess;

    QMutex mutex;

    QNetworkConfigurationManager *networkManager;
    QTcpSocket *server;
    bool socketReady;
};

#endif // CLIENT_H
