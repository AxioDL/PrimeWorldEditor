#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:34:33
#
#-------------------------------------------------

QT += core gui opengl widgets
QMAKE_CXXFLAGS += /WX
DEFINES += PWE_EDITOR
RESOURCES += Icons.qrc

win32: RC_ICONS += icons/AppIcon.ico

TEMPLATE = app
DESTDIR = $$PWD/../../bin
UI_DIR = $$BUILD_DIR/Editor
DEFINES += GLEW_STATIC

!PUBLIC_RELEASE {
    CONFIG += console
}

CONFIG(debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/Editor/debug
    MOC_DIR = $$BUILD_DIR/Editor/debug
    RCC_DIR = $$BUILD_DIR/Editor/debug
    TARGET = PrimeWorldEditor-debug

    # Debug Libs
    LIBS += -L$$BUILD_DIR/FileIO/ -lFileIOd \
            -L$$BUILD_DIR/Common/ -lCommond \
            -L$$BUILD_DIR/Math/ -lMathd \
            -L$$BUILD_DIR/Core/ -lCored \
            -L$$EXTERNALS_DIR/assimp/lib/ -lassimp-vc120-mtd \
            -L$$EXTERNALS_DIR/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-gd-1_56 \
            -L$$EXTERNALS_DIR/tinyxml2/lib/ -ltinyxml2d

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIOd.lib \
                          $$BUILD_DIR/Common/Commond.lib \
                          $$BUILD_DIR/Math/Mathd.lib \
                          $$BUILD_DIR/Core/Cored.lib
    }
}

CONFIG(release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/Editor/release
    MOC_DIR = $$BUILD_DIR/Editor/release
    RCC_DIR = $$BUILD_DIR/Editor/release
    TARGET = PrimeWorldEditor

    # Release Libs
    LIBS += -L$$BUILD_DIR/FileIO/ -lFileIO \
            -L$$BUILD_DIR/Common/ -lCommon \
            -L$$BUILD_DIR/Math/ -lMath \
            -L$$BUILD_DIR/Core/ -lCore \
            -L$$EXTERNALS_DIR/assimp/lib/ -lassimp-vc120-mt \
            -L$$EXTERNALS_DIR/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-1_56 \
            -L$$EXTERNALS_DIR/tinyxml2/lib/ -ltinyxml2

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$BUILD_DIR/FileIO/FileIO.lib \
                          $$BUILD_DIR/Common/Common.lib \
                          $$BUILD_DIR/Math/Math.lib \
                          $$BUILD_DIR/Core/Core.lib
    }
}

# Debug/Release Libs
LIBS += -L$$EXTERNALS_DIR/glew-1.9.0/lib/ -lglew32s \
        -L$$EXTERNALS_DIR/lzo-2.08/lib -llzo-2.08 \
        -L$$EXTERNALS_DIR/zlib/lib -lzdll

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/assimp/include \
               $$EXTERNALS_DIR/glew-1.9.0/include \
               $$EXTERNALS_DIR/glm/glm \
               $$EXTERNALS_DIR/lzo-2.08/include \
               $$EXTERNALS_DIR/tinyxml2/include \
               $$EXTERNALS_DIR/zlib/include

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
    Widgets/WColorPicker.h \
    Widgets/WDraggableSpinBox.h \
    Widgets/WIntegralSpinBox.h \
    Widgets/WResourceSelector.h \
    Widgets/WScanPreviewPanel.h \
    Widgets/WStringPreviewPanel.h \
    Widgets/WTextureGLWidget.h \
    Widgets/WTexturePreviewPanel.h \
    Widgets/WVectorEditor.h \
    WorldEditor/CLayerEditor.h \
    WorldEditor/CLayerModel.h \
    WorldEditor/CLinkModel.h \
    WorldEditor/CWorldEditor.h \
    WorldEditor/WCreateTab.h \
    WorldEditor/WInstancesTab.h \
    WorldEditor/WModifyTab.h \
    CBasicViewport.h \
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
    WorldEditor/CInstancesModel.h \
    Undo/CEditScriptPropertyCommand.h \
    Undo/CResizeScriptArrayCommand.h \
    Undo/IUndoCommand.h \
    WorldEditor/WEditorProperties.h \
    Undo/CChangeLayerCommand.h \
    WorldEditor/CTemplateEditDialog.h \
    WorldEditor/CLinkDialog.h \
    WorldEditor/CStateMessageModel.h \
    WorldEditor/CSelectInstanceDialog.h \
    Undo/CAddLinkCommand.h \
    Undo/CDeleteLinksCommand.h \
    Undo/CEditLinkCommand.h \
    WorldEditor/CConfirmUnlinkDialog.h \
    Undo/CDeleteSelectionCommand.h \
    Undo/CCreateInstanceCommand.h \
    WorldEditor/CTemplateMimeData.h \
    WorldEditor/CTemplateListView.h \
    CSelectionIterator.h \
    Undo/ObjReferences.h \
    Undo/CCloneSelectionCommand.h \
    CNodeCopyMimeData.h \
    Undo/CPasteNodesCommand.h \
    CPakToolDialog.h \
    WorldEditor/CRepackInfoDialog.h \
    CAboutDialog.h \
    CharacterEditor/CCharacterEditor.h \
    CharacterEditor/CCharacterEditorViewport.h \
    CGridRenderable.h \
    CharacterEditor/CSkeletonHierarchyModel.h \
    CLineRenderable.h \
    CGameExporter.h

