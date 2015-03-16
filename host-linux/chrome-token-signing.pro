TEMPLATE = app
CONFIG += console c++11 link_pkgconfig
CONFIG -= app_bundle
QT += widgets network
VERSION = 1.0.0
PKGCONFIG += openssl
INCLUDEPATH += ../host-shared
LIBS += -ldl
DEFINES += VERSION=\\\"$$VERSION\\\"
SOURCES += \
    ../host-shared/BinaryUtils.cpp \
    ../host-shared/Labels.cpp \
    ../host-shared/Logger.cpp \
    chrome-host.cpp
HEADERS += *.h ../host-shared/*.h
target.path = /usr/bin
hostconf.path = /etc/opt/chrome/native-messaging-hosts
hostconf.files += ee.ria.esteid.json
extension.path = /opt/google/chrome/extensions
extension.files += ../ckjefchnfjhjfedoccjbhjpbncimppeg.json
INSTALLS += target hostconf extension
