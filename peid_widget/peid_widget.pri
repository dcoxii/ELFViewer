INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/peid_widget.h

SOURCES += \
    $$PWD/peid_widget.cpp

!contains(XCONFIG, xpeid) {
    XCONFIG += xpeid
    include($$PWD/../XPEID/xpeid.pri)
}

!contains(XCONFIG, xscanwidgets) {
    XCONFIG += xscanwidgets
    include($$PWD/../XScanEngine/xscanwidgets.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/peid_widget.cmake
