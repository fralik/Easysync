The client is provided as a precompiled executable. You can download it from the [downloads section](https://github.com/fralik/Easysync/archives/master). There are two packages: one contains the Easysync executable and the other necessary dependences. Extract [*Qt_4.7.2_win32.zip*](https://github.com/downloads/fralik/Easysync/Qt_4.7.2_win32.zip) in the same folder with Easysync.

The client invokes Unison as an external process, this means that Unison should be available from your PATH. It is invoked as 'unison <profile_name>', so before running the client, make sure that you are able to flawlessly run Unison with desired profile. This document covers the usage of Unison with Putty.

* Setup Unison. I recommend to use [precompiled binaries](http://alan.petitepomme.net/unison/index.html) provided by Alan Schmitt. Note, that you need the same version of Unison on all of your computers: server and clients. To run Unison, you will need to download and install Microsoft Visual C++ 2008 SP1 Redistributable Package (use [vcredist_x86.exe](http://www.microsoft.com/downloads/details.aspx?familyid=A5C84275-3B97-4AB7-A40D-3802B2AF5FC2) on a 32-bit x86 architecture and [vcredist_x64.exe](http://www.microsoft.com/downloads/en/details.aspx?familyid=BD2A6171-E2D6-4230-B809-9A8D7548C1B6) on the IA64 or 64-bit x86 architecture).

* Martin Cleaver provides [detailed instructions](http://twiki.org/cgi-bin/view/Codev/UnisonKeySetup) about how to setup Unison with Putty. Use the ssh2plink.bat script from his page.

* Create a Unison profile. Skeleton profile is provided in file *PROFILE.win*. Change it to suit your needs. The profile should be in [*.unison*](http://www.seas.upenn.edu/~bcpierce/unison//download/releases/stable/unison-manual.html#unisondir) folder. On my computer this is *C:\Documents and settings\vadim\.unison*.

* Run Pageant and make sure that your SSH key is loaded.

* Run Unison to check your settings. The best solution will be to try Unison from the command prompt, but you can use Unison GUI as well. Just be sure that Unison perfroms synchronisation without any errors or prompts.

* Run easysync-client.exe. Alternative solution is to use easysync-client.cmd batch script. This script loads Pageant and then loads Easysync-client.exe. Just remember to edit it, so it points to your private key.
