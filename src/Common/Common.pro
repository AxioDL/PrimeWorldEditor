#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:27:18
#
#-------------------------------------------------

QT -= core gui
QMAKE_CXXFLAGS += /WX
DEFINES += PWE_COMMON

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$PWD/../../build/Common

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$PWD/../../build/Common/debug
    TARGET = Commond

    # Debug Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIOd

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIOd.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/Common/release
    TARGET = Common

    # Release Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIO

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIO.lib
    }
}

# Debug/Release Libs
LIBS += -L$$PWD/../../externals/lzo-2.08/lib -llzo-2.08 \
        -L$$PWD/../../externals/zlib/lib -lzdll

# Include Paths
INCLUDEPATH += $$PWD/.. \
               $$PWD/../../externals/lzo-2.08/include \
               $$PWD/../../externals/zlib/include

# Header Files
HEADERS += \
    CColor.h \
    CFourCC.h \
    CHashFNV1A.h \
    CompressionUtil.h \
    CTimer.h \
    CUniqueID.h \
    EKeyInputs.h \
    EMouseInputs.h \
    ETransformSpace.h \
    Flags.h \
    TString.h \
    types.h \
    Log.h

# Source Files
SOURCES += \
    CColor.cpp \
    CompressionUtil.cpp \
    CTimer.cpp \
    CUniqueID.cpp \
    TString.cpp \
    Log.cpp
