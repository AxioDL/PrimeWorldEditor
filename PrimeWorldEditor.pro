#-------------------------------------------------
#
# Project created by QtCreator 2014-08-09T16:15:10
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += console

TARGET = PrimeWorldEditor
TEMPLATE = app

SOURCES += \
    Common/AnimUtil.cpp \
    Common/CAABox.cpp \
    Common/CColor.cpp \
    Common/CFourCC.cpp \
    Common/CMatrix4f.cpp \
    Common/CompressionUtil.cpp \
    Common/CQuaternion.cpp \
    Common/CTimer.cpp \
    Common/CTransform4f.cpp \
    Common/CVector2f.cpp \
    Common/CVector3f.cpp \
    Common/CVector4f.cpp \
    Common/StringUtil.cpp \
    Core/main.cpp \
    Core/CSceneManager.cpp \
    Core/CRenderer.cpp \
    Core/CResCache.cpp \
    Core/CCamera.cpp \
    OpenGL/CIndexBuffer.cpp \
    OpenGL/CShaderGenerator.cpp \
    OpenGL/CVertexBuffer.cpp \
    OpenGL/GLCommon.cpp \
    Resource/CCollisionMesh.cpp \
    Resource/CGameArea.cpp \
    Resource/CMaterial.cpp \
    Resource/CMaterialSet.cpp \
    Resource/CPakFile.cpp \
    Resource/CTexture.cpp \
    Resource/CWorld.cpp \
    Resource/factory/CAreaLoader.cpp \
    Resource/factory/CBlockMgr.cpp \
    Resource/factory/CCollisionLoader.cpp \
    Resource/factory/CMaterialLoader.cpp \
    Resource/factory/CModelLoader.cpp \
    Resource/factory/CTextureDecoder.cpp \
    Resource/model/CBasicModel.cpp \
    Resource/model/CModel.cpp \
    Resource/model/CStaticModel.cpp \
    Resource/script/CScriptObject.cpp \
    Scene/CCollisionNode.cpp \
    Scene/CLightNode.cpp \
    Scene/CModelNode.cpp \
    Scene/CSceneNode.cpp \
    Scene/CStaticNode.cpp \
    UI/CWorldEditorWindow.cpp \
    UI/CStartWindow.cpp \
    Resource/script/CScriptTemplate.cpp \
    Resource/script/CScriptLayer.cpp \
    Resource/CAnimSet.cpp \
    Resource/factory/CAnimSetLoader.cpp \
    Resource/factory/CScriptLoader.cpp \
    Scene/CScriptNode.cpp \
    Resource/CLight.cpp \
    OpenGL/CShader.cpp \
    OpenGL/CUniformBuffer.cpp \
    Resource/factory/CWorldLoader.cpp \
    Resource/CStringTable.cpp \
    Resource/factory/CStringLoader.cpp \
    UI/CEditorGLWidget.cpp \
    Core/CGraphics.cpp \
    Resource/CFont.cpp \
    Resource/factory/CFontLoader.cpp \
    OpenGL/CDynamicVertexBuffer.cpp \
    UI/WColorPicker.cpp \
    UI/WDraggableSpinBox.cpp \
    Resource/cooker/CModelCooker.cpp \
    Resource/cooker/CMaterialCooker.cpp \
    Common/CUniqueID.cpp \
    Resource/cooker/CSectionMgrOut.cpp \
    Common/CHashFNV1A.cpp \
    UI/CModelEditorWindow.cpp \
    Resource/CResource.cpp \
    Core/CToken.cpp \
    Core/CRenderBucket.cpp \
    Core/CDrawUtil.cpp \
    UI/WTextureGLWidget.cpp \
    UI/TestDialog.cpp \
    UI/WRollout.cpp \
    UI/CMaterialEditor.cpp \
    UI/WResourceSelector.cpp \
    UI/IPreviewPanel.cpp \
    UI/WTexturePreviewPanel.cpp \
    UI/UICommon.cpp \
    OpenGL/CVertexArrayManager.cpp \
    UI/WCollapsibleGroupBox.cpp \
    UI/CSimpleDelegate.cpp \
    UI/CDarkStyle.cpp \
    Resource/cooker/CTextureEncoder.cpp \
    Resource/CMaterialPass.cpp \
    OpenGL/CFramebuffer.cpp \
    OpenGL/CRenderbuffer.cpp \
    OpenGL/CGL.cpp \
    UI/CWorldEditor.cpp \
    UI/WorldEditor/WCreateTab.cpp \
    UI/WorldEditor/WModifyTab.cpp \
    UI/WorldEditor/WInstancesTab.cpp \
    Common/CRay.cpp \
    Resource/model/SSurface.cpp \
    Common/CRayCollisionTester.cpp \
    Common/Math.cpp \
    Scene/CBoundingBoxNode.cpp \
    Core/Log.cpp \
    Common/CVector2i.cpp \
    UI/CNodeSelection.cpp \
    UI/WPropertyEditor.cpp \
    UI/WVectorEditor.cpp \
    Resource/script/CMasterTemplate.cpp \
    Resource/factory/CTemplateLoader.cpp \
    Core/CAreaAttributes.cpp \
    UI/WorldEditor/CLinkModel.cpp \
    UI/WorldEditor/CLayersInstanceModel.cpp \
    UI/WorldEditor/CTypesInstanceModel.cpp \
    UI/WorldEditor/CLayerEditor.cpp \
    UI/WorldEditor/CLayerModel.cpp \
    Resource/CScan.cpp \
    Resource/factory/CScanLoader.cpp \
    UI/WStringPreviewPanel.cpp \
    UI/WScanPreviewPanel.cpp \
    UI/WIntegralSpinBox.cpp

