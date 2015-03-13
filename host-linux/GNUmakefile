all:
	qmake -qt=5
	make -f Makefile

clean:
	make -f Makefile distclean

install:
	cp chrome-token-signing /usr/bin
	test -d /opt/google/chrome && mkdir -p /etc/opt/chrome/native-messaging-hosts
	test -d /opt/google/chrome && cp ee.ria.esteid.json /etc/opt/chrome/native-messaging-hosts
	test -d /opt/google/chrome && mkdir -p /opt/google/chrome/extensions
	test -d /opt/google/chrome && cp ../ckjefchnfjhjfedoccjbhjpbncimppeg.json /opt/google/chrome/extensions

uninstall:
	rm -f /opt/google/chrome/extensions/ckjefchnfjhjfedoccjbhjpbncimppeg.json
	rm -f /etc/opt/chrome/native-messaging-hosts/ee.ria.esteid.json
	rm -f /usr/bin/chrome-token-signing
