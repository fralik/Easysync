#!/opt/bin/bash
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
# Check if unison is installed
###############################################################################
echo -n "* Checking that unison is installed..."
if [ ! -f /opt/bin/unison ]; then 
	echo; echo "	ERROR: Unison is not installed. Install Unison in /opt/bin/" ; exit 1;
else
	echo "ok"
fi


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
	echo "	> installing /opt/bin/easysync-server..."
	cp easysync-server /opt/bin/; chown root:root /opt/bin/easysync-server; chmod 755 /opt/bin/easysync-server
	/opt/bin/easysync-server -i
	echo "	done"
########################
	echo -n "	> installing /opt/etc/init.d/easysync-server..."
	cp etc/init.d/easysync-server-mips /opt/etc/init.d/easysync-server; chown root:root /opt/etc/init.d/easysync-server; chmod 755 /opt/etc/init.d/easysync-server
	echo "done"
########################
	#echo -n "	> setting easysync to run during the system startup..."
	#sudo update-rc.d easysync-server defaults
	#echo "done"
########################
	echo -n "   > prepare config file..."
	mkdir /opt/etc/easysync-server; chown root:root /opt/etc/easysync-server; chmod 755 /opt/etc/easysync-server;
	sudo cp config.ini.sample /opt/etc/easysync-server/config.ini
	echo "done"
########################
}

uninstall(){
	#echo -n "	NOTICE: stopping easysync-server service..."
	su root -c "/opt/etc/init.d/easysync-server stop >> /dev/null"
	su root -c "/opt/bin/easysync-server -u"
	#sudo update-rc.d -f easysync-server remove
	echo "done"

	echo -n " 	NOTICE: removing easysync-server files..."
	rm -rf /opt/etc/init.d/easysync-server
	rm -rf /opt/bin/easysync-s*
	rm -rf /opt/etc/easysync-server/
	echo "done"
}

adduser(){
	echo "* Adding user ${username}"
	su root -c "/opt/bin/easysync-server -e --adduser ${username} --config /opt/etc/easysync-server/config.ini"
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
su root -c "/opt/etc/init.d/easysync-server start"
if [ -f /tmp/var/run/easysync-server.pid ]; then
	echo "	NOTICE: easysync-server is running as pid `cat /tmp/var/run/easysync-server.pid`"
	echo "	Check /tmp/var/log/messages for details"
else
	echo "	NOTICE: easysync-server failed to start..."
	echo "	Check /tmp/var/log/messages for details"
fi
###############################################################################

exit 0;
