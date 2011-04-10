    qmake ./easysync-server.pro -r -spec linux-g++
    make
    chmod +x setup.sh
    sudo ./setup.sh
Server is installed in */usr/local/bin*, database can be found in */usr/local/share/easysync*. It is also possible to start/stop server through */etc/init.d/easysync-server*. By default, server uses configuration file *config.ini.sample*, which is copied to */usr/local/share/easysync/config.ini*. Review it for default values.

To uninstall Easysync-server run
    sudo ./setup.sh uninstall

You can install Easysync-server on your router. For an example, refer to [Wiki page](https://github.com/fralik/Easysync/wiki/Installation-on-Asus-RT-N16)
