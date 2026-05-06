// For the ICA South Africa 2026 Conference Badge project.
// Xbox-like controller, with blinky LEDs where the controller buttons are.
// Summer / Fall 2025
// Sleep help from jim-p and alto777 (Arduino forum).

// Include Libraries
// For Sleep and Interrupt functionality.
#include <avr/sleep.h>
#include <avr/interrupt.h>


// Global Variables
// YRGB is the order of the lights, starting at noon and going clockwise (based on the buttons on an XBox 360 controller).
// SET PINS HERE (note difference between Arduino Metro board and custom board pin numbers).
// You can't use the simple pin numbers from the chip, you need to translate them since the Arduino IDE uses a different scheme.
// Here:  https://community.platformio.org/t/atmega328pb-arduino-pin-mappings/4248 
const int led_yellow = PD2; // ATMega328PB pin references.
const int led_red    = PD0;  
const int led_green  = PD1;  // Weird order due to circuit layout realities of the PCB and the ATMEGA328PB.
const int led_blue   = PD3;

// Different online schematics show different pin desginations. Beware!
const int button_onoff = 8; //12;  PB0  // Really is sleep, PULL UP RESISTOR (long story).
const int button_mode  = 9; //13;  PB1  // The mode button, PULL DOWN RESISTOR.

// Mode buttons setup here.
// https://docs.arduino.cc/built-in-examples/digital/StateChangeDetection/
int button_state = 0;  // Variable for reading the pushbutton status.
int button_prev  = 0;  // LOW. For the previous state of the button, helps get one click not a human-length hold/click.

// Pattern varaibles.
const int num_patterns = 13; // How many pattens do I have.
int pattern = 1; // 1 to num_patterns for # patterns of LED blinking. Will start at this pattern number. 

// The ISR SUPER IMPORTANT, MCU and schematic specific. 
ISR(PCINT0_vect) {
	PCICR &= ~_BV(PCIE0);  // Disable the PCINT[7:0] interrupts
}


void setup() {
  set_unused_pins(); // Set unused pins as inputs with pull-ups enabled (prevents floating pins from drawing current).

  pinMode(led_yellow, OUTPUT); // Set LED pins after setting unused pins, otherwise the LEDs are dim, I don't know why. 
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_blue, OUTPUT);
  // Ports B, C, D, and E all have internal resistors that can be set.
  pinMode(button_mode, INPUT);          // Build has a pulldown resistor on it.   
  pinMode(button_onoff, INPUT_PULLUP);  // INPUT_PULLUP if I set it like that, is that chip-internal? (I have a resistor on the board.)
      // Input pullups essentially set the default state of the pin to HIGH or 1, so that the pin is not affected by interference.
      // Do not need a resistor in this case. (I have one on the board.)

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);  // Set sleep mode to full power down.

  init_off(); // Make sure all LEDs are set to LOW to start.
}

void loop() {
  if (pattern == 1)  blink();      // Starts on this one (unless changed above). 
  if (pattern == 2)  rotate();
  if (pattern == 3)  alternate(); 
  if (pattern == 4)  snake(); 
  if (pattern == 5)  rave();  
  if (pattern == 6)  random_slow(); 
  if (pattern == 7)  random_medium(); 
  if (pattern == 8)  random_fast(); 
  if (pattern == 9)  random_path();
  if (pattern == 10) bounce();
  if (pattern == 11) sideside(); 
  if (pattern == 12) fire(); 
  if (pattern == 13) fireworks();    // If you change these, change var num_patterns above.
} // main loop



