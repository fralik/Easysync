#ifndef EASYSYNCSERVICE_H
#define EASYSYNCSERVICE_H

#include "qt/qtservice.h"
#include "easysyncserver.h"

QT_BEGIN_NAMESPACE
class QCoreApplication;
QT_END_NAMESPACE

class EasysyncService : public QtService<QCoreApplication>
{
public:
    EasysyncService(int argc, char **argv);
    virtual ~EasysyncService();

protected:
    void start();
    void stop();
    void pause();
    void resume();

private:
    EasysyncServer *server;
};

#endif // EASYSYNCSERVICE_H
