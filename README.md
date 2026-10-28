# badge_ICA2026
KiCAD etc files for the ICA 2026 PCB badge.
I am not entirely sure which files to place here, as I read some varying info on the web, so, best guess.

These files should get you a game-controller shaped blinking badge (four LEDs, colors/positions matching the Xbox controller), based on the ATMEGA 328PB chip. Has a Sleep button and a Mode button (to change blink patterns). Wants a 3v coin battery (2025/2032), and uses a connector off the programmer from Tag-Connect. Only has SMD components (so no THT on the back to catch on clothing, it's a badge to be worn). 

My coding is mostly Python and R scripts (linear), and I learned just before event-loop programming took hold, so my code works but I have been told it is a bit odd and there are better ways to do it. Next time!

Should have enough files for...

Badge - shape, art, components and layout (so the schematic, layout, and program files). 

BOM - Bill Of Materials (the parts to order).

Code - Arduino / C++ code (the .ino file). 

None of this would have happened without a lot of helpful people - posts to forums about schematics and code, sample working schematics (I copied the Arduino Uno schematic to start), some help from friends and relatives who know this material, and help from some people on the Arduino forum about the sleep code for the chip. 


Fall 2026 Update

While debugging the 2027 code (for the 2027 badge), we used Claude Code because some things that had worked in the 2026 code weren’t working. We discovered some errors in the 2026 code that either didn’t effect things or worked via a bit of random luck (specifically the pin assignments for the LEDs).

I can’t actually test these changes on the 2026 badges because I gave all of them away! Which was great. But the changes work well enough in the 2027 code. So I’m not updating the code for 2026, although if I ever ordered more of those badges I would and then I could test it.

LED pins should be…
const int some_led = PIN_PD0
…otherwise they aren’t quite assigned correctly except in some cases, but that’s not intentionally it’s just the two naming schemes happen to align briefly. That is, if you use the “PD0” code/location/name, you have to use “PIN_” in front of it.

Also, from Claude, and assuming no hallucinations, but these changes all seem to not negatively affect the 2027 board:

The analog comparator is never turned back on. ACSR |= (0 << ACD) does nothing. It should be ACSR &= ~(1 << ACD);.
[That’s annoying since I got that code from somewhere official looking.]

The brown-out detector isn't actually turned off during sleep. sleep_bod_disable() only lasts a few clock cycles before sleep has to start, and the ADC/ACSR lines in between use up that window. Use this order:
  ADCSRA &= ~(1<<ADEN);
  ACSR |= (1<<ACD);
  sleep_enable();
  sleep_bod_disable();
  sleep_mode();

Wake-up interrupt names. These work by luck. PD3 is PCINT19. Writing PCINT3 into PCMSK2 happens to set the same bit, but bitSet(PCMSK2, PCINT19) is clearer. 
The 328PB also has a fourth pin-change group, so the "clear all flags" line should be PCIFR = 0x0F.



