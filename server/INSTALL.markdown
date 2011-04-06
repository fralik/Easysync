    qmake ./easysync-server.pro -r -spec linux-g++
    make
    chmod +x setup.sh
    sudo ./setup.sh
Server is installed in */usr/local/bin*, database can be found in */usr/local/share/easysync*. It is also possible to start/stop server through */etc/init.d/easysync-server*. By default, server uses configuration file *config.ini.sample*, which is copied to */usr/local/share/easysync/config.ini*. Review it for default values.

To uninstall Easysync-server run
    sudo ./setup.sh uninstall
