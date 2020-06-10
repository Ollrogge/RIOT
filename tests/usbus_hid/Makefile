BOARD ?= nrf52840dk

include ../Makefile.tests_common

USEMODULE += usbus
USEMODULE += usbus_hid
USEMODULE += stdio_hid
USEMODULE += xtimer
USEMODULE += isrpipe

DISABLE_MODULE += auto_init_usbus

USB_VID = 1915
USB_PID = 521f
PRODUCT_STRING = Usb Hid Test Device

CFLAGS += -DCONFIG_USB_VID=0x$(USB_VID)
CFLAGS += -DCONFIG_USB_PID=0x$(USB_PID)
CFLAGS += -DCONFIG_USB_PRODUCT_STR='"$(PRODUCT_STRING)"'

include $(RIOTBASE)/Makefile.include