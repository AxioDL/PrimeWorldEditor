#ifndef SRENDERABLEPTR_H
#define SRENDERABLEPTR_H

#include "ERenderCommand.h"
#include "IRenderable.h"
#include <Common/BasicTypes.h>
#include <Common/Math/CAABox.h>

struct SRenderablePtr
{
    IRenderable *pRenderable;
    uint32 ComponentIndex;
    CAABox AABox;
    ERenderCommand Command;
};

#endif // SRENDERABLEPTR_H
