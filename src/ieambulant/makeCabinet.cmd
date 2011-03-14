:: Script to create CAB file for IEAmbulant ActiveX plugin.
:: NOTE: Do not run this script, btu copy the contents into
:: the IEAmbulant-Installer project, "make" command lines.
::
:: This script expects to be run from within Visual Studio only,
:: because it depends on various variable names defined there.
::
:: NOTE: if you change this file you must also change the
:: AmbulantActiveX.inf file, the contents of the two files
:: MUST match.
::
:: For an example how to use the .cab, see: ieambulentWeb.htm
:: ----------------------------------------------------------------------------
:: copy all files for the cab into this directory
@echo on
@echo "copying all files for the cab into this directory"
del $(IntDir)\*.*
mkdir $(IntDir)
::
:: redistributable C-runtime
copy ^"$(WindowsSDKDir)\Bootstrapper\Packages\vcredist_x86\vcredist_x86.exe^" $(intdir)\vcredist_x86.exe
:: Ambulant dll's
copy ..\..\bin\win32\AmbulantActiveX.dll $(intdir)\AmbulantActiveX.dll
copy ..\..\bin\win32\libambulant_shwin32.dll $(intdir)\libambulant_shwin32.dll
copy ..\..\bin\win32\libamplugin_ffmpeg.dll $(intdir)\libamplugin_ffmpeg.dll 
copy ..\..\bin\win32\libamplugin_state_xpath.dll $(intdir)\libamplugin_state_xpath.dll
copy ..\..\bin\win32\avcodec-52.dll $(intdir)\avcodec-52.dll
copy ..\..\bin\win32\avformat-52.dll $(intdir)\avformat-52.dll 
copy ..\..\bin\win32\avutil-50.dll $(intdir)\avutil-50.dll
copy ..\..\bin\win32\swscale-0.dll $(intdir)\swscale-0.dll
copy ..\..\bin\win32\SDL.dll $(intdir)\SDL.dll
copy ..\..\bin\win32\xerces-c_3_1.dll $(intdir)\xerces-c_3_1.dll
copy ..\..\src\ieambulant\AmbulantActiveX.inf $(intdir)\AmbulantActiveX.inf
:: Create a new cabinet (.cab) archive
^"$(CabArc)^" -s 6144 n $(OutDir)\ieambulant.cab $(IntDir)\*.exe $(IntDir)\*.dll $(IntDir)\*.inf
:: Code sign it with code signing certificate (.pfx = Personal Information Exchange) 
set signtool=^"$(WindowsSDKDir)Bin\signtool.exe^"
%signtool% sign /f $(ieambulant_certificate) /p ambulant /v $(TargetPath)
:: timestamp the signature
%signtool% timestamp  /v /t "http://timestamp.verisign.com/scripts/timstamp.dll" $(TargetPath)
:: verify the resulting cabinet^" (.cab) archive
%signtool% verify /v /a /pa $(TargetPath)