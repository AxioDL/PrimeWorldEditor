#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:34:33
#
#-------------------------------------------------

QT += core gui opengl widgets
DEFINES += PWE_EDITOR
RESOURCES += Icons.qrc

win32: {
    QMAKE_CXXFLAGS += /WX \
        -std:c++17

    RC_ICONS += icons/AppIcon.ico
    QT += winextras
}

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
    LIBS += -L$$BUILD_DIR/Core/ -lCored \
            -L$$EXTERNALS_DIR/assimp/lib/Debug -lassimp-vc140-mt \
            -L$$EXTERNALS_DIR/LibCommon/Build -lLibCommond \
            -L$$EXTERNALS_DIR/nod/lib/Debug -lnod \
            -L$$EXTERNALS_DIR/nod/logvisor/Debug -llogvisor \
            -L$$EXTERNALS_DIR/zlib/lib/ -lzlibd

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$EXTERNALS_DIR/LibCommon/Build/LibCommond.lib \
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
    LIBS += -L$$BUILD_DIR/Core/ -lCore \
            -L$$EXTERNALS_DIR/assimp/lib/Release -lassimp-vc140-mt \
            -L$$EXTERNALS_DIR/LibCommon/Build -lLibCommon \
            -L$$EXTERNALS_DIR/nod/lib/Release -lnod \
            -L$$EXTERNALS_DIR/nod/logvisor/Release -llogvisor \
            -L$$EXTERNALS_DIR/zlib/lib/ -lzlib

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$EXTERNALS_DIR/LibCommon/Build/LibCommon.lib \
                          $$BUILD_DIR/Core/Core.lib
    }
}

# Debug/Release Libs
LIBS += -L$$EXTERNALS_DIR/glew-2.1.0/lib/Release/x64 -lglew32s \
        -lopengl32

# Include Paths
INCLUDEPATH += $$PWE_MAIN_INCLUDE \
               $$EXTERNALS_DIR/assimp/include \
               $$EXTERNALS_DIR/CodeGen/include \
               $$EXTERNALS_DIR/glew-2.1.0/include \
               $$EXTERNALS_DIR/LibCommon/Source \
               $$EXTERNALS_DIR/lzo/include \
               $$EXTERNALS_DIR/nod/include \
               $$EXTERNALS_DIR/nod/logvisor/include \
               $$EXTERNALS_DIR/tinyxml2 \
               $$EXTERNALS_DIR/zlib

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
    Widgets/WColorPicker.h \
    Widgets/WDraggableSpinBox.h \
    Widgets/WIntegralSpinBox.h \
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
    INodeEditor.h \
    TestDialog.h \
    UICommon.h \
    CErrorLogDialog.h \
    Undo/CSelectAllCommand.h \
    Undo/CInvertSelectionCommand.h \
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
    CAboutDialog.h \
    CharacterEditor/CCharacterEditor.h \
    CharacterEditor/CCharacterEditorViewport.h \
    CGridRenderable.h \
    CharacterEditor/CSkeletonHierarchyModel.h \
    CLineRenderable.h \
    WorldEditor/CCollisionRenderSettingsDialog.h \
    ResourceBrowser/CResourceBrowser.h \
    ResourceBrowser/CResourceTableModel.h \
    ResourceBrowser/CResourceProxyModel.h \
    ResourceBrowser/CVirtualDirectoryModel.h \
    CEditorApplication.h \
    IEditor.h \
    Widgets/CResourceSelector.h \
    CExportGameDialog.h \
    WorldEditor/CScriptEditSidebar.h \
    WorldEditor/CWorldInfoSidebar.h \
    WorldEditor/CWorldTreeModel.h \
    Widgets/CTimedLineEdit.h \
    CProjectSettingsDialog.h \
    WorldEditor/CPoiMapSidebar.h \
    WorldEditor/CWorldEditorSidebar.h \
    CProgressDialog.h \
    IProgressNotifierUI.h \
    CUIRelay.h \
    Widgets/CSelectResourcePanel.h \
    Widgets/CFilteredResourceModel.h \
    ResourceBrowser/CResourceDelegate.h \
    ResourceBrowser/CResourceTableContextMenu.h \
    ResourceBrowser/CResourceMimeData.h \
    ResourceBrowser/CResourceTableView.h \
    Undo/CMoveResourceCommand.h \
    Undo/CMoveDirectoryCommand.h \
    Undo/CRenameResourceCommand.h \
    Undo/CRenameDirectoryCommand.h \
    CFileNameValidator.h \
    Undo/ICreateDeleteDirectoryCommand.h \
    ResourceBrowser/CVirtualDirectoryTreeView.h \
    CPropertyNameValidator.h \
    Widgets/CSoftValidatorLineEdit.h \
    Widgets/CValidityLabel.h \
    CGeneratePropertyNamesDialog.h \
    CProgressBarNotifier.h \
    Widgets/CCheckableTreeWidgetItem.h \
    Widgets/CCheckableTreeWidget.h \
    Undo/IEditPropertyCommand.h \
    Widgets/TEnumComboBox.h \
    StringEditor/CStringEditor.h \
    StringEditor/CStringListModel.h \
    StringEditor/CStringDelegate.h \
    CCustomDelegate.h \
    CTweakEditor.h \
    Undo/CEditIntrinsicPropertyCommand.h \
    Undo/TSerializeUndoCommand.h

