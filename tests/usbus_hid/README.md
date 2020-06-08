Expected behavior
===============

After flashing the device executing the command:

```
usb-devices
```

should show the RIOT device with the product string "Usb Hid Test Device". The driver should be usbhid, indicating that the host has successfully recognized the device.

Following is a screenshot of the expected output:

![Alt text](usb-devices.png?raw=true)

The output of the command

```
dmesg
```

should also contain logs stating that a new USB HID device was found.

Following is a screenshot of the expected output:

![Alt text](dmesg.png?raw=true)


Problem
===============

As can be seen in the screenshot above, the HID device is called hidraw6.
Technically we should be able to send input to the device.  For example:

```
echo "Test" /dev/hidraw6
```

### Expected behavior
```
bash: echo: write error: Connection timed out
```

### Correct behavior
The printf statement in the main function gets triggered stating how many bytes we received over the USB HID interface.

