/*
  Arduino Photobooth

  Harm Aldick - 2015
  http://www.kitesurfer1404.de/tech/photobooth/en
  https://github.com/kitesurfer1404/photobooth


  FEATURES
    * 7-segment-display for countdown
    * LED-speedlights
    * Pushbutton-support
    * "breathing" pushbutton-LED
    * Standalone or remote controlled operation (USB-serial)


  NOTES
    * Pushbutton-pins are configured to use internal pullup resistors 
      of the Arduino for less external parts. If you want to use external 
      pullup resistors change INPUT_PULLUP to INPUT in setup().
    * Set your camera to continuous focus for best results.
    * Use exposure compensation to compensate the dimmed focusing light. 
      Values arround -1 EV work best for me in low light situations.
    * You might also need to adjust values in takePhoto() to your needs.
      Brightness and delays might be different with different LED-bulbs.
    * Serial speed is 9600 Baud. See processCommand() for a complete 
      list of supported commands.
    * There is support for multiple pushbuttons - although only one is used
      by now. When connecting to a Raspberry Pi a second pushbutton might be
      used to shut the Pi down due to its lack of a power-down-button. 
    * analogWrite() works only on Pins 3, 5, 6, 9, 10, 11 - that's why the LED-spots
      are connected to pins 10, 11, 5, 6 so we can get dimmed focusing light.
    * Make sure to connect all pushbutton LEDs to PWM outputs so the breathing
      will work.
    * Read the datasheet of your 7-segment-display for pin-layout. 
      Connect this way:
    
       Segment     Pin (Arduino)
          A         2
          B         3
          C         4
          D         7
          E         8
          F        12
          G        13

      Use darlington array and and current limiting resistors as appropriate.
    * See repository for example circuit.
    
    * This software is free and provided "as is" - but I appreciate a donation when
      you make money off it (e.g. as a commercial photographer).


  LICENSE
  
  The MIT License (MIT)

  Copyright (c) 2015  Harm Aldick 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG

  2016-01-16   Initial beta release

*/


/* This is where you might define different IO-pins */

int segment_pins[] = { 2, 3, 4, 7, 8, 12, 13 }; // pins segments A-G of the display are connected to
int spot_pins[] = { 10, 11, 5, 6 };             // pins the spot-lights are connected to, use PWM-pins here

int pin_focus = 15;   // camera: focus (using analog pin A1 as digital output)
int pin_shutter = 16; // camera: shutter (using analog pin A2 as digital output)

int pushbutton_leds[] = { 9 };  // pin(s) on which the LED(s) of the pushbutton-switch(es) is/are connected to. Use PWM-pins here.
int pushbutton_pins[] = { 14 }; // pin(s) on which the pushbutton(s) is/are connected to. Using analog pin A0 as digital input
int lastButtonState[] = { 1 };  // saving the last state(s) of the button(s)


/* Some magic configuration happening below. */

byte segment_digits[10][7] = { // segments to light up on the corresponding index
  { 1,1,1,1,1,1,0 },    // 0
  { 0,1,1,0,0,0,0 },    // 1
  { 1,1,0,1,1,0,1 },    // 2
  { 1,1,1,1,0,0,1 },    // 3
  { 0,1,1,0,0,1,1 },    // 4
  { 1,0,1,1,0,1,1 },    // 5
  { 1,0,1,1,1,1,1 },    // 6
  { 1,1,1,0,0,0,0 },    // 7 
  { 1,1,1,1,1,1,1 },    // 8
  { 1,1,1,1,0,1,1 }     // 9
};

int breath_delay_steps[] =       {   7,   9,  13, 15, 16, 17, 18, 930, 19, 18, 15, 13,   9,   7,   4,   5 }; // magic numbers for breathing LED
int breath_brightness_steps[] =  { 150, 125, 100, 75, 50, 25, 16,  15, 16, 25, 50, 75, 100, 125, 150, 220 }; // even more magic numbers!

// initial/default values for breathing LED
#define breath_initial_brightness 15
#define breath_initial_direction 1
#define breath_initial_index 8
#define breath_initial_delay 1

int breath_brightness; // initial brightness of breathing LED
int breath_direction;  // increase brightness
int breath_index;      // start at index 8 of our magic numbers
int breath_delay;      // no delay at startup, load values imidiately
boolean breathing_enabled = true;  // enables/disables breathing LED at all.

String inputCommand = "";         // a string to hold incoming data from serial
boolean commandComplete = false;  // whether the command string is complete

boolean mode_auto = true; // default is running in standalone-mode



