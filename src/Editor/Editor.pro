#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:34:33
#
#-------------------------------------------------

QT += core gui opengl widgets
QMAKE_CXXFLAGS += /WX
DEFINES += PWE_EDITOR
RESOURCES += Icons.qrc

CONFIG += console
TEMPLATE = app
DESTDIR = $$PWD/../../bin
UI_DIR = $$PWD/../../build/Editor
DEFINES += GLEW_STATIC

CONFIG(debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$PWD/../../build/Editor/debug
    MOC_DIR = $$PWD/../../build/Editor/debug
    RCC_DIR = $$PWD/../../build/Editor/debug
    TARGET = PrimeWorldEditor-debug

    # Debug Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIOd \
            -L$$PWD/../../build/Common/ -lCommond \
            -L$$PWD/../../build/Math/ -lMathd \
            -L$$PWD/../../build/Core/ -lCored \
            -L$$PWD/../../externals/assimp/lib/ -lassimp-vc120-mtd \
            -L$$PWD/../../externals/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-gd-1_56 \
            -L$$PWD/../../externals/tinyxml2/lib/ -ltinyxml2d

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIOd.lib \
                          $$PWD/../../build/Common/Commond.lib \
                          $$PWD/../../build/Math/Mathd.lib \
                          $$PWD/../../build/Core/Cored.lib
    }
}

CONFIG(release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/Editor/release
    MOC_DIR = $$PWD/../../build/Editor/release
    RCC_DIR = $$PWD/../../build/Editor/release
    TARGET = PrimeWorldEditor

    # Release Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIO \
            -L$$PWD/../../build/Common/ -lCommon \
            -L$$PWD/../../build/Math/ -lMath \
            -L$$PWD/../../build/Core/ -lCore \
            -L$$PWD/../../externals/assimp/lib/ -lassimp-vc120-mt \
            -L$$PWD/../../externals/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-1_56 \
            -L$$PWD/../../externals/tinyxml2/lib/ -ltinyxml2

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIO.lib \
                          $$PWD/../../build/Common/Common.lib \
                          $$PWD/../../build/Math/Math.lib \
                          $$PWD/../../build/Core/Core.lib
    }
}

# Debug/Release Libs
LIBS += -L$$PWD/../../externals/glew-1.9.0/lib/ -lglew32s \
        -L$$PWD/../../externals/lzo-2.08/lib -llzo-2.08 \
        -L$$PWD/../../externals/zlib/lib -lzdll

# Include Paths
INCLUDEPATH += $$PWD/../ \
               $$PWD/../../externals/assimp/include \
               $$PWD/../../externals/boost_1_56_0 \
               $$PWD/../../externals/glew-1.9.0/include \
               $$PWD/../../externals/glm/glm \
               $$PWD/../../externals/lzo-2.08/include \
               $$PWD/../../externals/tinyxml2/include \
               $$PWD/../../externals/zlib/include

# Header Files
HEADERS += \
    ModelEditor/CModelEditorViewport.h \
    ModelEditor/CModelEditorWindow.h \
    Undo/CClearSelectionCommand.h \
    Undo/CDeselectNodeCommand.h \
    Undo/CRotateNodeCommand.h \
    Undo/CScaleNodeCommand.h \
    Undo/CSelectNodeCommand.h \
    Undo/CTranslateNodeCommand.h \
    Undo/EUndoCommand.h \
    Undo/UndoCommands.h \
    Widgets/IPreviewPanel.h \
    Widgets/WAnimParamsEditor.h \
    Widgets/WCollapsibleGroupBox.h \
    Widgets/WColorPicker.h \
    Widgets/WDraggableSpinBox.h \
    Widgets/WIntegralSpinBox.h \
    Widgets/WResourceSelector.h \
    Widgets/WRollout.h \
    Widgets/WScanPreviewPanel.h \
    Widgets/WStringPreviewPanel.h \
    Widgets/WTextureGLWidget.h \
    Widgets/WTexturePreviewPanel.h \
    Widgets/WVectorEditor.h \
    WorldEditor/CAboutDialog.h \
    WorldEditor/CLayerEditor.h \
    WorldEditor/CLayerModel.h \
    WorldEditor/CLinkModel.h \
    WorldEditor/CWorldEditor.h \
    WorldEditor/WCreateTab.h \
    WorldEditor/WInstancesTab.h \
    WorldEditor/WModifyTab.h \
    CBasicViewport.h \
    CDarkStyle.h \
    CGizmo.h \
    CNodeSelection.h \
    CSceneViewport.h \
    CStartWindow.h \
    INodeEditor.h \
    TestDialog.h \
    UICommon.h \
    CErrorLogDialog.h \
    Undo/CSelectAllCommand.h \
    Undo/CInvertSelectionCommand.h \
    WorldEditor/CPoiMapEditDialog.h \
    WorldEditor/CPoiMapModel.h \
    WorldEditor/CPoiListDialog.h \
    PropertyEdit/CPropertyModel.h \
    PropertyEdit/CPropertyDelegate.h \
    PropertyEdit/CPropertyView.h \
    PropertyEdit/CPropertyRelay.h \
    WorldEditor/CInstancesProxyModel.h \
    WorldEditor/CInstancesModel.h

