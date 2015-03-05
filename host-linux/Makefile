EXE = out/chrome-token-signing

PKCS11_MODULE := opensc-pkcs11.so
COPT = -O2
CXX = g++
INCLUDES := -I. -I../host-shared -I../host-shared/json -I../host-shared/pkcs11  `pkg-config --cflags openssl` `pkg-config --cflags gtk+-3.0` `pkg-config --cflags gtkmm-3.0`
LIBS := -ldl -pthread `pkg-config --libs gtk+-3.0` `pkg-config --libs gtkmm-3.0` `pkg-config --libs openssl`
CPPFLAGS := $(COPT) -g $(INCLUDES) -pthread -D_GLIBCXX_USE_NANOSLEEP -std=c++11 -DPKCS11_MODULE=\"$(PKCS11_MODULE)\"
LDFLAGS := $(LIBPATH) -g $(LIBS)


SOURCES = CertDialog.cpp chrome-host.cpp PinDialog.cpp
SOURCES += ../host-shared/BinaryUtils.cpp ../host-shared/InputParser.cpp ../host-shared/locked_allocator.cpp ../host-shared/DateUtils.cpp ../host-shared/Labels.cpp ../host-shared/Logger.cpp


OBJS = $(patsubst %.cpp,%.o, $(SOURCES)) ../host-shared/json/jsonxx.o

all: $(JSON_OBJS) $(OBJS)
	mkdir -p out
	@echo Linking...
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $(EXE)
	
clean:
	rm -rf $(EXE) *.o ../host-shared/*.o

install:
	cp $(EXE) /usr/bin
	test -d /opt/google/chrome && mkdir -p /etc/opt/chrome/native-messaging-hosts
	test -d /opt/google/chrome && cp ee.ria.esteid.json /etc/opt/chrome/native-messaging-hosts
	test -d /opt/google/chrome && mkdir -p /opt/google/chrome/extensions
	test -d /opt/google/chrome && cp ../ckjefchnfjhjfedoccjbhjpbncimppeg.json /opt/google/chrome/extensions

uninstall:
	rm -f /opt/google/chrome/extensions/ckjefchnfjhjfedoccjbhjpbncimppeg.json
	rm -f /etc/opt/chrome/native-messaging-hosts/ee.ria.esteid.json
	rm -f /usr/bin/chrome-token-signing
