
QT += gui
QT += widgets
QT += sql
QT += printsupport
QT += network
QT += svg
QT += xml


LIBS += -lpoppler-qt5
LIBS += -lpoppler
LIBS += -lzip

windows {
    LIBS += -lopenjp2
    LIBS += -lbcrypt
}


macx {
    LIBS += -L/usr/local/opt/poppler/lib -lpoppler-qt5
    LIBS += -L/usr/local/opt/libzip -lzip
    INCLUDEPATH += /usr/local/include
}

CONFIG += c++14

TARGET = kitsas

TEMPLATE = app

include(sources.pri)

SOURCES += main.cpp

DISTFILES += \
    uusikp/luo.sql \
    aloitussivu/qrc/avaanappi.png \
    aloitussivu/qrc/aloitus.css \
    uusikp/update3.sql

TRANSLATIONS = tr/kitsas_en.ts \
               tr/kitsas_sv.ts

RC_ICONS = kitupiikki.ico




