# Source Files
SOURCES += \
    ModelEditor/CModelEditorViewport.cpp \
    ModelEditor/CModelEditorWindow.cpp \
    Undo/CClearSelectionCommand.cpp \
    Undo/CDeselectNodeCommand.cpp \
    Undo/CRotateNodeCommand.cpp \
    Undo/CScaleNodeCommand.cpp \
    Undo/CSelectNodeCommand.cpp \
    Undo/CTranslateNodeCommand.cpp \
    Widgets/IPreviewPanel.cpp \
    Widgets/WAnimParamsEditor.cpp \
    Widgets/WCollapsibleGroupBox.cpp \
    Widgets/WColorPicker.cpp \
    Widgets/WDraggableSpinBox.cpp \
    Widgets/WIntegralSpinBox.cpp \
    Widgets/WResourceSelector.cpp \
    Widgets/WRollout.cpp \
    Widgets/WScanPreviewPanel.cpp \
    Widgets/WStringPreviewPanel.cpp \
    Widgets/WTextureGLWidget.cpp \
    Widgets/WTexturePreviewPanel.cpp \
    Widgets/WVectorEditor.cpp \
    WorldEditor/CAboutDialog.cpp \
    WorldEditor/CLayerEditor.cpp \
    WorldEditor/CLayerModel.cpp \
    WorldEditor/CLinkModel.cpp \
    WorldEditor/CWorldEditor.cpp \
    WorldEditor/WCreateTab.cpp \
    WorldEditor/WInstancesTab.cpp \
    WorldEditor/WModifyTab.cpp \
    CBasicViewport.cpp \
    CDarkStyle.cpp \
    CGizmo.cpp \
    CNodeSelection.cpp \
    CSceneViewport.cpp \
    CStartWindow.cpp \
    INodeEditor.cpp \
    main.cpp \
    TestDialog.cpp \
    UICommon.cpp \
    CErrorLogDialog.cpp \
    Undo/CSelectAllCommand.cpp \
    Undo/CInvertSelectionCommand.cpp \
    WorldEditor/CPoiMapEditDialog.cpp \
    WorldEditor/CPoiMapModel.cpp \
    PropertyEdit/CPropertyModel.cpp \
    PropertyEdit/CPropertyDelegate.cpp \
    PropertyEdit/CPropertyView.cpp \
    WorldEditor/CInstancesModel.cpp

# UI Files
FORMS += \
    CStartWindow.ui \
    TestDialog.ui \
    ModelEditor/CModelEditorWindow.ui \
    Widgets/WScanPreviewPanel.ui \
    Widgets/WTexturePreviewPanel.ui \
    WorldEditor/CAboutDialog.ui \
    WorldEditor/CLayerEditor.ui \
    WorldEditor/CWorldEditor.ui \
    WorldEditor/WCreateTab.ui \
    WorldEditor/WInstancesTab.ui \
    WorldEditor/WModifyTab.ui \
    CErrorLogDialog.ui \
    WorldEditor/CPoiMapEditDialog.ui
