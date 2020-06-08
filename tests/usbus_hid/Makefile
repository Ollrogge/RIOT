BOARD ?= nrf52840dk

include ../Makefile.tests_common

USEMODULE += usbus
USEMODULE += stdio_hid

USB_VID = 1915
USB_PID = 521f

CFLAGS += -DCONFIG_USB_VID=0x$(USB_VID)
CFLAGS += -DCONFIG_USB_PID=0x$(USB_PID)

include $(RIOTBASE)/Makefile.include