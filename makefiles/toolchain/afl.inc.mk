#include $(RIOTMAKE)/toolchain/gnu.inc.mk
include $(RIOTMAKE)/toolchain/llvm.inc.mk

#CC     = $(PREFIX)afl-gcc
#CXX    = $(PREFIX)afl-g++
#LINK   = $(PREFIX)afl-gcc
#LINKXX = $(PREFIX)afl-g++

CC     = $(PREFIX)afl-clang-fast
CXX    = $(PREFIX)afl-clang-fast++
LINK   = $(PREFIX)afl-clang-fast
LINKXX = $(PREFIX)afl-clang-fast++
