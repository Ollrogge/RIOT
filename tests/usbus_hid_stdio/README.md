Problem
===============

An error occurs before python shell is attached, therefore we never enter main.

The initialization code in auto_init_usb.c, which runs before main, is also not reached.