// The setup function runs once when you press reset or power the board.
void setup() {
  // Set all 7-segment-pins as output and turn LEDs off
  for(int i = 0; i < sizeof(segment_pins)/sizeof(int); i++) {
    pinMode(segment_pins[i], OUTPUT);
    digitalWrite(segment_pins[i], LOW);
  }
  
  // Turn all spots off.
  for(int i = 0; i < sizeof(spot_pins)/sizeof(int); i++) {
    digitalWrite(spot_pins[i], 0);
  }
  
  // Set all pushbutton-LED-pins as output and turn LEDs off
  for(int i = 0; i < sizeof(pushbutton_leds)/sizeof(int); i++) {
    pinMode(pushbutton_leds[i], OUTPUT);
    digitalWrite(pushbutton_leds[i], LOW);
  }
  
  // Set all pushbutton-pins as input and enable pullup resistor
  for(int i = 0; i < sizeof(pushbutton_pins)/sizeof(int); i++) {
    pinMode(pushbutton_pins[i], INPUT_PULLUP); // change INPUT_PULLUP to INPUT when there is an external pullup-resistor attached
  }
  
  // setup focus and shutter pins
  pinMode(pin_focus, OUTPUT);
  digitalWrite(pin_focus, LOW);
  pinMode(pin_shutter, OUTPUT);
  digitalWrite(pin_shutter, LOW);
  
  
  Serial.begin(9600); // initialize serial

  inputCommand.reserve(100); // reserve 100 bytes for the inputCommand

  breathing_reset(); // reset breathing LEDs
  
  Serial.println("ready"); // we're ready to rock!
}


/* turns all breathing LEDs gently on and resets everything for restart breathing */
void breathing_reset() {
  for(int i = 0; i <= 15; i++) {
    for(int p = 0; p < sizeof(pushbutton_leds)/sizeof(int); p++) {
      analogWrite(pushbutton_leds[p], i);
    }
    delay(10);
  }

  breathing_enabled = true;
  breath_brightness = breath_initial_brightness;
  breath_direction = breath_initial_direction;
  breath_index = breath_initial_index;
  breath_delay = breath_initial_delay;
}


/* turns all breathing LEDs off and disables breathing at all */
void breathing_stop() {
  for(int i = 0; i < sizeof(pushbutton_leds)/sizeof(int); i++) {
    analogWrite(pushbutton_leds[i], 0);
  }
  breathing_enabled = false;
}


/* does one step in "breathing", so we don't block anything else too long. */
void breathing_do_step() {
  if(!breathing_enabled) {
    return;  // nothing to do if disabled
  }
  
  breath_delay--;
  
  // update brightness and delays when waited long enough
  if(breath_delay == 0) {
    // increase or decrese brightness
    breath_brightness = breath_brightness + breath_direction;

    // change direction when LED is fully lit or very dimm
    if(breath_brightness == 255 || breath_brightness == 15) {
      breath_direction *= -1; 
    }
  
    // update index of current delay when target brightness is reached
    if(breath_brightness == breath_brightness_steps[breath_index]) {
      breath_index++;
      
      // start over when the end is reached
      if(breath_index == sizeof(breath_brightness_steps)/sizeof(int)) {
        breath_index = 0;
      }
    }
 
    // get new delay
    breath_delay = breath_delay_steps[breath_index];
  
    // set new brightness to all pushbutton leds
    for(int i = 0; i < sizeof(pushbutton_leds)/sizeof(int); i++) {
      analogWrite(pushbutton_leds[i], breath_brightness);
    }

  } else {
    delay(1); // if we have still some rounds to go, wait 1ms and return
  }
}


/* flashes the 7-segment-display in a fancy way */
void animation_one(int wait) {
  for (int i = 0; i < sizeof(segment_pins)/sizeof(int); i++) {
    digitalWrite(segment_pins[i], LOW);
  }
  
  for(int n = 0; n < 5; n++) {
    for (int i = 0; i < 6; i++) {
      digitalWrite(segment_pins[i], HIGH);
      delay(wait);
      digitalWrite(segment_pins[i], LOW);
    }
  }
}


/* flashes the 7-segment-display in another fancy way */
void animation_two(int wait) {
  int animation[] = { 1, 2, 7, 5, 4, 3, 7, 6 };
  
  for (int i = 0; i < sizeof(segment_pins)/sizeof(int); i++) {
    digitalWrite(segment_pins[i], LOW);
  }
  
  for(int n = 0; n < 5; n++) {
    for(int i = 0; i < 8; i++) {
      int segment = animation[i] - 1;
      digitalWrite(segment_pins[segment], HIGH);
      delay(wait);
      digitalWrite(segment_pins[segment], LOW);
    }
  }
}


/* flashes the spots in a fancy way */
void animation_spots(int wait) {
  for (int i = 0; i < sizeof(spot_pins)/sizeof(int); i++) {
    spot_off(i);
  }
  
  for (int i = 0; i < sizeof(spot_pins)/sizeof(int); i++) {
    spot_on(i);
    delay(wait);
    spot_off(i);
  }    
}


/* turns a single spot on */
void spot_on(int i) {
  analogWrite(spot_pins[i], 255);
}


/* turns a single spot off */
void spot_off(int i) {
  analogWrite(spot_pins[i], 0);
}