// SLEEP FUNCTION hooray
void sleeper() {           // c/o jim-p (Arduino forum)
  bool yellow_on = (digitalRead(led_yellow) == HIGH);  // Have to reset the LEDs upon return from sleep.
  bool red_on    = (digitalRead(led_red) == HIGH);
  bool green_on  = (digitalRead(led_green) == HIGH);
  bool blue_on   = (digitalRead(led_blue) == HIGH);
  init_off();              // Turns all LEDs off, because it can catch them on at button press. 
	PCIFR = 0x07;            // Clear all the change on interrupt flags
	bitSet(PCICR, PCIE0);    // Enanble change interrupts on PCINT[7:0] pins
	bitSet(PCMSK0, PCINT0);  // Enable for PCINT0 (D8) This is the pin the button is connected to.
	interrupts();            // Make sure interrupts are enabled
	sleep_bod_disable();     // Disble BOD; saves power
  ADCSRA &= ~(1<<ADEN);    // Disable the ADC, must be re-endabled.
  ACSR |= (1 << ACD);      // Disable the analog comparator, must be re-endabled.
	sleep_mode();            // Enables sleep, puts cpu to sleep and disbles sleep when awoken

	// We are now ASLEEP, will stay here until interrupt occurs (the interrrupt that we just put on pin D8 with the "bitSet" lines just above).
	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
	// Now we are awake here because pin D8/PB0 got a signal.
  do {delay(20);}
    while (digitalRead(button_onoff) == LOW);     // Debounce, finger to get off the button. c/o alto777 (Arduino forum)
  ADCSRA |= (1<<ADEN);                            // Re-enables the ADC.
  ACSR |= (0 << ACD);                             // Re-enables the analog comparator.
  if (yellow_on) digitalWrite(led_yellow, HIGH);  // Set the LEDs back to where they were (matters for some patterns).
  if (red_on) digitalWrite(led_red, HIGH);
  if (green_on) digitalWrite(led_green, HIGH);
  if (blue_on) digitalWrite(led_blue, HIGH);
}


void set_unused_pins() {
  // Set unused pins as inputs with pull-ups enabled (prevents floating pins from drawing current).
  // The pins here are never used in this design (ICA SA 2026 XBOX badge).
  // Top and Right side of 328PB
  pinMode(PC5, INPUT_PULLUP); // 28
  pinMode(PC4, INPUT_PULLUP); // 27
  pinMode(PC3, INPUT_PULLUP); // 26
  pinMode(PC2, INPUT_PULLUP); // 25
  pinMode(PC1, INPUT_PULLUP); // 24
  pinMode(PC0, INPUT_PULLUP); // 23
  pinMode(PE3, INPUT_PULLUP); // 22
  pinMode(PE2, INPUT_PULLUP); // 19

  // Bottom of 328PB
  pinMode(PD5, INPUT_PULLUP); //  9
  pinMode(PD6, INPUT_PULLUP); // 10
  pinMode(PD7, INPUT_PULLUP); // 11
  pinMode(PB2, INPUT_PULLUP); // 14

  // Left side of 328PB
  pinMode(PD4, INPUT_PULLUP); // 2
  pinMode(PE0, INPUT_PULLUP); // 3
  pinMode(PE1, INPUT_PULLUP); // 6
  pinMode(PB6, INPUT_PULLUP); // 7
  pinMode(PB7, INPUT_PULLUP); // 8
}


// SECONDARY FUNCTIONS (these assist the main pattern functions).

// Helper functions that set LEDs to various states.
void init_off() { 
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_red, LOW);
  digitalWrite(led_green, LOW);
  digitalWrite(led_blue, LOW);
}
void init_on() {
  digitalWrite(led_yellow, HIGH);
  digitalWrite(led_red, HIGH);
  digitalWrite(led_green, HIGH);
  digitalWrite(led_blue, HIGH);
}

// For the mode/pattern button, which pattern by number.
void increment() {
  pattern++; // Pattern is a global var so it is just set, not returned.
  if (pattern > num_patterns) pattern = 1;
  init_off(); // Because if you're in this function, the mode button was pushed, so start new pattern.
  delay(70); // Perhaps avoid bounce?
}

// If the LEDs are on turn them off and vice-versa, for the blink pattern.
void reverse_all() {
  if (digitalRead(led_red) == HIGH) init_off(); // If one is on, they are all on, turn all off.
  else init_on();
} // reverse_all

// These two are used by the rave function, if off, turn it on and return an "on until" time, etc. 
unsigned long rave_on(int which_led){
  digitalWrite(which_led, HIGH);
  unsigned long on_until = millis() + random(5, 50);  // times adjusted
  return on_until;
}
unsigned long rave_off(int which_led) {
  digitalWrite(which_led, LOW); 
  unsigned long off_until = millis() + random(5, 50);
  return off_until;
}

