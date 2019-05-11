#!/bin/python
# Connect a momentary switch between GND and this pin.
# As soon as this pin is switched to ground, the RasPi will shut down.
# For numbering see: 
# https://www.raspberrypi.org/documentation/usage/gpio/README.md
shutdown_pin = 26

################################################################################
import RPi.GPIO as GPIO
import subprocess
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(shutdown_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def shutdown(channel):
    time.sleep(5)
    if not GPIO.input(shutdown_pin):
        print("Shutting down. Goodbye!")
        proc = subprocess.Popen(["sudo", "halt"])

try:
    GPIO.add_event_detect(shutdown_pin, GPIO.FALLING, callback=shutdown, bouncetime=5000)
    while 1:
        time.sleep(1)

except KeyboardInterrupt:
  GPIO.cleanup()
  print "\n"
  print "Goodbye. Have a nice day!"
