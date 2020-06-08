Expected behavior
===============

After flashing the device executing the command: "usb-devices" ,should show the
RIOT device with the product string "Usb Hid Test Device". The driver should
be usbhid, indicating that the host has successfully recognized the device.
Following is a screenshot of the expected output:

![Alt text](usb-devices.png?raw=true))

Dmesg should also contain logs stating that a new USB HID device was found.
Following is a screenshot of the expected output:

![Alt text](dmesg.png?raw=true))
