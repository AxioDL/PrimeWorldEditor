#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:27:18
#
#-------------------------------------------------

QT -= core gui
QMAKE_CXXFLAGS += /WX

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
    LIBS += -L$$PWD/../../externals/FileIO/lib/ -lFileIOd
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/Common/release
    TARGET = Common

    # Release Libs
    LIBS += -L$$PWD/../../externals/FileIO/lib/ -lFileIO
}

# Debug/Release Libs
LIBS += -L$$PWD/../../externals/lzo-2.08/lib -llzo-2.08 \
        -L$$PWD/../../externals/zlib/lib -lzdll \

# Include Paths
INCLUDEPATH += $$PWD/.. \
               $$PWD/../../externals/FileIO/include \
               $$PWD/../../externals/glm/glm \
               $$PWD/../../externals/lzo-2.08/include \
               $$PWD/../../externals/zlib/include

# Source Files
HEADERS += \
    AnimUtil.h \
    CColor.h \
    CFourCC.h \
    CHashFNV1A.h \
    CompressionUtil.h \
    CTimer.h \
    CUniqueID.h \
    EKeyInputs.h \
    EMouseInputs.h \
    EnumUtil.h \
    ETransformSpace.h \
    TString.h \
    types.h \
    Math/CAABox.h \
    Math/CFrustumPlanes.h \
    Math/CMatrix4f.h \
    Math/CPlane.h \
    Math/CQuaternion.h \
    Math/CRay.h \
    Math/CTransform4f.h \
    Math/CVector2f.h \
    Math/CVector2i.h \
    Math/CVector3f.h \
    Math/CVector4f.h \
    Math/Math.h

SOURCES += \
    AnimUtil.cpp \
    CColor.cpp \
    CFourCC.cpp \
    CHashFNV1A.cpp \
    CompressionUtil.cpp \
    CTimer.cpp \
    CUniqueID.cpp \
    TString.cpp \
    Math/CAABox.cpp \
    Math/CFrustumPlanes.cpp \
    Math/CMatrix4f.cpp \
    Math/CPlane.cpp \
    Math/CQuaternion.cpp \
    Math/CRay.cpp \
    Math/CTransform4f.cpp \
    Math/CVector2f.cpp \
    Math/CVector2i.cpp \
    Math/CVector3f.cpp \
    Math/CVector4f.cpp \
    Math/Math.cpp
