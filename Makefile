
OUTPUT_PATH := out/
SOURCE_PATH := host/
JSON_SOURCE_PATH := json/
TEST_PATH := test/
EXE := $(OUTPUT_PATH)chrome-token-signing
TEST_EXE := $(EXE)-test

CPP := g++
CFLAGS := -O2
CXXFLAGS := $(CFLAGS) -g -std=c++0x -pthread -D_GLIBCXX_USE_NANOSLEEP
CXXFLAGS += -Igooglemock/include -Igtest/include -Ihost -Ijson -Ipkcs11 -I/usr/include/openssl 
CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0)
CXXFLAGS += $(shell pkg-config --cflags gtkmm-3.0)
LDFLAGS := -g -ldl
LDFLAGS += $(shell pkg-config --libs gtk+-3.0)
LDFLAGS += $(shell pkg-config --libs gtkmm-3.0)
LDFLAGS += $(shell pkg-config --libs openssl)
GTEST_DIR := gtest
GMOCK_DIR := googlemock

test: PP = -D_TEST

OBJS := $(patsubst %.cpp,%.o,$(wildcard $(SOURCE_PATH)*.cpp))
JSON_OBJS := $(patsubst %.cc,%.o,$(wildcard $(JSON_SOURCE_PATH)*.cc))
JSON_OBJS := $(filter-out $(JSON_SOURCE_PATH)jsonxx_test.o, $(JSON_OBJS))	
TEST_OBJS := $(patsubst %.cpp,%.o,$(wildcard $(TEST_PATH)*.cpp))

%.o:%.cpp
	@echo Compiling $(basename $<)...
	$(CPP) -c $(CXXFLAGS) $(PP) $< -o $@

$(EXE): $(JSON_OBJS) $(OBJS)
	mkdir -p $(OUTPUT_PATH)
	$(CPP) $(LDFLAGS) $^ $(LIBS) -o $@
	
test: $(JSON_OBJS) $(filter-out $(SOURCE_PATH)chrome-host.o, $(OBJS)) $(TEST_OBJS)
	$(CPP) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	$(CPP) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} -pthread -c ${GMOCK_DIR}/src/gmock-all.cc
	ar -rv libgmock.a gtest-all.o gmock-all.o
	mkdir -p $(OUTPUT_PATH)
	$(CPP) -isystem ${GMOCK_DIR}/include -isystem ${GTEST_DIR}/include -pthread \
	$(LDFLAGS) $^ libgmock.a $(LIBS) -o $(TEST_EXE)
	$(TEST_EXE) --gtest_output=xml:test_report.xml
	
clean:
	rm -rfv $(SOURCE_PATH)*.o $(EXE) $(TEST_PATH)*.o $(JSON_SOURCE_PATH)*.o *.o
	rm -rfv googlemock/make/g*

install:
	install -d $(DESTDIR)/usr/bin
	install -d $(DESTDIR)/usr/share/chrome-token-signing/native-messaging-hosts/
	install -d $(DESTDIR)/usr/share/chrome-token-signing/policies/managed/

	install -m755 $(EXE) $(DESTDIR)/usr/bin
	install -m644 ee.ria.esteid.json $(DESTDIR)/usr/share/chrome-token-signing/native-messaging-hosts/
	install -m644 esteid_policy.json $(DESTDIR)/usr/share/chrome-token-signing/policies/managed/
	install -m644 chrome-token-signing.crx $(DESTDIR)/usr/share/chrome-token-signing/
	install -m644 update.xml $(DESTDIR)/usr/share/chrome-token-signing/

	# For Google Chrome
	install -d $(DESTDIR)/etc/opt/chrome/native-messaging-hosts/
	install -d $(DESTDIR)/etc/opt/chrome/policies/managed/
	ln -s /usr/share/chrome-token-signing/native-messaging-hosts/ee.ria.esteid.json \
		$(DESTDIR)/etc/opt/chrome/native-messaging-hosts/ee.ria.esteid.json
	ln -s /usr/share/chrome-token-signing/policies/managed/esteid_policy.json \
		$(DESTDIR)/etc/opt/chrome/policies/managed/esteid_policy.json

	# For Chromium on Debian
	install -d $(DESTDIR)/etc/chromium/native-messaging-hosts/
	install -d $(DESTDIR)/etc/chromium/policies/managed/
	ln -s /usr/share/chrome-token-signing/native-messaging-hosts/ee.ria.esteid.json \
		$(DESTDIR)/etc/chromium/native-messaging-hosts/ee.ria.esteid.json
	ln -s /usr/share/chrome-token-signing/policies/managed/esteid_policy.json \
		$(DESTDIR)/etc/chromium/policies/managed/esteid_policy.json

	# For Chromium on Ubuntu
	install -d $(DESTDIR)/etc/chromium-browser/native-messaging-hosts/
	install -d $(DESTDIR)/etc/chromium-browser/policies/managed/
	ln -s /usr/share/chrome-token-signing/native-messaging-hosts/ee.ria.esteid.json \
		$(DESTDIR)/etc/chromium-browser/native-messaging-hosts/ee.ria.esteid.json
	ln -s /usr/share/chrome-token-signing/policies/managed/esteid_policy.json \
		$(DESTDIR)/etc/chromium-browser/policies/managed/esteid_policy.json
