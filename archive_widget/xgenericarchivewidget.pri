INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xgenericarchivewidget.h

SOURCES += \
    $$PWD/xgenericarchivewidget.cpp

FORMS += \
    $$PWD/xgenericarchivewidget.ui

!contains(XCONFIG, use_archive) {
    XCONFIG += use_archive
}

!contains(XCONFIG, xarchives) {
    XCONFIG += xarchives
    include($$PWD/../XArchive/xarchives.pri)
}

!contains(XCONFIG, xshortcuts) {
    XCONFIG += xshortcuts
    include($$PWD/../XShortcuts/xshortcuts.pri)
}

DISTFILES += \
    $$PWD/xgenericarchivewidget.cmake
