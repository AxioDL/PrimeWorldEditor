#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:31:43
#
#-------------------------------------------------

QT -= core gui
QMAKE_CXXFLAGS += /WX
DEFINES += PWE_CORE

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$PWD/../../build/Core
DEFINES += GLEW_STATIC

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$PWD/../../build/Core/debug
    TARGET = Cored

    # Debug Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIOd \
            -L$$PWD/../../build/Common/ -lCommond \
            -L$$PWD/../../build/Math/ -lMathd \
            -L$$PWD/../../externals/assimp/lib/ -lassimp-vc120-mtd \
            -L$$PWD/../../externals/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-gd-1_56 \
            -L$$PWD/../../externals/tinyxml2/lib/ -ltinyxml2d

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIOd.lib \
                          $$PWD/../../build/Common/Commond.lib \
                          $$PWD/../../build/Math/Mathd.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$PWD/../../build/Core/release
    TARGET = Core

    # Release Libs
    LIBS += -L$$PWD/../../build/FileIO/ -lFileIO \
            -L$$PWD/../../build/Common/ -lCommon \
            -L$$PWD/../../build/Math/ -lMath \
            -L$$PWD/../../externals/assimp/lib/ -lassimp-vc120-mt \
            -L$$PWD/../../externals/boost_1_56_0/lib32-msvc-12.0 -llibboost_filesystem-vc120-mt-1_56 \
            -L$$PWD/../../externals/tinyxml2/lib/ -ltinyxml2

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$PWD/../../build/FileIO/FileIO.lib \
                          $$PWD/../../build/Common/Common.lib \
                          $$PWD/../../build/Math/Math.lib
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
    Render/CCamera.h \
    Render/CDrawUtil.h \
    Render/CGraphics.h \
    Render/CRenderBucket.h \
    Render/CRenderer.h \
    Render/ERenderCommand.h \
    Render/IRenderable.h \
    Render/SRenderablePtr.h \
    Render/SViewInfo.h \
    Resource/Cooker/CMaterialCooker.h \
    Resource/Cooker/CModelCooker.h \
    Resource/Cooker/CSectionMgrOut.h \
    Resource/Cooker/CTemplateWriter.h \
    Resource/Cooker/CTextureEncoder.h \
    Resource/Cooker/CWorldCooker.h \
    Resource/Factory/CAnimSetLoader.h \
    Resource/Factory/CAreaLoader.h \
    Resource/Factory/CCollisionLoader.h \
    Resource/Factory/CFontLoader.h \
    Resource/Factory/CMaterialLoader.h \
    Resource/Factory/CModelLoader.h \
    Resource/Factory/CScanLoader.h \
    Resource/Factory/CScriptLoader.h \
    Resource/Factory/CStringLoader.h \
    Resource/Factory/CTemplateLoader.h \
    Resource/Factory/CTextureDecoder.h \
    Resource/Factory/CWorldLoader.h \
    Resource/Model/CBasicModel.h \
    Resource/Model/CModel.h \
    Resource/Model/CStaticModel.h \
    Resource/Model/CVertex.h \
    Resource/Model/SSurface.h \
    Resource/Script/CMasterTemplate.h \
    Resource/Script/CScriptLayer.h \
    Resource/Script/CScriptObject.h \
    Resource/Script/CScriptTemplate.h \
    Resource/Script/EPropertyType.h \
    Resource/Script/EVolumeShape.h \
    Resource/Script/SConnection.h \
    Resource/CAnimationParameters.h \
    Resource/CAnimSet.h \
    Resource/CCollisionMesh.h \
    Resource/CCollisionMeshGroup.h \
    Resource/CFont.h \
    Resource/CGameArea.h \
    Resource/CLight.h \
    Resource/CMaterial.h \
    Resource/CMaterialPass.h \
    Resource/CMaterialSet.h \
    Resource/CPakFile.h \
    Resource/CResCache.h \
    Resource/CResource.h \
    Resource/CScan.h \
    Resource/CStringTable.h \
    Resource/CTexture.h \
    Resource/CWorld.h \
    Resource/EResType.h \
    Resource/ETevEnums.h \
    Resource/ETexelFormat.h \
    Resource/SDependency.h \
    Resource/SNamedResource.h \
    Resource/SResInfo.h \
    Resource/TResPtr.h \
    Scene/CCollisionNode.h \
    Scene/CLightNode.h \
    Scene/CModelNode.h \
    Scene/CRootNode.h \
    Scene/CSceneNode.h \
    Scene/CScriptNode.h \
    Scene/CStaticNode.h \
    Scene/ENodeType.h \
    ScriptExtra/CDamageableTriggerExtra.h \
    ScriptExtra/CDoorExtra.h \
    ScriptExtra/CPointOfInterestExtra.h \
    ScriptExtra/CScriptExtra.h \
    ScriptExtra/CSpacePirateExtra.h \
    ScriptExtra/CWaypointExtra.h \
    CAreaAttributes.h \
    CLightParameters.h \
    CRayCollisionTester.h \
    Log.h \
    SRayIntersection.h \
    OpenGL/CDynamicVertexBuffer.h \
    OpenGL/CFramebuffer.h \
    OpenGL/CGL.h \
    OpenGL/CIndexBuffer.h \
    OpenGL/CRenderbuffer.h \
    OpenGL/CShader.h \
    OpenGL/CShaderGenerator.h \
    OpenGL/CUniformBuffer.h \
    OpenGL/CVertexArrayManager.h \
    OpenGL/CVertexBuffer.h \
    OpenGL/GLCommon.h \
    ScriptExtra/CRadiusSphereExtra.h \
    Resource/EGame.h \
    Resource/Cooker/CAreaCooker.h \
    Resource/Script/IPropertyValue.h \
    Resource/Script/IPropertyTemplate.h \
    Resource/Script/IProperty.h \
    Resource/Model/EVertexAttribute.h \
    Render/FRenderOptions.h \
    Scene/FShowFlags.h \
    Scene/CScene.h \
    Scene/CSceneIterator.h \
    Resource/CResourceInfo.h \
    Resource/CPoiToWorld.h \
    Resource/Factory/CPoiToWorldLoader.h \
    Resource/Cooker/CPoiToWorldCooker.h \
    Resource/Factory/CSectionMgrIn.h \
    Resource/Cooker/CScriptCooker.h

