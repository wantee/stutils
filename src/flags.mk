CC = gcc

OUTINC_DIR = ../include/
OUTLIB_DIR = ../lib
OUTBIN_DIR = ..

OBJ_DIR = ./.objs
DEP_DIR = ./.deps

CFLAGS += -Wall -pipe -g -m64
CFLAGS += -march=native -mtune=native -O3
CPPFLAGS += -I. -I$(OUTINC_DIR)
CPPFLAGS += -DNDEBUG
#CFLAGS += -pg
#LDFLAGS += -pg
ifeq ($(shell uname -s),Darwin)
LDFLAGS+=
else
LDFLAGS+=-lm
endif
LDFLAGS += -lpthread
