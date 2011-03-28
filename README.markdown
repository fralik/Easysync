Easysync
========

A client-server Qt-based application that provides automated file synchronisation. The synchronisation is done based on [Unison](http://www.cis.upenn.edu/~bcpierce/unison/).

Description
-----------

We all know Dropbox and how it changed our lives. There's also plenty of other similar services and you often tend to use more than just one. While the individual pricing plans are affordable, it can get pretty upsetting when all the bills arrive in the end of the month. So if one already owns a personal server why not to use it?

I was recently playing around with Unison and its command line interface and decided to write a wrapper around these commands. Easysync is what I came up with. 

The server part of Easysync handles client connections and propagates sync signals among them. Multi-user setup is possible, though it hasn't been tested. The authentication is performed using only the username.  Each user may have zero or more associated clients. A client is a computer running the client part of Easysync. A client is identified by its hostname. Server stores information about users and clients in an SQLITE database. The first user is added to the database during the server installation process. A client is registered automatically if it has successfully connected to the server and passed the authentication (username comparison in our case). *The server part is designed for Debian-based Linux operating systems. Note that the path to the database is hard-coded.*

The client part handles server connection and synchronisation process. Synchronisation process is based solely on Unison and it is no more than execution of a command "unison <profile_name>", where <profile_name> is the name of a Unison profile. This means that before running the client, user should properly setup Unison and test that it runs smoothly (this part makes Easysync really not so easy, but this might change in future releases).

**Please note, that current version was done just to prove the concept. There's still room for improvement.**

Client side
-----------

The client side is a GUI-based piece of software with Windows and Linux versions, though the realisation is a little different between the two OSs. As a programmer I would like system API (or framework API) to handle much of the work. In this particular case, I would like to provide only the path of watched directory and be notified about changes in the tree of this directory. Unfortunately this is not how Qt works. Qt uses QFileSystemWatcher class that is not able to track changes in subfolders. However, there's a way around it in Windows: a function called [FindFirstChangeNotification](http://msdn.microsoft.com/en-us/library/aa364417.aspx) can monitor directory trees. Easysync client contains modified version of QFileSystemWatcher, called MFileSystemWatcher. All the related files are found in *qt* folder. The changes to QFileSystemWatcher are done not only to receive signals about changes in subfolders, but also to track changes of files inside the directory tree.

In Linux Qt uses Inotify or Dnotify to watch directories. Inotify and Dnotify don't support subfolder tracking nor is there a function similar to FindFirstChangeNotification in Linux. Easysync finds all subfolders of a given folder and adds watchers to these folders.

Installation
============

General instructions:
* Get the source code
    git clone git@github.com:fralik/Easysync.git
* Navigate to the desired directory (server or client).
* Read installation instruction in appropriate INSTALL file.

Precompiled executable is available for Windows users. Please, refer to *client/INSTALL.win.markdown* file.

Known Issues
============

* There is a SIGSEGV report related to Easysync in syslog. However, I can not see how it affects the application. Need to find the cause of the problem.
 
* Client does not *remember* that it should perform synchronisation. So if you modify something in the watched directory while there is no connection to the server and quit the application afterwards, the client will not perform the synchronization on the next successful connection to the server.

* Mac OS X support. It is possible to build the client in OS X, however I had no luck to install command line Unison 2.40.61. Synchronisation using GUI failed for some reason. Probably becuase of the file permissions.

* Deeper integration with Unison should be done for better user experience.

Found a bug? [Report it!](https://github.com/fralik/Easysync/issues)

Have a suggestion or critical comment? Write me at fralik@gmail.com!

Acknowledgement
===============

* I am using icons from [Tango](http://tango.freedesktop.org/Tango_Desktop_Project) collection. Some of them are slightly modified, but nevertheless based on Tango.

* Thanks to Gregory Konradt (g.konradt@gmail.com) for his support and help.

License
=======
Easysync is licensed under GNU General Public License version 3.0 as published by the Free Software Foundation and appearing in the file LICENSE included in the packaging of this software.

Copyright (c) 2011 Vadim Frolov
