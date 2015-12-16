#-------------------------------------------------
#
# Project created by QtCreator 2015-12-16T12:35:34
#
#-------------------------------------------------

QT       -= core gui
QMAKE_CXXFLAGS += /WX
DEFINES += PWE_MATH

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$PWD/../../build/Math

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$PWD/../../build/Math/debug
    TARGET = Mathd

    # Debug Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIOd \
            -L$$PWD/../../build/Common/ -lCommond

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIOd.lib \
                          $$PWD/../../build/Common/Commond.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/Math/release
    TARGET = Math

    # Release Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIO \
            -L$$PWD/../../build/Common/ -lCommon

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIO.lib \
                          $$PWD/../../build/Common/Common.lib
    }
}

# Debug/Release Libs
LIBS += -L$$PWD/../../externals/lzo-2.08/lib -llzo-2.08 \
        -L$$PWD/../../externals/zlib/lib -lzdll

# Include Paths
INCLUDEPATH += $$PWD/../ \
               $$PWD/../../externals/glm/glm \
               $$PWD/../../externals/lzo-2.08/include \
               $$PWD/../../externals/zlib/include

# Header Files
HEADERS += \
    CAABox.h \
    CFrustumPlanes.h \
    CMatrix4f.h \
    CPlane.h \
    CQuaternion.h \
    CRay.h \
    CTransform4f.h \
    CVector2f.h \
    CVector2i.h \
    CVector3f.h \
    CVector4f.h \
    ETransformSpace.h \
    MathUtil.h

# Source Files
SOURCES += \
    CAABox.cpp \
    CFrustumPlanes.cpp \
    CMatrix4f.cpp \
    CPlane.cpp \
    CQuaternion.cpp \
    CRay.cpp \
    CTransform4f.cpp \
    CVector2f.cpp \
    CVector2i.cpp \
    CVector3f.cpp \
    CVector4f.cpp \
    MathUtil.cpp
