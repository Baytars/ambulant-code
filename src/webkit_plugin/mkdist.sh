#!/bin/sh
if [ x$1 = x ]; then
	echo Usage: $0 distname
	echo distname will be something like Ambulant-2.0-webkitplugin.
	exit
fi
rm -rf $1
mkdir $1
cp -R ~/Library/Internet\ Plug-Ins/AmbulantWebKitPlugin.plugin $1
cp README $1
zip -r $1.zip $1
