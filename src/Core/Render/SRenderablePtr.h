#ifndef SRENDERABLEPTR_H
#define SRENDERABLEPTR_H

#include "ERenderCommand.h"
#include "Core/Resource/CMaterial.h"
#include "Core/Scene/CSceneNode.h"
#include <Common/Math/CAABox.h>
#include <Common/types.h>

struct SRenderablePtr
{
    IRenderable *pRenderable;
    u32 ComponentIndex;
    CAABox AABox;
    ERenderCommand Command;
};

#endif // SRENDERABLEPTR_H