# Source Files
SOURCES += \
    ModelEditor/CModelEditorViewport.cpp \
    ModelEditor/CModelEditorWindow.cpp \
    Undo/CRotateNodeCommand.cpp \
    Undo/CScaleNodeCommand.cpp \
    Undo/CTranslateNodeCommand.cpp \
    Widgets/IPreviewPanel.cpp \
    Widgets/WColorPicker.cpp \
    Widgets/WDraggableSpinBox.cpp \
    Widgets/WIntegralSpinBox.cpp \
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
    INodeEditor.cpp \
    main.cpp \
    TestDialog.cpp \
    UICommon.cpp \
    CErrorLogDialog.cpp \
    WorldEditor/CPoiMapModel.cpp \
    PropertyEdit/CPropertyModel.cpp \
    PropertyEdit/CPropertyDelegate.cpp \
    PropertyEdit/CPropertyView.cpp \
    WorldEditor/CInstancesModel.cpp \
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
    CAboutDialog.cpp \
    CharacterEditor/CCharacterEditor.cpp \
    CharacterEditor/CCharacterEditorViewport.cpp \
    CharacterEditor/CSkeletonHierarchyModel.cpp \
    WorldEditor/CCollisionRenderSettingsDialog.cpp \
    ResourceBrowser/CResourceBrowser.cpp \
    CEditorApplication.cpp \
    Widgets/CResourceSelector.cpp \
    CExportGameDialog.cpp \
    WorldEditor/CScriptEditSidebar.cpp \
    WorldEditor/CWorldInfoSidebar.cpp \
    WorldEditor/CWorldTreeModel.cpp \
    CProjectSettingsDialog.cpp \
    WorldEditor/CPoiMapSidebar.cpp \
    WorldEditor/CWorldEditorSidebar.cpp \
    CProgressDialog.cpp \
    Widgets/CSelectResourcePanel.cpp \
    ResourceBrowser/CResourceDelegate.cpp \
    ResourceBrowser/CResourceTableContextMenu.cpp \
    ResourceBrowser/CResourceTableModel.cpp \
    ResourceBrowser/CResourceTableView.cpp \
    ResourceBrowser/CVirtualDirectoryModel.cpp \
    ResourceBrowser/CVirtualDirectoryTreeView.cpp \
    CPropertyNameValidator.cpp \
    CGeneratePropertyNamesDialog.cpp \
    Undo/IEditPropertyCommand.cpp \
    StringEditor/CStringEditor.cpp \
    StringEditor/CStringListModel.cpp \
    IEditor.cpp \
    StringEditor/CStringDelegate.cpp \
    CTweakEditor.cpp

# UI Files
FORMS += \
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
    WorldEditor/CTemplateEditDialog.ui \
    WorldEditor/CLinkDialog.ui \
    WorldEditor/CSelectInstanceDialog.ui \
    CAboutDialog.ui \
    CharacterEditor/CCharacterEditor.ui \
    WorldEditor/CCollisionRenderSettingsDialog.ui \
    ResourceBrowser/CResourceBrowser.ui \
    CExportGameDialog.ui \
    WorldEditor/CWorldInfoSidebar.ui \
    CProjectSettingsDialog.ui \
    WorldEditor/CPoiMapSidebar.ui \
    CProgressDialog.ui \
    Widgets/CSelectResourcePanel.ui \
    CGeneratePropertyNamesDialog.ui \
    StringEditor/CStringEditor.ui \
    CTweakEditor.ui

# Codegen
CODEGEN_DIR = $$EXTERNALS_DIR/CodeGen
CODEGEN_OUT_PATH = $$BUILD_DIR/Editor/codegen_build/auto_codegen.cpp
CODEGEN_SRC_PATH = $$PWD
include($$EXTERNALS_DIR/CodeGen/codegen.pri)
