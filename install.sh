#!/bin/bash

echo build
ndk-build
if [ $? == 1 ]
then
   exit
fi

echo remount
adb remount
echo push
adb push libs/armeabi/deepsleep /system/xbin/deepsleep
echo chmod
adb shell chmod 777 /system/xbin/deepsleep
echo done