// All of these four turn one on and make sure the others are off.
void yellow_only() {
  digitalWrite(led_yellow, HIGH);
  digitalWrite(led_red, LOW);
  digitalWrite(led_green, LOW);
  digitalWrite(led_blue, LOW);
}
void red_only() {
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_red, HIGH);
  digitalWrite(led_green, LOW);
  digitalWrite(led_blue, LOW);
}
void green_only() {
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_red, LOW);
  digitalWrite(led_green, HIGH);
  digitalWrite(led_blue, LOW);
}
void blue_only() {
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_red, LOW);
  digitalWrite(led_green, LOW);
  digitalWrite(led_blue, HIGH);
}

// Gamma Correction Function (for fading the LEDs).
int gamma_correct(int uncorrected_level) {
  // Convert 0-255 to 0.0-1.0 .
  float decimal_level = uncorrected_level / 255.0; // "If doing math with floats, you need to add a decimal point, otherwise it will be treated as an int."  = 0
  // Gamma adjust.
  float corrected_decimal = decimal_level * decimal_level * decimal_level;
  // Convert back to 0-255 metric and return it.
  return corrected_decimal * 255;
}


// PRIMARY FUNCTIONS these are the patterns (they may call a seconadry function for common actions).

void blink() {
  unsigned long start = millis();
  unsigned long now;
  init_on();

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - start >= 30) {   // beta timing for board
      reverse_all();  //
      start = millis();
    }
  } // while
} // blink


void rotate() {
  unsigned long now = millis();
  unsigned long y_off = now + 45;
  unsigned long r_off = now + 90;
  unsigned long g_off = now + 135;
  unsigned long b_off = now + 180;

  digitalWrite(led_yellow, HIGH);

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }

    if (now >= y_off) {
      digitalWrite(led_yellow, LOW);
      digitalWrite(led_red, HIGH);
      y_off = now + 180;
      r_off = now + 45;
    } 
    else if (now >= r_off) {
      digitalWrite(led_red, LOW);
      digitalWrite(led_green, HIGH);
      r_off = now + 180;
      g_off = now + 45;
    }
    else if (now >= g_off) {
      digitalWrite(led_green, LOW);
      digitalWrite(led_blue, HIGH);
      g_off = now + 180;
      b_off = now + 45;
    }
    else if (now >= b_off) {
      digitalWrite(led_blue, LOW);
      digitalWrite(led_yellow, HIGH);
      b_off = now + 180;
      y_off = now + 45;
    }   
  } // while
} // rotate


void alternate() {
  // Up and down, then left and right.
  const int alternate_delay = 50; // The millis delay for this function.  Changed for PCB.
  unsigned long ud_on = millis();  // Up Down when it has turned on.
  unsigned long lr_on = ud_on + alternate_delay; // Left Right on.
  unsigned long now;

  digitalWrite(led_yellow, HIGH);
  digitalWrite(led_green, HIGH);
  digitalWrite(led_red, LOW);
  digitalWrite(led_blue, LOW);

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - ud_on >= alternate_delay) {
      digitalWrite(led_yellow, LOW);
      digitalWrite(led_green, LOW);
      digitalWrite(led_red, HIGH);
      digitalWrite(led_blue, HIGH);
      ud_on = lr_on + alternate_delay; // e.g., 50 + 50
    }
    else if (now - lr_on >= alternate_delay) {
      digitalWrite(led_yellow, HIGH);
      digitalWrite(led_green, HIGH);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, LOW);
      lr_on = ud_on + alternate_delay;
    }
  } // while
} // alternate


