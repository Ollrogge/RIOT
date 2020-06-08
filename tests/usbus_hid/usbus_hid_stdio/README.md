Problem
===============

Registering usb_hid_stdio as stdio interface in `stdio.inc.mk` results in an error.

Code never reaches main.

The initialization code in auto_init_usb.c, which runs before main, is also not reached.


Correct behavior
===============
auto_init_usb.c should initialize the USB HID module via the hid_stdio.c file.
Code execution should reach main function.