TEMPLATE = app
CONFIG += c++17 qt console
QT += core network
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += FactorioWebApi/

include(FactorioWebApi/FactorioWebApi.pri)

SOURCES += \
    main.cpp
HEADERS += \
    main.h


unix {
    target.path = /usr/bin
}
!isEmpty(target.path): INSTALLS += target
