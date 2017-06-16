
insmod ./device_drivers/cdc-acm.ko
wait
insmod ./device_drivers/dipswitch_driver.ko
wait
insmod ./device_drivers/bcmdhd.ko
wait

wpa_supplicant -Dwext -iwlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf -B
wait
wpa_cli select_network 0
wait
ifconfig wlan0 192.168.10.27
wait
route add default gw 192.168.10.1
wait
ifconfig eth0 192.168.0.2
wait