# Source Files
SOURCES += \
    ModelEditor/CModelEditorViewport.cpp \
    ModelEditor/CModelEditorWindow.cpp \
    Undo/CRotateNodeCommand.cpp \
    Undo/CScaleNodeCommand.cpp \
    Undo/CTranslateNodeCommand.cpp \
    Widgets/IPreviewPanel.cpp \
    Widgets/WAnimParamsEditor.cpp \
    Widgets/WColorPicker.cpp \
    Widgets/WDraggableSpinBox.cpp \
    Widgets/WIntegralSpinBox.cpp \
    Widgets/WResourceSelector.cpp \
    Widgets/WScanPreviewPanel.cpp \
    Widgets/WStringPreviewPanel.cpp \
    Widgets/WTextureGLWidget.cpp \
    Widgets/WTexturePreviewPanel.cpp \
    Widgets/WVectorEditor.cpp \
    WorldEditor/CLayerEditor.cpp \
    WorldEditor/CLayerModel.cpp \
    WorldEditor/CLinkModel.cpp \
    WorldEditor/CWorldEditor.cpp \
    WorldEditor/WCreateTab.cpp \
    WorldEditor/WInstancesTab.cpp \
    WorldEditor/WModifyTab.cpp \
    CBasicViewport.cpp \
    CGizmo.cpp \
    CSceneViewport.cpp \
    CStartWindow.cpp \
    INodeEditor.cpp \
    main.cpp \
    TestDialog.cpp \
    UICommon.cpp \
    CErrorLogDialog.cpp \
    WorldEditor/CPoiMapEditDialog.cpp \
    WorldEditor/CPoiMapModel.cpp \
    PropertyEdit/CPropertyModel.cpp \
    PropertyEdit/CPropertyDelegate.cpp \
    PropertyEdit/CPropertyView.cpp \
    WorldEditor/CInstancesModel.cpp \
    Undo/CEditScriptPropertyCommand.cpp \
    Undo/CResizeScriptArrayCommand.cpp \
    WorldEditor/WEditorProperties.cpp \
    Undo/CChangeLayerCommand.cpp \
    WorldEditor/CTemplateEditDialog.cpp \
    WorldEditor/CLinkDialog.cpp \
    WorldEditor/CSelectInstanceDialog.cpp \
    Undo/CAddLinkCommand.cpp \
    Undo/CDeleteLinksCommand.cpp \
    Undo/CEditLinkCommand.cpp \
    Undo/CDeleteSelectionCommand.cpp \
    Undo/CCreateInstanceCommand.cpp \
    Undo/CCloneSelectionCommand.cpp \
    Undo/CPasteNodesCommand.cpp \
    WorldEditor/CRepackInfoDialog.cpp \
    CAboutDialog.cpp \
    CharacterEditor/CCharacterEditor.cpp \
    CharacterEditor/CCharacterEditorViewport.cpp \
    CharacterEditor/CSkeletonHierarchyModel.cpp \
    CGameExporter.cpp

# UI Files
FORMS += \
    CStartWindow.ui \
    TestDialog.ui \
    ModelEditor/CModelEditorWindow.ui \
    Widgets/WScanPreviewPanel.ui \
    Widgets/WTexturePreviewPanel.ui \
    WorldEditor/CLayerEditor.ui \
    WorldEditor/CWorldEditor.ui \
    WorldEditor/WCreateTab.ui \
    WorldEditor/WInstancesTab.ui \
    WorldEditor/WModifyTab.ui \
    CErrorLogDialog.ui \
    WorldEditor/CPoiMapEditDialog.ui \
    WorldEditor/CTemplateEditDialog.ui \
    WorldEditor/CLinkDialog.ui \
    WorldEditor/CSelectInstanceDialog.ui \
    WorldEditor/CRepackInfoDialog.ui \
    CAboutDialog.ui \
    CharacterEditor/CCharacterEditor.ui
