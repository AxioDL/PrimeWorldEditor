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
DESTDIR = $$BUILD_DIR/Math

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/Math/debug
    TARGET = Mathd

    # Debug Libs
    LIBS += -L$$BUILD_DIR/FileIO/ -lFileIOd \
            -L$$BUILD_DIR/Common/ -lCommond \
            -L$$EXTERNALS_DIR/lzo-2.09/lib/ -llzo2d \
            -L$$EXTERNALS_DIR/tinyxml2/lib/ -ltinyxml2d \
            -L$$EXTERNALS_DIR/zlib/lib/ -lzlibd

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIOd.lib \
                          $$BUILD_DIR/Common/Commond.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/Math/release
    TARGET = Math

    # Release Libs
    LIBS += -L$$BUILD_DIR/FileIO/ -lFileIO \
            -L$$BUILD_DIR/Common/ -lCommon \
            -L$$EXTERNALS_DIR/lzo-2.09/lib/ -llzo2 \
            -L$$EXTERNALS_DIR/tinyxml2/lib/ -ltinyxml2 \
            -L$$EXTERNALS_DIR/zlib/lib/ -lzlib

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIO.lib \
                          $$BUILD_DIR/Common/Common.lib
    }
}

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/lzo-2.09/include \
               $$EXTERNALS_DIR/tinyxml2/include \
               $$EXTERNALS_DIR/zlib/include

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
    CQuaternion.cpp \
    CTransform4f.cpp \
    CVector2f.cpp \
    CVector2i.cpp \
    CVector3f.cpp \
    CVector4f.cpp \
    MathUtil.cpp
