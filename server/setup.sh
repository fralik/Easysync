#!/bin/bash
# Name : setup.sh
# Author : Vadim Frolov <fralik@gmail.com>
# Site : 
# Desc : This script sets up the easysync server

clear
stty erase '^?'
echo "Easysync server setup script"


###############################################################################
# Check if the user is root
###############################################################################
echo -n "* Checking that user is root..."
if [ "$(id -u)" != "0" ]; then 
	echo; echo "	ERROR: User $(whoami) is not root, and does not have sudo privileges" ; exit 1;
else
	echo "ok"
fi
###############################################################################


###############################################################################
# Checking if this system is either Debian or Ubuntu
###############################################################################
echo -n "* Checking if the installer supports this system..."
if [ `cat /etc/issue.net | cut -d' ' -f1` == "Debian" ] || [ `cat /etc/issue.net | cut -d' ' -f1` == "Ubuntu" ];then
	echo "ok"
else
	echo; echo "	ERROR: this installer currently does not support your system,"
	echo       "	but you can try, it could work (tm) - let us know if it does"
fi
###############################################################################

###############################################################################
# Define functions
###############################################################################
questions(){
	echo -n "	- username (will be able to use easysync): "
    	read username
}

install(){
########################
	echo "* Installing easysync server..."
	echo "	> installing /usr/local/bin/easysync-server..."
	cp build/easysync-server /usr/local/bin/; chown root:root /usr/local/bin/easysync-server; chmod 755 /usr/local/bin/easysync-server
	/usr/local/bin/easysync-server -i
	echo "	done"
########################
	echo -n "	> installing /etc/init.d/easysync-server..."
	cp etc/init.d/easysync-server /etc/init.d/easysync-server; chown root:root /etc/init.d/easysync-server; chmod 755 /etc/init.d/easysync-server
	echo "done"
########################
	echo -n "	> setting easysync to run during the system startup..."
	sudo update-rc.d easysync-server defaults
	echo "done"
########################
    echo -n "   > prepare config file..."
    mkdir /usr/local/share/easysync; chown root:root /usr/local/share/easysync; chmod 755 /usr/local/share/easysync/;
    sudo cp config.ini.sample /usr/local/share/easysync/config.ini
    echo "done"
    ########################
}

uninstall(){
	#echo -n "	NOTICE: stopping easysync-server service..."
	su root -c "/etc/init.d/easysync-server stop >> /dev/null"
	su root -c "/usr/local/bin/easysync-server -u"
	sudo update-rc.d -f easysync-server remove
	echo "done"

	echo -n " 	NOTICE: removing easysync-server files..."
	rm -rf /etc/init.d/easysync-server
    rm -rf /usr/local/bin/easysync-s*
	rm -rf /usr/local/share/easysync/
	echo "done"
}

adduser(){
	echo "* Adding user ${username}"
    su root -c "/usr/local/bin/easysync-server -e --adduser ${username} --config /usr/local/share/config.ini"
	echo "done"
}
###############################################################################

###############################################################################
# Install or uninstall the lipsync service
###############################################################################
if [ "${1}" = "uninstall" ]; then
	echo "	ALERT: Uninstall option chosen, all easysync-server files and configuration will be purged!"
	echo -n "	ALERT: To continue press enter to continue, otherwise hit ctrl-c now to bail..."
	read continue
	uninstall
	exit 0
else
	questions
	install
	adduser
fi
###############################################################################

###############################################################################
# Startup easysync-server and exit
###############################################################################
echo "easysync-server setup complete, staring easysync-server..."
su root -c "/etc/init.d/easysync-server start"
if [ -f /var/run/easysync-server.pid ]; then
	echo "	NOTICE: easysync-server is running as pid `cat /var/run/easysync-server.pid`"
	echo "	Check /var/log/syslog for details"
else
	echo "	NOTICE: easysync-server failed to start..."
	echo "	Check /var/log/syslog for details"
fi
###############################################################################

exit 0;
