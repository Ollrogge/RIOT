# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

APPLICATION = example_psa_crypto

BOARD ?= native

ifeq (1, $(SECURE_ELEMENT))
	CFLAGS += -DSECURE_ELEMENT
	CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(8*THREAD_STACKSIZE_DEFAULT\)
	KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.se
else
	ifeq (1, $(MULTIPLE_SE))
		CFLAGS += -DMULTIPLE_SE
		CFLAGS += -DSECURE_ELEMENT
		CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(12*THREAD_STACKSIZE_DEFAULT\)
		KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.se.multi
	else
		ifeq (1, $(SOFTWARE))
			KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.sw
		else
			CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(12*THREAD_STACKSIZE_DEFAULT\)
			KCONFIG_ADD_CONFIG += $(APPDIR)/app.config.test.auto
		endif
	endif
endif

export TEST_KCONFIG ?= 1

include $(RIOTBASE)/Makefile.include
