TEMPLATE = app
CONFIG += console c++11 link_pkgconfig
CONFIG -= app_bundle
QT += widgets network
isEmpty(VERSION):VERSION=1.0.8.0
PKGCONFIG += libpcsclite
INCLUDEPATH += ../host-shared
LIBS += -ldl
DEFINES += VERSION=\\\"$$VERSION\\\"
SOURCES += \
    ../host-shared/Labels.cpp \
    ../host-shared/Logger.cpp \
    ../host-shared/BinaryUtils.cpp \
    ../host-shared/PKCS11Path.cpp \
    chrome-host.cpp
HEADERS += *.h ../host-shared/*.h
RESOURCES += chrome-token-signing.qrc
isEmpty(LIBPATH):LIBPATH=/usr/lib
target.path = /usr/bin
hostconf.path = /etc/opt/chrome/native-messaging-hosts
hostconf.files += ee.ria.esteid.json
# https://bugzilla.mozilla.org/show_bug.cgi?id=1318461
ffconf.path = $${LIBPATH}/mozilla/native-messaging-hosts
ffconf.files += ff/ee.ria.esteid.json
extension.path = /opt/google/chrome/extensions
extension.files += ../ckjefchnfjhjfedoccjbhjpbncimppeg.json
extensionpol.path = /etc/opt/chrome/policies/managed
extensionpol.files += ee.ria.chrome-token-signing.policy.json
ffextension.path = /usr/share/mozilla/extensions/{ec8030f7-c20a-464f-9b0e-13a3a9e97384}
ffextension.files += ../{443830f0-1fff-4f9a-aa1e-444bafbc7319}.xpi
INSTALLS += target hostconf ffconf extension ffextension
