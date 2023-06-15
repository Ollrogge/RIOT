# name of your application
APPLICATION = fuck

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Set this to 1 to enable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 0

USEMODULE += sock_dns
USEMODULE += sock_udp
USEMODULE += gnrc_ipv6
USEMODULE += auto_init_sock_dns

DISABLE_MODULE += cortexm_fpu


include $(RIOTBASE)/Makefile.include
