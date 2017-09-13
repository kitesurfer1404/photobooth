Photobooth Printer
==================

Is is a collection of scripts for a printing setup using a canon selphy printer
and the WiFi Photobooth.

It is a almost undocumente mess. But it works. Proofen on my sisters wedding.


Main idea
---------

This code runs on a linux PC which is connected to the Photobooth and has a static IP.
There is a webserver running locally provinding the webinterface.
A browser is running in full screen mode and shows the webinterface in a kind of "Kiosk mode".

There are four arcade buttons connected via USB encoder board (search "arcade diy usb" on ebay or amazon) to controll the webinterface:

* jump to previous image
* jump to next image
* jump to newest image
* print

This can be skipped. The buttons control the webinterface by simulating keystrokes A, B, C and D. You might just use the keyboard.

Pressing "D" sends an ajax request which starts the print.sh script on the machine.
print.sh uses the Selphy Go tool from http://git.shaftnet.org/cgit/users/pizza/public/selphy_go.git/ to print the image on the Canon Selphy printer via WiFi (connect Selphy to Photobooth WiFi with static IP 10.0.0.3)


Install
-------


```shell
sudo apt install xdotool unclutter joy2key
```

Grep a copy of

http://git.shaftnet.org/cgit/users/pizza/public/selphy_go.git/

and compile. This needs some Go compiler stuff. Unsure which packages I installed for that. Sorry.
Add the selphy binary to the printer directory.


Running
-------

Just run start_printer.sh in a terminal on your desktop.



More information
----------------


Arcade/Joystick documentation: http://cy-822b.blogspot.de/

Selphy Go Code: http://git.shaftnet.org/cgit/users/pizza/public/selphy_go.git/


TODO
----
* Update documentation
* Add images of the control box to website
* cleanup code

