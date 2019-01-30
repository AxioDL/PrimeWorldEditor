#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T15:31:43
#
#-------------------------------------------------

QT -= core gui
DEFINES += PWE_CORE

CONFIG += staticlib
TEMPLATE = lib
DESTDIR = $$BUILD_DIR/Core
DEFINES += GLEW_STATIC

win32 {
    QMAKE_CXXFLAGS += -std:c++17
}
unix {
    target.path = /usr/lib
    QMAKE_CXXFLAGS += /WX
    INSTALLS += target
}

CONFIG (debug, debug|release) {
    # Debug Config
    OBJECTS_DIR = $$BUILD_DIR/Core/debug
    TARGET = Cored

    # Debug Libs
    LIBS += -L$$EXTERNALS_DIR/assimp/lib/Debug -lassimp-vc140-mt \
            -L$$EXTERNALS_DIR/LibCommon/Build -lLibCommond \
            -L$$EXTERNALS_DIR/nod/lib/Debug -lnod \
            -L$$EXTERNALS_DIR/nod/logvisor/Debug -llogvisor \
            -L$$EXTERNALS_DIR/zlib/lib/ -lzlibd

    # Debug Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$EXTERNALS_DIR/LibCommon/Build/LibCommond.lib
    }
}

CONFIG (release, debug|release) {
    # Release Config
    OBJECTS_DIR = $$BUILD_DIR/Core/release
    TARGET = Core

    # Release Libs
    LIBS += -L$$EXTERNALS_DIR/assimp/lib/Release -lassimp-vc140-mt \
            -L$$EXTERNALS_DIR/LibCommon/Build -lLibCommon \
            -L$$EXTERNALS_DIR/nod/lib/Release -lnod \
            -L$$EXTERNALS_DIR/nod/logvisor/Release -llogvisor \
            -L$$EXTERNALS_DIR/zlib/lib/ -lzlib

    # Release Target Dependencies
    win32 {
        PRE_TARGETDEPS += $$EXTERNALS_DIR/LibCommon/Build/LibCommon.lib
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
    Render/CCamera.h \
    Render/CDrawUtil.h \
    Render/CGraphics.h \
    Render/CRenderBucket.h \
    Render/CRenderer.h \
    Render/ERenderCommand.h \
    Render/IRenderable.h \
    Render/SRenderablePtr.h \
    Render/SViewInfo.h \
    Resource/Area/CGameArea.h \
    Resource/Cooker/CMaterialCooker.h \
    Resource/Cooker/CModelCooker.h \
    Resource/Cooker/CSectionMgrOut.h \
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
    Resource/Factory/CTextureDecoder.h \
    Resource/Factory/CWorldLoader.h \
    Resource/Model/CBasicModel.h \
    Resource/Model/CModel.h \
    Resource/Model/CStaticModel.h \
    Resource/Model/CVertex.h \
    Resource/Model/SSurface.h \
    Resource/Script/CScriptLayer.h \
    Resource/Script/CScriptObject.h \
    Resource/Script/CScriptTemplate.h \
    Resource/Script/EVolumeShape.h \
    Resource/StringTable/CStringTable.h \
    Resource/CCollisionMesh.h \
    Resource/CCollisionMeshGroup.h \
    Resource/CFont.h \
    Resource/CLight.h \
    Resource/CMaterial.h \
    Resource/CMaterialPass.h \
    Resource/CMaterialSet.h \
    Resource/CResource.h \
    Resource/CTexture.h \
    Resource/CWorld.h \
    Resource/EResType.h \
    Resource/ETevEnums.h \
    Resource/ETexelFormat.h \
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
    SRayIntersection.h \
    OpenGL/CDynamicVertexBuffer.h \
    OpenGL/CFramebuffer.h \
    OpenGL/CIndexBuffer.h \
    OpenGL/CRenderbuffer.h \
    OpenGL/CShader.h \
    OpenGL/CShaderGenerator.h \
    OpenGL/CUniformBuffer.h \
    OpenGL/CVertexArrayManager.h \
    OpenGL/CVertexBuffer.h \
    OpenGL/GLCommon.h \
    ScriptExtra/CRadiusSphereExtra.h \
    Resource/Cooker/CAreaCooker.h \
    Resource/Model/EVertexAttribute.h \
    Render/FRenderOptions.h \
    Scene/FShowFlags.h \
    Scene/CScene.h \
    Scene/CSceneIterator.h \
    Resource/CPoiToWorld.h \
    Resource/Factory/CPoiToWorldLoader.h \
    Resource/Cooker/CPoiToWorldCooker.h \
    Resource/Factory/CSectionMgrIn.h \
    Resource/Cooker/CScriptCooker.h \
    ScriptExtra/CSplinePathExtra.h \
    Resource/Script/CLink.h \
    Resource/Factory/CSkeletonLoader.h \
    Scene/CCharacterNode.h \
    Resource/Factory/CAnimationLoader.h \
    Render/CBoneTransformData.h \
    Resource/Factory/CSkinLoader.h \
    Render/EDepthGroup.h \
    Scene/CScriptAttachNode.h \
    ScriptExtra/CSandwormExtra.h \
    Resource/CCollisionMaterial.h \
    GameProject/CGameProject.h \
    GameProject/CPackage.h \
    GameProject/CGameExporter.h \
    GameProject/CResourceStore.h \
    GameProject/CVirtualDirectory.h \
    GameProject/CResourceEntry.h \
    GameProject/CResourceIterator.h \
    Resource/CDependencyGroup.h \
    Resource/Factory/CDependencyGroupLoader.h \
    GameProject/CDependencyTree.h \
    Resource/Factory/CUnsupportedFormatLoader.h \
    Resource/Factory/CUnsupportedParticleLoader.h \
    Resource/Resources.h \
    Resource/Factory/CResourceFactory.h \
    GameProject/DependencyListBuilders.h \
    Resource/CAudioGroup.h \
    Resource/Factory/CAudioGroupLoader.h \
    Resource/CAudioLookupTable.h \
    Resource/CStringList.h \
    CAudioManager.h \
    Resource/Factory/CAnimEventLoader.h \
    Resource/AnimationClasses.h \
    Resource/Animation/CAnimation.h \
    Resource/Animation/CAnimationParameters.h \
    Resource/Animation/CAnimEventData.h \
    Resource/Animation/CAnimSet.h \
    Resource/Animation/CSkeleton.h \
    Resource/Animation/CSkin.h \
    Resource/Animation/IMetaTransition.h \
    Resource/Animation/IMetaAnimation.h \
    GameProject/CAssetNameMap.h \
    GameProject/AssetNameGeneration.h \
    GameProject/CGameInfo.h \
    Resource/CResTypeInfo.h \
    Resource/Cooker/CResourceCooker.h \
    Resource/CAudioMacro.h \
    CompressionUtil.h \
    Resource/Animation/CSourceAnimData.h \
    Resource/CMapArea.h \
    Resource/CSavedStateID.h \
    IProgressNotifier.h \
    IUIRelay.h \
    Resource/CResTypeFilter.h \
    GameProject/COpeningBanner.h \
    Resource/Script/Property/CPropertyNameGenerator.h \
    Resource/Script/Property/IProperty.h \
    Resource/Script/Property/CEnumProperty.h \
    Resource/Script/Property/CFlagsProperty.h \
    Resource/Script/Property/CAssetProperty.h \
    Resource/Script/Property/CPointerProperty.h \
    Resource/Script/Property/CArrayProperty.h \
    Resource/Script/Property/Properties.h \
    Resource/Script/Property/TPropertyRef.h \
    Resource/Script/Property/CBoolProperty.h \
    Resource/Script/Property/CByteProperty.h \
    Resource/Script/Property/CShortProperty.h \
    Resource/Script/Property/CIntProperty.h \
    Resource/Script/Property/CFloatProperty.h \
    Resource/Script/Property/CStringProperty.h \
    Resource/Script/Property/CSoundProperty.h \
    Resource/Script/Property/CAnimationProperty.h \
    Resource/Script/Property/CSequenceProperty.h \
    Resource/Script/Property/CSplineProperty.h \
    Resource/Script/Property/CAnimationSetProperty.h \
    Resource/Script/Property/CVectorProperty.h \
    Resource/Script/Property/CColorProperty.h \
    Resource/Script/Property/CStructProperty.h \
    Resource/Script/Property/CGuidProperty.h \
    Resource/Script/CGameTemplate.h \
    Resource/Script/NPropertyMap.h \
    Resource/Script/NGameList.h \
    Resource/StringTable/ELanguage.h \
    Tweaks/CTweakManager.h \
    Tweaks/CTweakData.h \
    Tweaks/CTweakLoader.h \
    Tweaks/CTweakCooker.h \
    Resource/Cooker/CStringCooker.h \
    Resource/Scan/CScan.h \
    Resource/Scan/SScanParametersMP1.h \
    Resource/Scan/ELogbookCategory.h \
    Resource/Cooker/CScanCooker.h \
    NCoreTests.h

# Source Files
SOURCES += \
    Render/CCamera.cpp \
    Render/CDrawUtil.cpp \
    Render/CGraphics.cpp \
    Render/CRenderer.cpp \
    Render/CRenderBucket.cpp \
    Resource/Area/CGameArea.cpp \
    Resource/Cooker/CMaterialCooker.cpp \
    Resource/Cooker/CModelCooker.cpp \
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
    Resource/Factory/CTextureDecoder.cpp \
    Resource/Factory/CWorldLoader.cpp \
    Resource/Model/CBasicModel.cpp \
    Resource/Model/CModel.cpp \
    Resource/Model/CStaticModel.cpp \
    Resource/Model/SSurface.cpp \
    Resource/Script/CScriptObject.cpp \
    Resource/Script/CScriptTemplate.cpp \
    Resource/CCollisionMesh.cpp \
    Resource/CFont.cpp \
    Resource/CLight.cpp \
    Resource/CMaterial.cpp \
    Resource/CMaterialPass.cpp \
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
    OpenGL/CDynamicVertexBuffer.cpp \
    OpenGL/CFramebuffer.cpp \
    OpenGL/CIndexBuffer.cpp \
    OpenGL/CShader.cpp \
    OpenGL/CShaderGenerator.cpp \
    OpenGL/CVertexArrayManager.cpp \
    OpenGL/CVertexBuffer.cpp \
    OpenGL/GLCommon.cpp \
    ScriptExtra/CRadiusSphereExtra.cpp \
    Resource/Cooker/CAreaCooker.cpp \
    Scene/FShowFlags.cpp \
    Scene/CScene.cpp \
    Scene/CSceneIterator.cpp \
    Resource/CPoiToWorld.cpp \
    Resource/Factory/CPoiToWorldLoader.cpp \
    Resource/Cooker/CPoiToWorldCooker.cpp \
    Resource/Cooker/CScriptCooker.cpp \
    ScriptExtra/CSplinePathExtra.cpp \
    Resource/Factory/CSkeletonLoader.cpp \
    Scene/CCharacterNode.cpp \
    Resource/Factory/CAnimationLoader.cpp \
    Resource/Factory/CSkinLoader.cpp \
    Resource/Model/EVertexAttribute.cpp \
    Scene/CScriptAttachNode.cpp \
    ScriptExtra/CSandwormExtra.cpp \
    Resource/CCollisionMaterial.cpp \
    GameProject/CGameProject.cpp \
    GameProject/CGameExporter.cpp \
    GameProject/CResourceStore.cpp \
    GameProject/CVirtualDirectory.cpp \
    GameProject/CResourceEntry.cpp \
    GameProject/CPackage.cpp \
    Resource/Factory/CDependencyGroupLoader.cpp \
    GameProject/CDependencyTree.cpp \
    Resource/Factory/CUnsupportedFormatLoader.cpp \
    Resource/Factory/CUnsupportedParticleLoader.cpp \
    GameProject/DependencyListBuilders.cpp \
    Resource/Factory/CAudioGroupLoader.cpp \
    CAudioManager.cpp \
    Resource/Factory/CAnimEventLoader.cpp \
    Resource/Animation/CAnimation.cpp \
    Resource/Animation/CAnimationParameters.cpp \
    Resource/Animation/CSkeleton.cpp \
    Resource/Animation/IMetaAnimation.cpp \
    Resource/Animation/IMetaTransition.cpp \
    GameProject/AssetNameGeneration.cpp \
    GameProject/CAssetNameMap.cpp \
    GameProject/CGameInfo.cpp \
    Resource/CResTypeInfo.cpp \
    CompressionUtil.cpp \
    IUIRelay.cpp \
    GameProject\COpeningBanner.cpp \
    IProgressNotifier.cpp \
    Resource/Script/Property/CPropertyNameGenerator.cpp \
    Resource/Script/Property/IProperty.cpp \
    Resource/Script/Property/CStructProperty.cpp \
    Resource/Script/Property/CFlagsProperty.cpp \
    Resource/Script/CGameTemplate.cpp \
    Resource/Script/NPropertyMap.cpp \
    Resource/Script/NGameList.cpp \
    Resource/StringTable/CStringTable.cpp \
    Tweaks/CTweakManager.cpp \
    Tweaks/CTweakLoader.cpp \
    Tweaks/CTweakCooker.cpp \
    Resource/Cooker/CStringCooker.cpp \
    Resource/Scan/CScan.cpp \
    Resource/Cooker/CScanCooker.cpp \
    NCoreTests.cpp

# Codegen
CODEGEN_DIR = $$EXTERNALS_DIR/CodeGen
CODEGEN_OUT_PATH = $$BUILD_DIR/Core/codegen_build/auto_codegen.cpp
CODEGEN_SRC_PATH = $$PWD
include($$EXTERNALS_DIR/CodeGen/codegen.pri)

# Library Sources
SOURCES += $$EXTERNALS_DIR/lzo/src/lzo_init.c \
           $$EXTERNALS_DIR/lzo/src/lzo1x_9x.c \
           $$EXTERNALS_DIR/lzo/src/lzo1x_d1.c
