Easysync client
===============


Compilation. Windows
---------------------

In order to compile the source code you will need Qt sources. You need to create environment variable *QT_SOURCE*, which points to the folder with Qt sources. Example:
        QT_SOURCE=C:\QtSDK\QtSources\4.8.1

You will see error messages about compiler being not able to find *qobject_p.h* if you do not add this variable.

After this you can open the project file in Qt Creator and build it.