include ../Makefile.fuzzing_common
# name of your application
APPLICATION=dns_afl

# Set this to 1 to enable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 0

USEMODULE += sock_dns
USEMODULE += sock_udp
USEMODULE += gnrc_ipv6

 #Needed when using afl++ and persistent fuzzing
CFLAGS += -Wno-gnu-statement-expression
CFLAGS += -Wno-pedantic


include $(RIOTBASE)/Makefile.include
