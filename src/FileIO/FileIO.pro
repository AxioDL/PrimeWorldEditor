#-------------------------------------------------
#
# Project created by QtCreator 2015-12-16T12:35:06
#
#-------------------------------------------------

QT       -= core gui
DEFINES += PWE_FILEIO

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$BUILD_DIR/FileIO

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/FileIO/debug
    TARGET = FileIOd
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/FileIO/release
    TARGET = FileIO
}

# Header Files
HEADERS += \
    CFileInStream.h \
    CFileOutStream.h \
    CMemoryInStream.h \
    CMemoryOutStream.h \
    CTextInStream.h \
    CTextOutStream.h \
    CVectorOutStream.h \
    FileIO.h \
    IOUtil.h \
    IInputStream.h \
    IOutputStream.h \
    CBitStreamInWrapper.h \
    CFileLock.h

# Source Files
SOURCES += \
    CFileInStream.cpp \
    CFileOutStream.cpp \
    CMemoryInStream.cpp \
    CMemoryOutStream.cpp \
    CTextInStream.cpp \
    CTextOutStream.cpp \
    CVectorOutStream.cpp \
    IOUtil.cpp \
    IInputStream.cpp \
    IOutputStream.cpp \
    CBitStreamInWrapper.cpp
