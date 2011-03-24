#include "easysyncservice.h"

int main(int argc, char *argv[])
{

    EasysyncService service(argc, argv);

    return service.exec();
}
