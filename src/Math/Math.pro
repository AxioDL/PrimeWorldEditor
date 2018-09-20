#-------------------------------------------------
#
# Project created by QtCreator 2015-12-16T12:35:34
#
#-------------------------------------------------

QT       -= core gui
DEFINES += PWE_MATH

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$BUILD_DIR/Math

unix {
    target.path = /usr/lib
    QMAKE_CXXFLAGS += /WX
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/Math/debug
    TARGET = Mathd

    # Debug Libs
    LIBS += -L$$BUILD_DIR/Common/ -lCommond

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/Common/Commond.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/Math/release
    TARGET = Math

    # Release Libs
    LIBS += -L$$BUILD_DIR/Common/ -lCommon

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/Common/Common.lib
    }
}

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/CodeGen/include \
               $$EXTERNALS_DIR/tinyxml2

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

# Codegen
CODEGEN_DIR = $$EXTERNALS_DIR/CodeGen
CODEGEN_OUT_PATH = $$BUILD_DIR/Common/codegen_build/auto_codegen.cpp
CODEGEN_SRC_PATH = $$PWD
include($$EXTERNALS_DIR/CodeGen/codegen.pri)