HEADERS  += \
    Common/AnimUtil.h \
    Common/CAABox.h \
    Common/CColor.h \
    Common/CFourCC.h \
    Common/CMatrix4f.h \
    Common/CompressionUtil.h \
    Common/CQuaternion.h \
    Common/CTimer.h \
    Common/CTransform4f.h \
    Common/CVector2f.h \
    Common/CVector3f.h \
    Common/CVector4f.h \
    Common/StringUtil.h \
    Common/types.h \
    Core/CCamera.h \
    Core/CRenderer.h \
    Core/CResCache.h \
    Core/CSceneManager.h \
    OpenGL/CIndexBuffer.h \
    OpenGL/CShaderGenerator.h \
    OpenGL/CVertexBuffer.h \
    OpenGL/GLCommon.h \
    OpenGL/SMeshPointer.h \
    UI/CWorldEditorWindow.h \
    UI/PWEMaterialEditor.h \
    UI/CStartWindow.h \
    Resource/CCollisionMesh.h \
    Resource/CGameArea.h \
    Resource/CPakFile.h \
    Resource/CMaterial.h \
    Resource/CMaterialSet.h \
    Resource/CResource.h \
    Resource/CTexture.h \
    Resource/CWorld.h \
    Resource/EFormatVersion.h \
    Resource/factory/CAreaLoader.h \
    Resource/factory/CBlockMgrIn.h \
    Resource/factory/CCollisionLoader.h \
    Resource/factory/CMaterialLoader.h \
    Resource/factory/CModelLoader.h \
    Resource/factory/CTextureDecoder.h \
    Resource/model/CBasicModel.h \
    Resource/model/CModel.h \
    Resource/model/CStaticModel.h \
    Resource/model/CVertex.h \
    Resource/script/CProperty.h \
    Resource/script/CScriptLayer.h \
    Resource/script/CScriptObject.h \
    Resource/script/CScriptTemplate.h \
    Resource/script/EObjectType.h \
    Resource/script/SConnection.h \
    Resource/SNamedResource.h \
    Resource/SResInfo.h \
    Scene/CCollisionNode.h \
    Scene/CLightNode.h \
    Scene/CModelNode.h \
    Scene/CSceneNode.h \
    Scene/CStaticNode.h \
    Resource/script/EAttribType.h \
    Resource/CAnimSet.h \
    Resource/factory/CAnimSetLoader.h \
    Resource/factory/CScriptLoader.h \
    Resource/script/EPropertyType.h \
    Scene/CScriptNode.h \
    Resource/CLight.h \
    OpenGL/CShader.h \
    OpenGL/CUniformBuffer.h \
    Resource/factory/CWorldLoader.h \
    Resource/SDependency.h \
    Resource/CStringTable.h \
    Resource/factory/CStringLoader.h \
    UI/CEditorGLWidget.h \
    Core/CGraphics.h \
    Resource/CFont.h \
    Resource/factory/CFontLoader.h \
    OpenGL/CDynamicVertexBuffer.h \
    UI/WColorPicker.h \
    UI/WDraggableSpinBox.h \
    Resource/cooker/CModelCooker.h \
    Resource/cooker/CMaterialCooker.h \
    Common/CUniqueID.h \
    Resource/model/SSurface.h \
    Resource/model/EVertexDescription.h \
    Common/EnumUtil.h \
    Resource/cooker/CSectionMgrOut.h \
    Common/CHashFNV1A.h \
    Core/ERenderOptions.h \
    UI/CModelEditorWindow.h \
    Core/CToken.h \
    Core/CRenderBucket.h \
    Common/EMouseInputs.h \
    Common/EKeyInputs.h \
    Core/CDrawUtil.h \
    UI/WTextureGLWidget.h \
    UI/TestDialog.h \
    Resource/ETexelFormat.h \
    UI/WRollout.h \
    UI/CMaterialEditor.h \
    UI/WResourceSelector.h \
    Resource/EResType.h \
    UI/IPreviewPanel.h \
    UI/WTexturePreviewPanel.h \
    UI/UICommon.h \
    OpenGL/CVertexArrayManager.h \
    UI/WCollapsibleGroupBox.h \
    UI/CSimpleDelegate.h \
    UI/CDarkStyle.h \
    Resource/cooker/CTextureEncoder.h \
    Resource/CMaterialPass.h \
    Resource/ETevEnums.h \
    OpenGL/CFramebuffer.h \
    OpenGL/CRenderbuffer.h \
    OpenGL/CGL.h \
    UI/CWorldEditor.h \
    UI/WorldEditor/WCreateTab.h \
    UI/WorldEditor/WModifyTab.h \
    Common/CRay.h \
    Common/SRayIntersection.h \
    Common/CRayCollisionTester.h \
    Scene/ENodeType.h \
    Common/Math.h \
    Scene/CBoundingBoxNode.h \
    Core/Log.h \
    Scene/CRootNode.h \
    Common/CVector2i.h \
    Core/ERenderCommand.h \
    UI/CNodeSelection.h \
    UI/WPropertyEditor.h \
    UI/WVectorEditor.h \
    Resource/script/CMasterTemplate.h \
    Resource/script/CTemplateCategory.h \
    Resource/factory/CTemplateLoader.h \
    Core/CAreaAttributes.h \
    UI/WorldEditor/CLinkModel.h \
    UI/WorldEditor/WInstancesTab.h \
    UI/WorldEditor/CLayersInstanceModel.h \
    UI/WorldEditor/CTypesInstanceModel.h \
    UI/WorldEditor/CLayerEditor.h \
    UI/WorldEditor/CLayerModel.h \
    Resource/CScan.h \
    Resource/factory/CScanLoader.h \
    UI/WStringPreviewPanel.h \
    UI/WScanPreviewPanel.h \
    UI/WIntegralSpinBox.h

