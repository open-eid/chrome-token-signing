UNAME = `uname`

detect:
	make $(UNAME)

Linux:
	/usr/bin/google-chrome --pack-extension=extension --pack-extension-key=development-key.pem
	make -C host-linux

Darwin:
	/Applications/Google\ Chrome.app/Contents/MacOS/Google\ Chrome --pack-extension=extension --pack-extension-key=development-key.pem
	make -C host-osx

clean:
	rm *.crx
