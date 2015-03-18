ifeq ($(BUILD_NUMBER),)
BUILD_NUMBER = 0
endif
VERSION = 1.0.0.$(BUILD_NUMBER)

all:
	qmake -qt=5 VERSION=$(VERSION)
	make -f Makefile

clean:
	make -f Makefile distclean

install:
	make -f Makefile install

uninstall:
	make -f Makefile uninstall
