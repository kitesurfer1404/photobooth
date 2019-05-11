# Connect a momentary switch between GND and this pin.
# As soon as this pin is switched to ground, the RasPi will shut down.
# For numbering see: 
# https://www.raspberrypi.org/documentation/usage/gpio/README.md
shutdown_pin = 26

################################################################################
import RPi.GPIO as GPIO
import subprocess

GPIO.setmode(GPIO.BCM)
GPIO.setup(shutdown_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

try:
  while True:
    if not GPIO.input(shutdown_pin):
      proc = subprocess.Popen(["sudo", "halt"])
        
except KeyboardInterrupt:
  GPIO.cleanup()
  print "\n"
  print "Goodbye. Have a nice day!"
