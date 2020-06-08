MODULE = usbus_hid
SRC = hid.c

ifneq (,$(filter stdio_hid,$(USEMODULE)))
  SRC += hid_stdio.c
endif

include $(RIOTBASE)/Makefile.base
