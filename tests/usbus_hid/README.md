Expected result
===============

After flashing the device executing the command:

```
dmesg
```

should contain logs stating that a new USB HID device was found.

Following is a screenshot of the expected output:

![Alt text](dmesg.png?raw=true)

Note, that the endpoint (in this case hidraw6) might differ.

After successful initialization of USB HID one should be able to communicate
via USB HID simply by echoing input to the device.

Based on the screenshot above, an example command:

```
echo "Test" > /dev/hidraw6
```

The input string "Test" should be read by the test application and
printed to stdout.