ifeq ($(BUILD_NUMBER),)
BUILD_NUMBER = 0
endif
include ../VERSION.mk

all:
	qmake -qt=5 VERSION=$(VERSIONEX)
	make -f Makefile

clean:
	make -f Makefile distclean

install:
	make -f Makefile install

uninstall:
	make -f Makefile uninstall
