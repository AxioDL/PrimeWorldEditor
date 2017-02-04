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
DESTDIR = $$BUILD_DIR/Common

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/Common/debug
    TARGET = Commond

    # Debug Libs
    LIBS += -L$$BUILD_DIR/FileIO/ -lFileIOd \
            -L$$EXTERNALS_DIR/boost_1_63_0/lib64-msvc-14.0 -llibboost_filesystem-vc140-mt-gd-1_63 \
            -L$$EXTERNALS_DIR/lzo-2.09/lib -llzo2d \
            -L$$EXTERNALS_DIR/tinyxml2/lib -ltinyxml2d \
            -L$$EXTERNALS_DIR/zlib/lib -lzlibd

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIOd.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/build/Common/release
    TARGET = Common

    # Release Libs
    LIBS += -L$$BUILD_DIR/FileIO/ -lFileIO \
            -L$$EXTERNALS_DIR/boost_1_63_0/lib64-msvc-140 -llibboost_filesystem-vc140-mt-1_63 \
            -L$$EXTERNALS_DIR/lzo-2.09/lib -llzo2 \
            -L$$EXTERNALS_DIR/tinyxml2/lib -ltinyxml2 \
            -L$$EXTERNALS_DIR/zlib/lib -lzlib

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIO.lib
    }
}

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/boost_1_63_0 \
               $$EXTERNALS_DIR/lzo-2.09/include \
               $$EXTERNALS_DIR/tinyxml2/include \
               $$EXTERNALS_DIR/zlib/include

# Header Files
HEADERS += \
    CColor.h \
    CFourCC.h \
    CHashFNV1A.h \
    CompressionUtil.h \
    CTimer.h \
    EKeyInputs.h \
    EMouseInputs.h \
    ETransformSpace.h \
    Flags.h \
    TString.h \
    types.h \
    Log.h \
    FileUtil.h \
    AssertMacro.h \
    CScopedTimer.h \
    CAssetID.h \
    Serialization/IArchive.h \
    Serialization/CXMLWriter.h \
    Serialization/CXMLReader.h \
    EGame.h \
    Serialization/CBasicBinaryWriter.h \
    Serialization/CBasicBinaryReader.h \
    Serialization/CBinaryWriter.h \
    Serialization/CBinaryReader.h \
    Serialization/CSerialVersion.h \
    Serialization/XML.h \
    Serialization/Binary.h

# Source Files
SOURCES += \
    CColor.cpp \
    CompressionUtil.cpp \
    CTimer.cpp \
    TString.cpp \
    Log.cpp \
    FileUtil.cpp \
    CAssetID.cpp \
    EGame.cpp \
    Serialization/CSerialVersion.cpp
