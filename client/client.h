#ifndef CLIENT_H
#define CLIENT_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QProcess>

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

private Q_SLOTS:
    void connectToServer();
    void checkServerConnection();
    void handleSocketConnected();
    void handleSocketDisconnected();
    void readFromServer();
    /*! Sends "pingpong" message to the server. */
    void sendKeepAlive();
    void onlineStateChanged(bool isOnline);

    void handleDirectoryChanged(const QString &path);
    //! Synchronisation is started after a short period of time after change event has occured.
    void handleSyncTimer();
    void handleSyncIsFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void handleSaveButton();
    void selectDirectory();
    void showAbout();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void setIcon(Easysync::AvailableIcons icon);

private:
    //! Checks settings presence in the configuration file. */
    void checkSettings();
    /*! Creates action for the systray menu. */
    void createActions();
    void createTrayIcon();
    void writeSettings();

    /*! Sends initial message to the server. */
    void sendGreetings();
    void startSynchronisation(bool notifyAll = Easysync::NotifyAll);

    void installWatcher();
    void stopWatcher();

    /*! Returns list of all subfolders of a given folder. */
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

    bool syncInProgress;
    bool needToSync;

    QNetworkConfigurationManager *networkManager;
    QTcpSocket *server;
    bool socketReady;
};

#endif // CLIENT_H
