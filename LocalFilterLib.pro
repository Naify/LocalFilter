#-------------------------------------------------
#
# Project created by QtCreator 2019-10-28T13:52:06
#
#-------------------------------------------------

QT       += gui

TARGET = LocalFilterLib
TEMPLATE = lib
DESTDIR = ./lib
UI_DIR = ./src

DEFINES += LOCALFILTER_LIB

SOURCES += \
    src/localfilter.cpp \
    src/addeditfilter.cpp

HEADERS += \
    src/localfilter.h \
    src/addeditfilter.h

FORMS += \
    src/localfilter.ui \
    src/addeditfilter.ui

INCLUDEPATH += /usr/local/ods/include \
               /usr/local/metaManager/include \
               /usr/local/irs/include

LIBS += -L/usr/local/ods/lib -lOdsInterface \
        -L/usr/local/metaManager/lib -lMetaManager \
        -L/usr/local/irs/lib -lirs

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG(debug, release) {
        # Debug
        unix:TARGET = $$join(TARGET,,,-debug)
        else:TARGET = $$join(TARGET,,,d)
        OBJECTS_DIR = ./build/debug/.obj
        MOC_DIR = ./build/debug/.moc
} else {
        # Release
        OBJECTS_DIR = ./build/release/.obj
        MOC_DIR = ./build/release/.moc
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    LIBS+= -lqjson-qt5
    INCLUDEPATH += /usr/local/include/qjson-qt5
} else {
    LIBS+= -lqjson
    INCLUDEPATH += /usr/local/include/qjson
}
