Raspberry Pi Photobooth
=======================

This part describes the setup and contains the code to be run on the RasPi.

You will need a RasPi 3 Mod. B for this setup due to image-processing-power and
WiFi capability. 

If you use an older model, please adjust the WiFi as needed.


This code does NOT control the camera! Make sure the camera is triggered by
the Arduino code or cable release. The RasPi code only downloads images from the
camera after gphoto recognizes the corresponding event.

The code is mainly divided into three parts:

* slideshow running on port 8000 (http)
* script downloading and processing new images
* shutdown-script listening for pushbutton on IO for clean shutdown

photobooth.sh is the place to start your journey.


Install
-------

Install Raspian on your RasPi 3.
Setup your RasPi with raspi-config as needed. Set password.
Disable desktop if you want. Photobooth will run in terminal.

Enable SSH if needed:
```shell
sudo update-rc.d ssh defaults
sudo update-rc.d ssh enable
```

Install gphoto2 and imagemagick on your RasPi:
```shell
sudo apt install gphoto2 imagemagick
```

Disable auto-mount of cameras:
```shell
sudo chmod -x /usr/lib/gvfs/gvfs-gphoto2-volume-monitor
```

Download or copy the photobooth folder to /home/pi on your RasPi.


Edit .bashrc 
```shell
nano .bashrc
```

Add this to the very end:

```shell
if [ -n "$SSH_CLIENT" ] || [ -n "$SSH_TTY" ]; then
  echo "Only running on local shell."
else
  ~/photobooth/photobooth.sh
fi
```

Reboot:
```shell
sudo reboot
```


WiFi config/AP
--------------

The RasPi is configured as a WiFi AP. This way you can connect directly to the
RasPi with your devices.

You can find a great howto here:
https://frillip.com/using-your-raspberry-pi-3-as-a-wifi-access-point-with-hostapd/

Below the bare minimum follows.

```shell
sudo apt install hostapd dnsmasq
sudo nano /etc/dhcpcd.conf
```

Add to bottom of file:
```
denyinterfaces wlan0
```

Edit network config:
```shell
sudo nano /etc/network/interfaces
```

Edit wlan0 section so it becomes this:

```
allow-hotplug wlan0
iface wlan0 inet static
    address 10.0.0.1
    netmask 255.255.255.0
    network 10.0.0.0
    broadcast 10.0.0.255
#    wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf
```

Restart networking stuff

```shell
sudo service dhcpcd restart
sudo ifdown wlan0; sudo ifup wlan0
```

Edit hostapd config. 

```shell
sudo nano /etc/hostapd/hostapd.conf
```

Please adjust WiFi ssid and password to your needs.

```
ssid=Photobooth-WiFi
wpa_passphrase=Smile123456
interface=wlan0
driver=nl80211
hw_mode=g
channel=6
ieee80211n=1
wmm_enabled=1
ht_capab=[HT40][SHORT-GI-20][DSSS_CCK-40]
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
```

Test
```shell
sudo /usr/sbin/hostapd /etc/hostapd/hostapd.conf
```

You should now see Photobooth-WiFi floating around. Quit:
```shell
Ctrl-C
```

Start WiFi AP as daemon:
```shell
sudo nano /etc/default/hostapd
```

Edit config to
```
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```


Let's make sure we even get an IP address when using the WiFi:
```shell
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
sudo nano /etc/dnsmasq.conf
```

```
interface=wlan0      # Use interface wlan0
listen-address=10.0.0.1 # Explicitly specify the address to listen on
bind-interfaces      # Bind to the interface to make sure we aren't sending things elsewhere
server=10.0.0.1      # Answer DNS request by self
domain-needed        # Don't forward short names
bogus-priv           # Never forward addresses in the non-routed address spaces.
dhcp-range=10.0.0.50,10.0.0.150,24h # Assign IP addresses between 10.0.0.50 and 10.0.0.150 with a 24 hour lease time
```

```shell
sudo service hostapd start
sudo service dnsmasq start
```

Check. Double check. 

```shell
sudo reboot
```

Check again.


Shutdown script
---------------

We need a nice shutdown of the RasPi so no data is getting lost. I wrote a small
shutdown.py script that listens to one of the IO-pins. If pin is connected to
GND, the script calls sudo halt.

Connect a momentary switch between Pin 26 and GND on the GPIOs.

If you want to skip this, shutdown with a keyboard or so.

You might also want to try to add dtoverlay=gpio-shutdown,gpio-pin=X to /boot/config.txt (Thanks to @sxwid) instead of using the shutdown script.



(Almost) Done!
--------------

Please refer to the gphoto section below for gphoto setup for your specific camera model!

You might then reboot and enjoy the photobooth slideshow on 
http://10.0.0.1:8000





Detailed information on the Hows and Whys
=========================================

Main Control
------------

All needed processes are started by photobooth.sh which runs at startup.


gphoto
------

All the communication with the camera is done by gphoto2.
Please check their documentation to get your camera hooked up.

Photos are stored on the cameras memory card. This is set in photobooth.sh with:

```shell
gphoto2 --set-config-index /main/settings/capturetarget=1
```

You might want to adjust that command to your camera needs. Find the right
capturetarget with this command and identify the correct choice:

```shell
gphoto2 --get-config=capturetarget
```

```
Label: Capture Target                                                          
Type: RADIO
Current: Internal RAM
Choice: 0 Internal RAM
*Choice: 1 Memory card*
```

Try to set manually:

```shell
gphoto2 --set-config-index /main/settings/capturetarget=1
```

Check:

```shell
gphoto2 --get-config=capturetarget
```

```
Label: Capture Target                                                          
Type: RADIO
*Current: Memory card*
Choice: 0 Internal RAM
Choice: 1 Memory card
```

**Update photobooth.sh to your needs.**

gphoto waits for a photo to be taken and then downloads the photo to the 
RasPi. After downloading, the images are processed by postprocessing.sh 

You can test this manually if needed:

```shell
gphoto2 --wait-event-and-download --keep --hook-script=postprocessing.sh
```

At the moment RAW-files are just moved to images/raw
JPEGs are processed with imagemagicks convert command. You might want to check
the options in postprocessing.sh and adjust them to your needs.


Slideshow
---------

There is a minimal webserver running on port 8000. See server.py for details.
index.html redirects the viewer to the slideshow.

Mainly it is requesting the image-count on the RasPi and enques all new images
to the slideshow. This is done by an ajax-request for /image_count.txt on the
server. Then filenames and paths are generated in javascript.
Take a look at slideshow/index.html and the JS-vars image_path and image_extension
You need to adjust these to chances in the postprocessing.sh accordingly.

The slideshow is optimized for 1920x1080 screens.
Modify #slideshow and #slideshow img in slideshow.css to your needs.

Credits for the Ken Burns effect go to:

http://cssmojo.com/ken-burns-effect/
http://www.css-101.org/articles/ken-burns_effect/css-transition.php

