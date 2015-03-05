UNAME = `uname`

detect:
	make $(UNAME)

Linux:
	make -C host-linux

Darwin:
	make -C host-osx

