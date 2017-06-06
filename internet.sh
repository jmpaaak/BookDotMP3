#!/bin/sh	
insmod bcmdhd.ko
sleep 2;
wpa_supplicant -Dwext -iwlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf -B
sleep 2;
wpa_cli select_network 1
sleep 2;
ifconfig wlan0 192.168.10.30
sleep 2;
rout add default gw 192.168.10.1
