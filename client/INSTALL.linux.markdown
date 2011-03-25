* Setup Unison. Note that you need the same version as on the server.

* Create a Unison profile. Skeleton profile is provided in file *PROFILE.linux*. Change it to suit your needs. The profile should be in [*.unison*](http://www.seas.upenn.edu/~bcpierce/unison//download/releases/stable/unison-manual.html#unisondir) folder. On my computer this is *C:\Documents and settings\vadim\.unison*.

* Run Unison to check your settings. Type "unison <profile_name>" in the command prompt, make sure there are no error.

* Note, that you will need at least Qt version 4.7.0. This means you should either download it from Qt website or use the latest distributive version (Ubuntu 10.10 for example).

* Prepare Easysync:
    sudo apt-get install g++ qt4-dev-tools
    qmake ./easysync-client.pro -r -spec linux-g++
    make
    
* Run the client:
    cd ./build
    ./easysync-client
    