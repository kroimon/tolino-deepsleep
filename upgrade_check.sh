#!/system/bin/sh
i=0

if [ -e /data/no_sd_upgrade ] ; then
  rm /data/no_sd_upgrade
fi

upgrade_check(){
sleep 3
if [ -e /mnt/extsd/update.zip ] ; then
		echo "---> recovery mode setting ---------------------------------------"
		rm /cache/downloadfile*
		sync
		sync
		if [ -e /mnt/extsd/update_mark ] ; then
				rm /mnt/extsd/update.zip
				rm /mnt/extsd/update_mark
				upgrade_check
		fi
elif [ -e /mnt/sdcard/update.zip ] ; then
		echo "---> recovery mode setting ---------------------------------------"
		rm  /cache/downloadfile*
		sync
		sync
		rm  /mnt/sdcard/update.zip
		upgrade_check
elif [ -e /mnt/sdcard/show_info ] || [ -e /mnt/extsd/show_info ]; then
		echo "---> Programming show info. ----------------------------------"
		sync
		sync
		am start -n com.ntx.msg/.MsgShowInfoActivity
		sync
		sync
else
		if [ "$i" -le 5 ]; then
			i=$((i + 1))
			upgrade_check
		else
			busybox touch /data/no_sd_upgrade
		fi
fi
}

setprop persist.service.adb.enable 1

upgrade_check

pidof deepsleep
if [ $? != 0 ]
then
  nohup deepsleep > /dev/null &
fi
