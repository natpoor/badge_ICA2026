# badge_ICA2026
KiCAD etc files for the ICA 2026 PCB badge.
I am not entirely sure which files to place here, as I read some varying info on the web, so, best guess.

These files should get you a game-controller shaped blinking badge (four LEDs, colors/positions matching the Xbox controller), based on the ATMEGA 328PB chip. Has a Sleep button and a Mode button (to change blink patterns). Wants a 3v coin battery (2025/2032), and uses a connector off the programmer from Tag-Connect. Only has SMD components (so no THT on the back to catch on clothing, it's a badge to be worn). 

My coding is mostly Python and R scripts (linear), and I learned just before event-loop programming took hold, so my code works but I have been told it is a bit odd and there are better ways to do it. Next time!

Should have enough files for...
Badge - shape, art, compnents and layout (so the schematic, layout, and program files). 
BOM - Bill Of Materials (the parts to order).
Code - Arduino / C++ code.

None of this would have happened without a lot of helpful people - posts to forums about schematics and code, sample working schematics (I copied the Arduino Uno schematic to start), some help from friends and relatives who know this material, and help from some people on the Arduino forum about the sleep code for the chip. 
