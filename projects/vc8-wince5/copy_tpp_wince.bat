@echo off

REM $Id$

REM Ambulant home directory
set AMB_HOME=..\..

REM Third Party Packages home directory
set TPP_HOME=%AMB_HOME%\third_party_packages

set WCE_ARM_LIB=%AMB_HOME%\lib\wince-arm
set WCE_ARM_BIN=%AMB_HOME%\bin\wince

REM Create required directories
if not exist %AMB_HOME%\lib md %AMB_HOME%\lib
if not exist %AMB_HOME%\lib\wince-arm md %AMB_HOME%\lib\wince-arm
if not exist %AMB_HOME%\bin md %AMB_HOME%\bin
if not exist %AMB_HOME%\bin\wince md %AMB_HOME%\bin\wince

REM Expat static library
copy %TPP_HOME%\"expat\lib\Windows Mobile 5.0 Pocket PC SDK (ARMV4I)\Release"\libexpat.lib %WCE_ARM_LIB%\libexpat.lib

REM PNG static library
copy %TPP_HOME%\"lpng128\projects\emvc3\Windows Mobile 5.0 Pocket PC SDK (ARMV4I)"\Release\libpng.lib %WCE_ARM_LIB%\libpng.lib

REM ZLib static library
copy %TPP_HOME%\"lpng128\projects\emvc3\zlib\Windows Mobile 5.0 Pocket PC SDK (ARMV4I)\Release"\zlib.lib %WCE_ARM_LIB%\zlibce.lib

REM JPEG static library
copy %TPP_HOME%\"jpeg\wince\libjpeg\Windows Mobile 5.0 Pocket PC SDK (ARMV4I)"\Release\libjpeg.lib %WCE_ARM_LIB%\libjpeg.lib

REM MP3LIB static library
copy %TPP_HOME%\"mp3lib\Windows Mobile 5.0 Pocket PC SDK (ARMV4I)\Release"\mp3lib.lib %WCE_ARM_LIB%\mp3lib.lib

@echo on