# Source Files
SOURCES += \
    Render/CCamera.cpp \
    Render/CDrawUtil.cpp \
    Render/CGraphics.cpp \
    Render/CRenderBucket.cpp \
    Render/CRenderer.cpp \
    Resource/Cooker/CMaterialCooker.cpp \
    Resource/Cooker/CModelCooker.cpp \
    Resource/Cooker/CSectionMgrOut.cpp \
    Resource/Cooker/CTemplateWriter.cpp \
    Resource/Cooker/CTextureEncoder.cpp \
    Resource/Cooker/CWorldCooker.cpp \
    Resource/Factory/CAnimSetLoader.cpp \
    Resource/Factory/CAreaLoader.cpp \
    Resource/Factory/CCollisionLoader.cpp \
    Resource/Factory/CFontLoader.cpp \
    Resource/Factory/CMaterialLoader.cpp \
    Resource/Factory/CModelLoader.cpp \
    Resource/Factory/CScanLoader.cpp \
    Resource/Factory/CScriptLoader.cpp \
    Resource/Factory/CStringLoader.cpp \
    Resource/Factory/CTemplateLoader.cpp \
    Resource/Factory/CTextureDecoder.cpp \
    Resource/Factory/CWorldLoader.cpp \
    Resource/Model/CBasicModel.cpp \
    Resource/Model/CModel.cpp \
    Resource/Model/CStaticModel.cpp \
    Resource/Model/SSurface.cpp \
    Resource/Script/CMasterTemplate.cpp \
    Resource/Script/CScriptObject.cpp \
    Resource/Script/CScriptTemplate.cpp \
    Resource/CAnimationParameters.cpp \
    Resource/CAnimSet.cpp \
    Resource/CCollisionMesh.cpp \
    Resource/CCollisionMeshGroup.cpp \
    Resource/CFont.cpp \
    Resource/CGameArea.cpp \
    Resource/CLight.cpp \
    Resource/CMaterial.cpp \
    Resource/CMaterialPass.cpp \
    Resource/CMaterialSet.cpp \
    Resource/CPakFile.cpp \
    Resource/CResCache.cpp \
    Resource/CResource.cpp \
    Resource/CScan.cpp \
    Resource/CStringTable.cpp \
    Resource/CTexture.cpp \
    Resource/CWorld.cpp \
    Scene/CCollisionNode.cpp \
    Scene/CLightNode.cpp \
    Scene/CModelNode.cpp \
    Scene/CSceneNode.cpp \
    Scene/CScriptNode.cpp \
    Scene/CStaticNode.cpp \
    ScriptExtra/CDamageableTriggerExtra.cpp \
    ScriptExtra/CDoorExtra.cpp \
    ScriptExtra/CPointOfInterestExtra.cpp \
    ScriptExtra/CScriptExtra.cpp \
    ScriptExtra/CSpacePirateExtra.cpp \
    ScriptExtra/CWaypointExtra.cpp \
    CAreaAttributes.cpp \
    CRayCollisionTester.cpp \
    Log.cpp \
    OpenGL/CDynamicVertexBuffer.cpp \
    OpenGL/CFramebuffer.cpp \
    OpenGL/CGL.cpp \
    OpenGL/CIndexBuffer.cpp \
    OpenGL/CRenderbuffer.cpp \
    OpenGL/CShader.cpp \
    OpenGL/CShaderGenerator.cpp \
    OpenGL/CUniformBuffer.cpp \
    OpenGL/CVertexArrayManager.cpp \
    OpenGL/CVertexBuffer.cpp \
    OpenGL/GLCommon.cpp \
    ScriptExtra/CRadiusSphereExtra.cpp \
    Resource/Cooker/CAreaCooker.cpp \
    Resource/Script/IPropertyTemplate.cpp \
    Resource/Script/IProperty.cpp \
    Scene/FShowFlags.cpp \
    Scene/CScene.cpp \
    Scene/CSceneIterator.cpp \
    Resource/CPoiToWorld.cpp \
    Resource/Factory/CPoiToWorldLoader.cpp \
    Resource/Cooker/CPoiToWorldCooker.cpp \
    Resource/Cooker/CScriptCooker.cpp
