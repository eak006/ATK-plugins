#! /bin/sh

#shell script to automate IPlug Project build, code-signing and packaging on OSX

BASEDIR=$(dirname $0)
cd $BASEDIR

#---------------------------------------------------------------------------------------------------------

#variables

VERSION=`echo | grep PLUG_VER resource.h`
VERSION=${VERSION//\#define PLUG_VER }
VERSION=${VERSION//\'}
MAJOR_VERSION=$(($VERSION & 0xFFFF0000))
MAJOR_VERSION=$(($MAJOR_VERSION >> 16)) 
MINOR_VERSION=$(($VERSION & 0x0000FF00))
MINOR_VERSION=$(($MINOR_VERSION >> 8)) 
BUG_FIX=$(($VERSION & 0x000000FF))

FULL_VERSION=$MAJOR_VERSION"."$MINOR_VERSION"."$BUG_FIX

PLUGIN_NAME=`echo | grep BUNDLE_NAME resource.h`
PLUGIN_NAME=${PLUGIN_NAME//\#define BUNDLE_NAME }
PLUGIN_NAME=${PLUGIN_NAME//\"}

# work out the paths to the binaries

VST2=`echo | grep VST_FOLDER ../../common.xcconfig`
VST2=${VST2//\VST_FOLDER = }/$PLUGIN_NAME.vst

VST3=`echo | grep VST3_FOLDER ../../common.xcconfig`
VST3=${VST3//\VST3_FOLDER = }/$PLUGIN_NAME.vst3

AU=`echo | grep AU_FOLDER ../../common.xcconfig`
AU=${AU//\AU_FOLDER = }/$PLUGIN_NAME.component

APP=`echo | grep APP_FOLDER ../../common.xcconfig`
APP=${APP//\APP_FOLDER = }/$PLUGIN_NAME.app

PKG="installer/build-mac/$PLUGIN_NAME Installer.pkg"
PKG_US="installer/build-mac/$PLUGIN_NAME Installer.unsigned.pkg"

CERT_ID=`echo | grep CERTIFICATE_ID ../../common.xcconfig`
CERT_ID=${CERT_ID//\CERTIFICATE_ID = }

echo "making $PLUGIN_NAME version $FULL_VERSION mac distribution..."
echo ""

#---------------------------------------------------------------------------------------------------------

#call python script to update version numbers
./update_version.py

#here you can use the touch command to force xcode to rebuild
#touch MyPlugin.h

#---------------------------------------------------------------------------------------------------------

#if you are zipping the binaries, remove existing dist folder
#if [ -d installer/dist ] 
#then
#  rm -R installer/dist
#fi

#mkdir installer/dist

#remove existing binaries
if [ -d $APP ] 
then
  sudo rm -f -R -f $APP
fi

if [ -d $AU ] 
then
  sudo rm -f -R $AU
fi

if [ -d $VST2 ] 
then
  sudo rm -f -R $VST2
fi

if [ -d $VST3 ] 
then
  sudo rm -f -R $VST3
fi

#---------------------------------------------------------------------------------------------------------

# build xcode project. Change target to build individual formats 
xcodebuild -project $PLUGIN_NAME.xcodeproj -xcconfig $PLUGIN_NAME.xcconfig -target "All" -configuration Release 2> ./build-mac.log

if [ -s build-mac.log ]
then
  echo "build failed due to following errors:"
  echo ""
  cat build-mac.log
  exit 1
else
 rm build-mac.log
fi

#---------------------------------------------------------------------------------------------------------

#icon stuff - http://maxao.free.fr/telechargements/setfileicon.gz
echo "setting icons"
echo ""
./setfileicon resources/$PLUGIN_NAME.icns $AU
./setfileicon resources/$PLUGIN_NAME.icns $VST2
./setfileicon resources/$PLUGIN_NAME.icns $VST3

#---------------------------------------------------------------------------------------------------------

#strip debug symbols from binaries

echo "striping binaries"
strip -x $AU/Contents/MacOS/$PLUGIN_NAME
strip -x $VST2/Contents/MacOS/$PLUGIN_NAME
strip -x $VST3/Contents/MacOS/$PLUGIN_NAME
strip -x $APP/Contents/MacOS/$PLUGIN_NAME

#---------------------------------------------------------------------------------------------------------

#Mac AppStore stuff

#xcodebuild -project $PLUGIN_NAME.xcodeproj -xcconfig $PLUGIN_NAME.xcconfig -target "APP" -configuration Release 2> ./build-mac.log

#echo "code signing app for appstore"
#echo ""
#codesign -f -s "3rd Party Mac Developer Application: ""${CERT_ID}" $APP --entitlements resources/$PLUGIN_NAME.entitlements
 
#echo "building pkg for app store"
#echo ""
#productbuild \
#     --component $APP /Applications \
#     --sign "3rd Party Mac Developer Installer: ""${CERT_ID}" \
#     --product "/Applications/$PLUGIN_NAME.app/Contents/Info.plist" installer/$PLUGIN_NAME.pkg

#---------------------------------------------------------------------------------------------------------

#10.8 Gatekeeper/Developer ID stuff

echo "code app binary for Gatekeeper on 10.8"
echo ""
#codesign -f -s "Developer ID Application: ""${CERT_ID}" $APP

#TODO: code-sign plug-in binaries too?

#---------------------------------------------------------------------------------------------------------

# installer, uses Packages http://s.sudre.free.fr/Software/Packages/about.html
sudo rm -R -f installer/$PLUGIN_NAME-mac.dmg

echo "building installer"
echo ""
packagesbuild ./installer/$PLUGIN_NAME.pkgproj

echo "code-sign installer for Gatekeeper on 10.8"
echo ""
#mv "${PKG}" "${PKG_US}"
#productsign --sign "Developer ID Installer: ""${CERT_ID}" "${PKG_US}" "${PKG}"
                   
#rm -R -f "${PKG_US}"

#set installer icon
setfileicon resources/$PLUGIN_NAME.icns "${PKG}"

#---------------------------------------------------------------------------------------------------------

# dmg, can use dmgcanvas http://www.araelium.com/dmgcanvas/ to make a nice dmg

echo "building dmg"
echo ""

if [ -d installer/$PLUGIN_NAME.dmgCanvas ]
then
  dmgcanvas installer/$PLUGIN_NAME.dmgCanvas installer/$PLUGIN_NAME-mac.dmg
else
  hdiutil create installer/$PLUGIN_NAME.dmg -srcfolder installer/build-mac/ -ov -anyowners -volname $PLUGIN_NAME
  
  if [ -f installer/$PLUGIN_NAME-mac.dmg ]
  then
   rm -f installer/$PLUGIN_NAME-mac.dmg
  fi
  
  hdiutil convert installer/$PLUGIN_NAME.dmg -format UDZO -o installer/$PLUGIN_NAME-mac.dmg
  sudo rm -R -f installer/$PLUGIN_NAME.dmg
fi

sudo rm -R -f installer/build-mac/

#---------------------------------------------------------------------------------------------------------
# zip

# echo "copying binaries..."
# echo ""
# cp -R $AU installer/dist/$PLUGIN_NAME.component
# cp -R $VST2 installer/dist/$PLUGIN_NAME.vst
# cp -R $VST3 installer/dist/$PLUGIN_NAME.vst3
# cp -R $RTAS installer/dist/$PLUGIN_NAME.dpm
# cp -R $AAX installer/dist/$PLUGIN_NAME.aaxplugin
# cp -R $APP installer/dist/$PLUGIN_NAME.app
# 
# echo "zipping binaries..."
# echo ""
# ditto -c -k installer/dist installer/$PLUGIN_NAME-mac.zip
# rm -R installer/dist

#---------------------------------------------------------------------------------------------------------

echo "done"