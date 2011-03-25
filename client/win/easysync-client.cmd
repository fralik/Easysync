@echo off
rem	Starts pageant before starting Easysync

rem Edit it according to your setup
set PAGEANTEXE=D:\putty\pageant.exe
set KEY1="D:\secret_folder\private.ppk"

if exist %KEY1% (set KEYS=%KEY1%)

if not defined KEYS exit

start %PAGEANTEXE% %KEYS%
start easysync-client.exe