void snake() {
  // Grows and then shrinks    YRGB
  const int on_for    = 125;  // Times adjusted for PCB. 
  const int end_pause = 125;  // Pause before restarting the snake loop.

  unsigned long y_on  =  0;
  unsigned long r_on  = 25;
  unsigned long g_on  = 50;
  unsigned long b_on  = 75;
  unsigned long y_off = y_on + on_for;
  unsigned long r_off = r_on + on_for;
  unsigned long g_off = g_on + on_for;
  unsigned long b_off = b_on + on_for;
  unsigned long now   =  0; // This was why the start pattern was wonky.
  unsigned long start = millis();

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now >= y_on) {
      digitalWrite(led_yellow, HIGH);
      y_on = y_off + end_pause;
    }
    if (now >= r_on) {
      digitalWrite(led_red, HIGH);
      r_on = r_off + end_pause;
    }
    if (now >= g_on) {
      digitalWrite(led_green, HIGH);
      g_on = g_off + end_pause;
    }
    if (now >= b_on) {
      digitalWrite(led_blue, HIGH);
      b_on = b_off + end_pause;
    }
    if (now > y_off) {
      digitalWrite(led_yellow, LOW);
      y_off = y_on + on_for;
    }
    if (now > r_off) {
      digitalWrite(led_red, LOW);
      r_off = r_on + on_for;
    }
    if (now > g_off) {
      digitalWrite(led_green, LOW);
      g_off = g_on + on_for;
    }
    if (now > b_off) {
      digitalWrite(led_blue, LOW);
      b_off = b_on + on_for;
    }
    now = millis() - start; // Should time the function and add it to the "now".
  } // while
} // snake


void rave() {
  // This is random on all of them: random(min, max+1).
  unsigned long yellow_on_until  = 0;
  unsigned long yellow_off_until = 0;
  unsigned long red_on_until     = 0;
  unsigned long red_off_until    = 0;
  unsigned long green_on_until   = 0;
  unsigned long green_off_until  = 0;
  unsigned long blue_on_until    = 0;
  unsigned long blue_off_until   = 0;
  unsigned long now = 0;

  // Start all on with random rave on values.
  yellow_on_until = rave_on(led_yellow);
  red_on_until = rave_on(led_red);
  green_on_until = rave_on(led_green);
  blue_on_until = rave_on(led_blue);

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    // If on, should it be turned off? If off, should it be turned on?
    if (digitalRead(led_yellow) == HIGH) { if (now > yellow_on_until) yellow_off_until = rave_off(led_yellow); }
    else { if (now > yellow_off_until) yellow_on_until = rave_on(led_yellow); }

    if (digitalRead(led_red) == HIGH) { if (now > red_on_until) red_off_until = rave_off(led_red); }
    else { if (now > red_off_until) red_on_until = rave_on(led_red);  }

    if (digitalRead(led_green) == HIGH) { if (now > green_on_until) green_off_until = rave_off(led_green); }
    else { if (now > green_off_until) green_on_until = rave_on(led_green); }

    if (digitalRead(led_blue) == HIGH) { if (now > blue_on_until) blue_off_until = rave_off(led_blue); }
    else { if (now > blue_off_until) blue_on_until = rave_on(led_blue); }

  } // while
} // rave function


void random_slow() {
  unsigned long pause = 35;
  rando(pause);
}

void random_medium() {
  unsigned long pause = 21;
  rando(pause);
}

void random_fast() {
  unsigned long pause = 7;
  rando(pause);
}

void rando(unsigned long pause) {
  int prev   = 0;
  int prev_2 = 0;
  
  unsigned long now   = 0;
  unsigned long start = millis();

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - start >= pause) {
      start = millis();
      int roll = random(1,5); // Gives it a value so you can compare on next line.
      while (roll == prev_2) {roll = random(1,5);} // Reroll -- allows for 1 match but not more than that.
      prev_2 = prev; // Okay passed, so update these two.
      prev = roll;
      if (roll == 1) {
        yellow_only();
      }
      else if (roll == 2) {
        red_only();
      }
      else if (roll == 3) {
        green_only();
      }
      else if (roll == 4) {
        blue_only();
      }
    } // if now - start
  } // while true
} // rando


