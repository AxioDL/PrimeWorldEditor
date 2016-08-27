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
            -L$$EXTERNALS_DIR/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-gd-1_56 \
            -L$$EXTERNALS_DIR/tinyxml2/lib/ -ltinyxml2d

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
            -L$$EXTERNALS_DIR/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-1_56 \
            -L$$EXTERNALS_DIR/tinyxml2/lib/ -ltinyxml2

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIO.lib
    }
}

# Debug/Release Libs
LIBS += -L$$EXTERNALS_DIR/lzo-2.08/lib -llzo-2.08 \
        -L$$EXTERNALS_DIR/zlib/lib -lzdll

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/boost_1_56_0 \
               $$EXTERNALS_DIR/lzo-2.08/include \
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
