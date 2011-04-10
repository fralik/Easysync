#!/bin/bash
# Name : build_package_dd-wrt.sh
# Author : Vadim Frolov <fralik@gmail.com>
# Site : 
# Desc : Builds an installation package for DD-WRT
echo -n "* Checking that easysnc is built..."
if [ ! -f ./build/easysync-server ]; then 
	echo; echo "	ERROR: Easysync-server is not built or not in the ./build/ directory" ; exit 1;
else
	echo "ok"
fi

cp ./build/easysync-server ./easysync-server
tar -czf dd-wrt_bundle.tar.gz easysync-server config.ini.sample etc/init.d/easysync-server-mips setup_dd-wrt.sh
rm -f ./easysync-server

echo "* Done!"
echo "Copy dd-wrt_bundle.tar.gz to your device, unpack, edit config.ini.sample, run setup_dd-wrt.sh, enjoy!"
exit 0;