/* turns all spots on ("speedlight") */
void spots_on() {
  for (int i = 0; i < sizeof(spot_pins)/sizeof(int); i++) {
    spot_on(i);
  }
}


/* turns all spots off */
void spots_off() {
  for (int i = 0; i < sizeof(spot_pins)/sizeof(int); i++) {
    spot_off(i);
  }
}


/* turns all spots on (dimmed) for fucusing the camera in low light */
void spots_dimm(int value) {
  for (int i = 0; i < sizeof(spot_pins)/sizeof(int); i++) {
    analogWrite(spot_pins[i], value);
  }
}


/* displays a number on the 7-segment-display */
void displayNumber(int n) {
  for (int i = 0; i < sizeof(segment_pins)/sizeof(int); i++) {
    digitalWrite(segment_pins[i], segment_digits[n][i]);
  }
}


/* turns the 7-segment-display off */
void display_off() {
  for (int i = 0; i < sizeof(segment_pins)/sizeof(int); i++) {
    digitalWrite(segment_pins[i], LOW);
  }
}


/* turns every light off */
void clearAll() {
  spots_off();
  display_off();
  breathing_stop();
}


/* starts countdown and focusing, than takes picture */
void takePhoto() {
  Serial.println("Taking photo.");

  spots_dimm(5);
  digitalWrite(pin_focus, HIGH);

  animation_one(50);
  
  displayNumber(3);
  delay(1000);
  displayNumber(2);
  delay(1000);
  displayNumber(1);
  delay(1000);
  
  display_off();

  //spots_on();
  spots_dimm(100);
  delay(75);
  digitalWrite(pin_shutter, HIGH);

  delay(400);
  
  clearAll();

  digitalWrite(pin_focus, LOW);
  digitalWrite(pin_shutter, LOW);
  
  Serial.println("Photo taken.");
}


/* checks received command and calls corresponding function */
void processCommand(String cmd) {
  if(cmd == "1") displayNumber(1);
  if(cmd == "2") displayNumber(2);
  if(cmd == "3") displayNumber(3);
  if(cmd == "4") displayNumber(4);
  if(cmd == "5") displayNumber(5);
  if(cmd == "6") displayNumber(6);
  if(cmd == "7") displayNumber(7);
  if(cmd == "8") displayNumber(8);
  if(cmd == "9") displayNumber(9);
  if(cmd == "0") displayNumber(0);
  
  if(cmd == "display off") display_off();

  if(cmd == "spots on") spots_on();
  if(cmd == "spots off") spots_off();
  if(cmd == "spots dimm") spots_dimm(1);
  
  if(cmd == "S1 on") spot_on(0);
  if(cmd == "S1 off") spot_off(0);
  if(cmd == "S2 on") spot_on(1);
  if(cmd == "S2 off") spot_off(1);
  if(cmd == "S3 on") spot_on(2);
  if(cmd == "S3 off") spot_off(2);
  if(cmd == "S4 on") spot_on(3);
  if(cmd == "S4 off") spot_off(3);  
  
  if(cmd == "animation 1") animation_one(50);
  if(cmd == "animation 2") animation_two(50);
  if(cmd == "animation spots") animation_spots(100);
  
  if(cmd == "clear") clearAll();
 
  if(cmd == "mode auto") mode_auto = true;
  if(cmd == "mode manual") mode_auto = false;
  
  if(cmd == "take photo") takePhoto();
}


/* reads new input from serial to commandstring. Command is complete on \n */
void serialEvent() {
  while(Serial.available()) {
    char inChar = (char)Serial.read(); 
    if(inChar == '\n') {
      commandComplete = true;
    } else {
      inputCommand += inChar; 
    }
  }
}

// This loop function runs over and over again. Forever. Poor little thing.
void loop() {

  breathing_do_step(); // do one step in "breathing"

  // check all pushbuttons for changes
  for(int i = 0; i < sizeof(pushbutton_pins)/sizeof(int); i++) {
    if(digitalRead(pushbutton_pins[i]) == LOW) {  // if button is pressed
      if (lastButtonState[i] == HIGH) {             // and was not pressed before
        lastButtonState[i] = LOW;                     // save current state (pressed)
        if(mode_auto) {                               // in standalone-mode...
          inputCommand = "take photo";                  // set command
          commandComplete = true;
        } else {                                      // in remote-controlled mode...
          Serial.print("P");                            // notify computer which button was pressed
          Serial.println(i);
        }
      }
      delay(250);                                  // poor mans debouncing
    } else {                                     // if button is not pressed
      lastButtonState[i] = HIGH;                   // save state (released)
    }
  }


  if(commandComplete) {
    breathing_stop();             // stop breathing (turn LED off)
    processCommand(inputCommand); // process command
    breathing_reset();            // enable breathing again

    inputCommand = "";            // reset the commandstring
    commandComplete = false;      // reset command complete
    
    Serial.println("ready");      // notify computer
  }
}
