#-------------------------------------------------
#
# Project created by QtCreator 2015-12-16T12:35:06
#
#-------------------------------------------------

QT       -= core gui
QMAKE_CXXFLAGS += /WX

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$PWD/../../build/FileIO

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$PWD/../../build/FileIO/debug
    TARGET = FileIOd
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/FileIO/release
    TARGET = FileIO
}

# Header Files
HEADERS += \
    CFileInStream.h \
    CFileOutStream.h \
    CInputStream.h \
    CMemoryInStream.h \
    CMemoryOutStream.h \
    COutputStream.h \
    CTextInStream.h \
    CTextOutStream.h \
    CVectorOutStream.h \
    FileIO.h \
    IOUtil.h

# Source Files
SOURCES += \
    CFileInStream.cpp \
    CFileOutStream.cpp \
    CInputStream.cpp \
    CMemoryInStream.cpp \
    CMemoryOutStream.cpp \
    COutputStream.cpp \
    CTextInStream.cpp \
    CTextOutStream.cpp \
    CVectorOutStream.cpp \
    IOUtil.cpp
