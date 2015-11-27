#ifndef SRENDERABLEPTR_H
#define SRENDERABLEPTR_H

#include <Common/CAABox.h>
#include <Common/types.h>
#include <Core/ERenderCommand.h>
#include <Scene/CSceneNode.h>
#include <Resource/CMaterial.h>

struct SRenderablePtr
{
    IRenderable *pRenderable;
    u32 ComponentIndex;
    CAABox AABox;
    ERenderCommand Command;
};

#endif // SRENDERABLEPTR_H