void random_path() {
  // Random, but only activates a neighbor (a "path"). 
  unsigned long last = 0;
  unsigned long now = 0; 
  const int pause = 40;           // adjusted for PCB
  int which = 1; // 1y 2r 3g 4b
  int upit = 0; // Int read as Boolean, random, "up it" up the value or not, beware wrap 1-4.

  digitalWrite(led_yellow, HIGH);
  last = millis();

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - last >= pause) {   // Time to move it one.
      // First get the new LED number.
      upit = random(0,2); // Returns 0,1 can be read as Boolean.
      last = millis(); // Update that.
      if (upit) {
        which++;
        if (which > 4) which = 1;
      }
      else { // Else don't up-it, lower it 1-4.
        which--;
        if (which < 1) which = 4;
      }
      if (which == 1) yellow_only();
      else if (which == 2) red_only();
      else if (which == 3) green_only();
      else blue_only();
    } // if time to move it one

  } // while true
} // random_path


void bounce() { // 1, then 2&4, then 3, then 2&4, then 1, etc
  // Similar to sideside function.
  int which_step = 1; // Which step of the pattern, so which is on and which is next (1, 2, 3, 4).
  unsigned long now = 0;
  unsigned long last = millis();
  const int pause = 33;  // adjusted

  yellow_only(); // start

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - last >= pause) {
      last = millis();
      which_step++;
      if (which_step > 4) which_step = 1;
      if      (which_step == 1) yellow_only();
      else if (which_step == 2) {digitalWrite(led_red, HIGH); digitalWrite(led_blue, HIGH); digitalWrite(led_yellow, LOW);}
      else if (which_step == 3) green_only();
      else if (which_step == 4) {digitalWrite(led_red, HIGH); digitalWrite(led_blue, HIGH); digitalWrite(led_green, LOW);}
    }
  } // while true
} // bounce


void sideside() { // u, r, d, r, u, l, d, l, u, etc, 1 2 3 4 for position, do where in pattern?
  int which_step = 1; // Which step of the pattern, so which is on and which is next.
    // 1u 2r 3d 4r 5u 6l 7d 8l 9u that is the start again so 1-8.
  unsigned long now = 0;
  unsigned long last = millis();
  const int pause = 33;  // adjusted

  yellow_only(); // start

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - last >= pause) {
      last = millis();
      which_step++;
      if (which_step > 8) which_step = 1;
      if      (which_step == 1) yellow_only();
      else if (which_step == 2) red_only();
      else if (which_step == 3) green_only();
      else if (which_step == 4) red_only();
      else if (which_step == 5) yellow_only();
      else if (which_step == 6) blue_only();
      else if (which_step == 7) green_only();
      else if (which_step == 8) blue_only();
    }
  } // while true
} // sideside


