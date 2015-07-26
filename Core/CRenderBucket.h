#ifndef CRENDERBUCKET_H
#define CRENDERBUCKET_H

#include "CCamera.h"
#include "ERenderOptions.h"
#include <Common/types.h>
#include <OpenGL/SMeshPointer.h>
#include <vector>

class CRenderBucket
{
public:
    enum ESortType
    {
        BackToFront,
        FrontToBack
    };
private:
    ESortType mSortType;
    std::vector<SMeshPointer> mNodes;
    u32 mEstSize;
    u32 mSize;

public:
    CRenderBucket();
    void SetSortType(ESortType Type);
    void Add(const SMeshPointer& Mesh);
    void Sort(CCamera& Camera);
    void Clear();
    void Draw(ERenderOptions Options);
};

#endif // CRENDERBUCKET_H
