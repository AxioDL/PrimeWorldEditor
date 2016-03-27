#ifndef SRENDERABLEPTR_H
#define SRENDERABLEPTR_H

#include "ERenderCommand.h"
#include "IRenderable.h"
#include <Common/types.h>
#include <Math/CAABox.h>

struct SRenderablePtr
{
    IRenderable *pRenderable;
    u32 ComponentIndex;
    CAABox AABox;
    ERenderCommand Command;
};

#endif // SRENDERABLEPTR_H
