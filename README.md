Summary
------------
I'm working on a PS/2 mouse/keyboard driver for my little AVR chip here.
I want it so I can use an optical mouse to track position of a robot, 
with the aid of a gyroscope.

Relevant docs
---------------------

- [PS/2 interface](http://www.computer-engineering.org/ps2protocol/)
- [AVR libc](http://www.nongnu.org/avr-libc/user-manual/index.html)
- [My hardware](http://eecs.oregonstate.edu/education/hardware/wunder.1/)
- [MarkDown syntax](http://daringfireball.net/projects/markdown/)



File descriptions
-----------

###test.c
This is just a program to test what I can on my computer, where it's
easier to debug. I used it to test the queue, though that code is
now deleted.

###avr_sub.h
This header allows me to test the queue on my computer, ignoring the
interrupt enable/disable stuff. (atomic execution)
I can't, of course, test anything involving hardware registers on my
computer.

###driver_def.h
This contains definitions for variable types and methods. There should be
no global variables defined here.

###driver_int.h
This contains the code for the ISRs (interrupt service routines).
There are global variables defined here, because that is the only
way to pass data in and out of the interrupt. If these variables
are changed inside the interrupt, they must be volatile, or the 
compiler will optimize them out.