FORMS    += \
    UI/CWorldEditorWindow.ui \
    UI/CStartWindow.ui \
    UI/CModelEditorWindow.ui \
    UI/TestDialog.ui \
    UI/CMaterialEditor.ui \
    UI/WTexturePreviewPanel.ui \
    UI/CWorldEditor.ui \
    UI/WorldEditor/WCreateTab.ui \
    UI/WorldEditor/WModifyTab.ui \
    UI/WorldEditor/WInstancesTab.ui \
    UI/WorldEditor/CLayerEditor.ui \
    UI/WScanPreviewPanel.ui

INCLUDEPATH += E:\C++\Libraries\glm\glm  .\

LIBS += -lOpenGL32

LIBS += -LE:/C++/Libraries/zlib/lib -lzdll
INCLUDEPATH += E:/C++/Libraries/zlib/include

LIBS += -LE:/C++/Libraries/lzo-2.08/lib -llzo-2.08
INCLUDEPATH += E:/C++/Libraries/lzo-2.08/include

unix|win32: LIBS += -LE:/C++/Libraries/glew-1.9.0/lib/ -lglew32s
INCLUDEPATH += E:/C++/Libraries/glew-1.9.0/include
DEPENDPATH += E:/C++/Libraries/glew-1.9.0/include
PRE_TARGETDEPS += E:/C++/Libraries/glew-1.9.0/lib/glew32s.lib
DEFINES += GLEW_STATIC

INCLUDEPATH += E:/C++/Libraries/tinyxml2/include
win32:CONFIG(release, debug|release): LIBS += -LE:/C++/Libraries/tinyxml2/lib/ -ltinyxml2
else:win32:CONFIG(debug, debug|release): LIBS += -LE:/C++/Libraries/tinyxml2/lib/ -ltinyxml2d
win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += E:/C++/Libraries/tinyxml2/lib/libtinyxml2.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += E:/C++/Libraries/tinyxml2/lib/libtinyxml2d.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += E:/C++/Libraries/tinyxml2/lib/tinyxml2.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += E:/C++/Libraries/tinyxml2/lib/tinyxml2d.lib

win32:CONFIG(release, debug|release): LIBS += -LE:/C++/Libraries/FileIO/lib/ -lFileIO
else:win32:CONFIG(debug, debug|release): LIBS += -LE:/C++/Libraries/FileIO/lib/ -lFileIOd

INCLUDEPATH += E:/C++/Libraries/FileIO/include
DEPENDPATH += E:/C++/Libraries/FileIO/include
CONFIG(release, debug|release): PRE_TARGETDEPS += E:/C++/Libraries/FileIO/lib/FileIO.lib
CONFIG(debug, debug|release): PRE_TARGETDEPS += E:/C++/Libraries/FileIO/lib/FileIOd.lib

DISTFILES += \
    ../../../../../PWEassets/Icons/Free Camera.png \
    ../../../../../PWEassets/Icons/Material Highlight.png \
    ../../../../../PWEassets/Icons/Minus v2.png \
    ../../../../../PWEassets/Icons/Orbit Camera v2.png \
    ../../../../../PWEassets/Icons/Orbit Camera.png \
    ../../../../../PWEassets/Icons/Samus Silhouette Gradient.png \
    ../../../../../PWEassets/Icons/Square Preview.png

RESOURCES += \
    Icons.qrc