void fire() {
  // Start on blue, blink three times, fire aross to red, bounce back to y/g and b.
  const unsigned long pause_between = 175;
  // Seven blinks in the pattern. Hard code the entire thing why not.
  unsigned long starter   = 0;
  unsigned long step1_on  = 10; // B blinks three times.
  unsigned long step1_off = 20;
  unsigned long step2_on  = 30; 
  unsigned long step2_off = 40;
  unsigned long step3_on  = 50;
  unsigned long step3_off = 100;  // longer pause before firing  100 so far    times adjusted
  unsigned long step4_on  = 105;   // b fires
  unsigned long step4_off = 110;
  unsigned long step5_on  = 115;   // hits r
  unsigned long step5_off = 120;
  unsigned long step6_on  = 125;   // bounces back to y/g
  unsigned long step6_off = 130;
  unsigned long now;

  unsigned long start = millis();

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    now = millis();
    if (now - start >= starter) {digitalWrite(led_blue, HIGH); starter += (155 + pause_between);}
    else if (now - start > step1_on) {digitalWrite(led_blue, LOW); step1_on += (155 + pause_between);}
    else if (now - start > step1_off) {digitalWrite(led_blue, HIGH); step1_off += (155 + pause_between);}
    else if (now - start > step2_on) {digitalWrite(led_blue, LOW); step2_on += (155 + pause_between);}
    else if (now - start > step2_off) {digitalWrite(led_blue, HIGH); step2_off += (155 + pause_between);}
    else if (now - start > step3_on) {digitalWrite(led_blue, LOW); step3_on += (155 + pause_between);}
    else if (now - start > step3_off) {digitalWrite(led_blue, HIGH); step3_off += (155 + pause_between);}   // blue fires
    else if (now - start > step4_on) {digitalWrite(led_blue, LOW); step4_on += (155 + pause_between);}
    else if (now - start > step4_off) {digitalWrite(led_red, HIGH); step4_off += (155 + pause_between);}  // s4 done, red lights up
    else if (now - start > step5_on) {digitalWrite(led_red, LOW); step5_on += (155 + pause_between);}
    else if (now - start > step5_off) {digitalWrite(led_yellow, HIGH); digitalWrite(led_green, HIGH); step5_off += (155 + pause_between);}  // red done, y and g light up
    else if (now - start > step6_on) {digitalWrite(led_yellow, LOW); digitalWrite(led_green, LOW); step6_on += (155 + pause_between);}
    else if (now - start > step6_off) {
      digitalWrite(led_blue, HIGH); 
      step6_off += (155 + pause_between);
      int x = 200; // HIGH val for LED rounded down a bit.
      float y = 0.0;
      unsigned long previous_millis = millis();
      while (true){
        if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
          do {delay(20);}
            while (digitalRead(button_onoff) == LOW); 
          sleeper();
        }
        now = millis();
        button_prev = button_state; 
        button_state = digitalRead(button_mode);
        if (button_state == HIGH && button_prev == LOW) { // signal from mode button
          increment();
          return; // break out of the function completely
        }
        if (x <= 0) break; // should break out of this inner while but not return
        if (now - previous_millis >= 5) { 
          previous_millis = millis();
          x = x - 10;
          y = gamma_correct(x);
          analogWrite(led_blue, y);
        } 
      } // while inner true / button catch  
    } // If step_6 is completed.
  } // while true
} // fire


void fireworks() {
  // g pops, then y lights up and fades and r/b sparkle (back forth fast, and fade), like a firework. Requires PWM.
  unsigned long now;
  unsigned long start = millis();
  unsigned long previous_millis = millis(); // For fade cycle.
  const int fade_interval = 7;
  bool sparkle_lr = 1; // To alternate (sparkle) the two side LEDs.
  int x = 200; // For brightness of LED fade.
  int y; // Gamma corrected version of our good friend x.

  const int pause_between   = 150; // This is important or the green fires too soon. Also interacts with fade_interval and x.
  unsigned long step1_start = 50;  // Initial firing of the firework.
  unsigned long step1_end   = 57;
  unsigned long step2_start = 100;

  while (true) {
    if (digitalRead(button_onoff) == LOW) {     // if on/off button press, do sleep function call
      do {delay(20);}
        while (digitalRead(button_onoff) == LOW); 
      sleeper();
    }
    now = millis();
    button_prev = button_state; 
    button_state = digitalRead(button_mode);
    if (button_state == HIGH && button_prev == LOW) { // signal from mode button
      increment();
      return; // break out of the function
    }
    if (now - start > step1_start && now - start < step1_end) digitalWrite(led_green, HIGH);
    else if (now - start > step1_end) {
      digitalWrite(led_green, LOW);
    }
    if (now - start > step2_start && x > 0) { // If in step2, x is the LED fade.
      if (now - previous_millis >= fade_interval) {  // If so, time to +/- LEDs.
        sparkle_lr = !sparkle_lr;
        previous_millis = millis();
          x = x - 7;
          y = gamma_correct(x);
          analogWrite(led_yellow, y);
          if (sparkle_lr) {analogWrite(led_blue, y); digitalWrite(led_red, LOW);}
          else {analogWrite(led_red, y); digitalWrite(led_blue, LOW);}
      } // if it is time to +/- the LEDs
    }  // if in step2
    else if (x <= 0) {
      step1_start = step1_start + pause_between;  // If these step1 aren't here, they get out of sync due to x not being millis-centric. 
      step1_end   = step1_end + pause_between;
      step2_start = step2_start + pause_between;
      start       = start + pause_between;
      init_off();
      x = 200;
    }

  } // while true
} // fireworks

// END OF CODE


