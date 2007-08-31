@echo on
REM $Id$

REM Ambulant home directory
set AMB_HOME=..\..

REM Third Party Packages home directory
set TPP_HOME=%AMB_HOME%\third_party_packages

REM Create required directories
if not exist %AMB_HOME%\lib md %AMB_HOME%\lib
if not exist %AMB_HOME%\lib\win32 md %AMB_HOME%\lib\win32
if not exist %AMB_HOME%\bin md %AMB_HOME%\bin
if not exist %AMB_HOME%\bin\win32 md %AMB_HOME%\bin\win32

REM Expat
REM skip: copy %TPP_HOME%\expat\lib\Release\libexpat.dll %AMB_HOME%\bin\win32\libexpat.dll
copy %TPP_HOME%\expat\lib\Release\libexpat.lib %AMB_HOME%\lib\win32\libexpat.lib

REM PNG DLL
REM skip: copy %TPP_HOME%\lpng128\projects\visualc71\XXXXXX\libpng.dll %AMB_HOME%\bin\win32\libpng.dll
copy %TPP_HOME%\lpng128\projects\visualc71\Win32_LIB_ASM_Release\libpng.lib %AMB_HOME%\lib\win32\libpng.lib

REM ZLib DLL
REM skip: copy %TPP_HOME%\lpng128\projects\visualc71\XXXXXX\zlib\zlib.dll %AMB_HOME%\bin\win32\zlib.dll
copy %TPP_HOME%\lpng128\projects\visualc71\Win32_LIB_Release\zlib\zlib.lib %AMB_HOME%\lib\win32\zlib.lib

REM JPEG static library
copy %TPP_HOME%\jpeg\win32\Release\libjpeg.lib %AMB_HOME%\lib\win32\libjpeg.lib

REM Xerces lib, if it exists
set XER_BUILD=%TPP_HOME%\xerces-c-src_2_7_0\Build\Win32\VC8
if exist %XER_BUILD% copy %XER_BUILD%\Release\xerces-c_2.lib %AMB_HOME%\lib\win32\xerces-c_2.lib
if exist %XER_BUILD% copy %XER_BUILD%\Release\xerces-c_2_7.dll %AMB_HOME%\bin\win32\xerces-c_2_7.dll
if exist %XER_BUILD% copy %XER_BUILD%\Debug\xerces-c_2D.lib %AMB_HOME%\lib\win32\xerces-c_2D.lib
if exist %XER_BUILD% copy %XER_BUILD%\Debug\xerces-c_2_7D.dll %AMB_HOME%\bin\win32\xerces-c_2_7D.dll

REM ffmpeg
copy %TPP_HOME%\ffmpeg\libavcodec\avcodec-51.dll %AMB_HOME%\bin\win32\avcodec-51.dll
copy %TPP_HOME%\ffmpeg\libavcodec\avcodec-51.lib %AMB_HOME%\lib\win32\avcodec.lib
copy %TPP_HOME%\ffmpeg\libavformat\avformat-51.dll %AMB_HOME%\bin\win32\avformat-51.dll
copy %TPP_HOME%\ffmpeg\libavformat\avformat-51.lib %AMB_HOME%\lib\win32\avformat.lib
copy %TPP_HOME%\ffmpeg\libavutil\avutil-49.dll %AMB_HOME%\bin\win32\avutil-49.dll
copy %TPP_HOME%\ffmpeg\libavutil\avutil-49.lib %AMB_HOME%\lib\win32\avutil.lib

REM sdl
copy %TPP_HOME%\SDL-1.2.12\lib\SDL.dll %AMB_HOME%\bin\win32\SDL.dll
copy %TPP_HOME%\SDL-1.2.12\lib\SDL.lib %AMB_HOME%\lib\win32\SDL.lib
copy %TPP_HOME%\SDL-1.2.12\lib\SDLmain.lib %AMB_HOME%\lib\win32\SDLmain.lib

REM Live555
copy %TPP_HOME%\live_VC8\BUILD\BasicUsageEnvironment-Release\BasicUsageEnvironment.lib %AMB_HOME%\lib\win32\BasicUsageEnvironment.lib
copy %TPP_HOME%\live_VC8\BUILD\groupsock\Release-groupsock.lib %AMB_HOME%\lib\win32\groupsock.lib
copy %TPP_HOME%\live_VC8\BUILD\liveMedia\Release-liveMedia.lib %AMB_HOME%\lib\win32\liveMedia.lib
copy %TPP_HOME%\live_VC8\BUILD\UsageEnvironment-Release\UsageEnvironment.lib %AMB_HOME%\lib\win32\UsageEnvironment.lib

REM copy %TPP_HOME%\live_VC8\BUILD\BasicUsageEnvironment-Debug\BasicUsageEnvironmentD.lib %AMB_HOME%\lib\win32\BasicUsageEnvironmentD.lib
REM copy %TPP_HOME%\live_VC8\BUILD\groupsock-Debug\groupsockD.lib %AMB_HOME%\lib\win32\groupsockD.lib
REM copy %TPP_HOME%\live_VC8\BUILD\liveMedia-Debug\liveMediaD.lib %AMB_HOME%\lib\win32\liveMediaD.lib
REM copy %TPP_HOME%\live_VC8\BUILD\UsageEnvironment-Debug\UsageEnvironmentD.lib %AMB_HOME%\lib\win32\UsageEnvironmentD.lib
@echo on