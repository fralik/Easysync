    qmake ./easysync-server.pro -r -spec linux-g++
    make
    chmod +x setup.sh
    sudo ./setup.sh
Server is installed in */usr/local/bin*, database can be found in */usr/local/share/easysync*. It is also possible to start/stop server through */etc/init.d/easysync-server*. By default, server listens on port 1984.

To uninstall Easysync run
    sudo ./setup.sh uninstall
