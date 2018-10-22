#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:27:18
#
#-------------------------------------------------

QT -= core gui
DEFINES += PWE_COMMON

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$BUILD_DIR/Common

unix {
    target.path = /usr/lib
    QMAKE_CXXFLAGS += /WX
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/Common/debug
    TARGET = Commond
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/build/Common/release
    TARGET = Common
}

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/CodeGen/include \
               $$EXTERNALS_DIR/tinyxml2

# Header Files
HEADERS += \
    CColor.h \
    CFourCC.h \
    CTimer.h \
    EKeyInputs.h \
    EMouseInputs.h \
    Flags.h \
    TString.h \
    types.h \
    Log.h \
    FileUtil.h \
    AssertMacro.h \
    CScopedTimer.h \
    CAssetID.h \
    Hash\CFNV1A.h \
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
    Serialization/Binary.h \
    FileIO\CFileInStream.h \
    FileIO\CFileOutStream.h \
    FileIO\CMemoryInStream.h \
    FileIO\CMemoryOutStream.h \
    FileIO\CTextInStream.h \
    FileIO\CTextOutStream.h \
    FileIO\CVectorOutStream.h \
    FileIO\IOUtil.h \
    FileIO\IInputStream.h \
    FileIO\IOutputStream.h \
    FileIO\CBitStreamInWrapper.h \
    FileIO\CFileLock.h \
    FileIO.h \
    Common.h \
    Hash/CCRC32.h \
    NBasics.h

# Source Files
SOURCES += \
    CColor.cpp \
    CTimer.cpp \
    TString.cpp \
    Log.cpp \
    FileUtil.cpp \
    CAssetID.cpp \
    EGame.cpp \
    Serialization/CSerialVersion.cpp \
    FileIO\CFileInStream.cpp \
    FileIO\CFileOutStream.cpp \
    FileIO\CMemoryInStream.cpp \
    FileIO\CMemoryOutStream.cpp \
    FileIO\CTextInStream.cpp \
    FileIO\CTextOutStream.cpp \
    FileIO\CVectorOutStream.cpp \
    FileIO\IOUtil.cpp \
    FileIO\IInputStream.cpp \
    FileIO\IOutputStream.cpp \
    FileIO\CBitStreamInWrapper.cpp \
    Hash/CCRC32.cpp

# Codegen
CODEGEN_DIR = $$EXTERNALS_DIR/CodeGen
CODEGEN_OUT_PATH = $$BUILD_DIR/Common/codegen_build/auto_codegen.cpp
CODEGEN_SRC_PATH = $$PWD
include($$EXTERNALS_DIR/CodeGen/codegen.pri)

# Library Sources
SOURCES += $$EXTERNALS_DIR/tinyxml2/tinyxml2.cpp
