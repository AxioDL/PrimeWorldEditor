#ifndef SMESHPOINTER_H
#define SMESHPOINTER_H

#include <Common/CAABox.h>
#include <Common/types.h>
#include <Core/ERenderCommand.h>
#include <Scene/CSceneNode.h>
#include <Resource/CMaterial.h>

struct SMeshPointer
{
    CSceneNode *pNode;
    u32 Asset;
    CAABox AABox;
    ERenderCommand Command;
};

#endif // SMESHPOINTER_H
