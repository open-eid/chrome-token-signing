TEMPLATE = app
CONFIG += console c++11 link_pkgconfig
CONFIG -= app_bundle
QT += widgets
VERSION = 1.0.0
PKGCONFIG += openssl
INCLUDEPATH += ../host-shared/json ../host-shared
LIBS += -ldl
DEFINES += VERSION=\\\"$$VERSION\\\"
SOURCES += \
    ../host-shared/BinaryUtils.cpp \
    ../host-shared/DateUtils.cpp \
    ../host-shared/Labels.cpp \
    ../host-shared/locked_allocator.cpp \
    ../host-shared/Logger.cpp \
    ../host-shared/json/jsonxx.cc \
    chrome-host.cpp
HEADERS += *.h ../host-shared/*.h ../host-shared/json/*.h
