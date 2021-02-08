#ifndef SSCANPARAMETERSMP1_H
#define SSCANPARAMETERSMP1_H

#include "ELogbookCategory.h"
#include <Common/Common.h>
#include <array>

/** Struct mapping to SCAN property layout in MP1 */
enum class EScanSpeed
{
    Normal = 0,
    Slow = 1,
};

enum class EScanImagePane
{
    Pane0 = 0,
    Pane1 = 1,
    Pane2 = 2,
    Pane3 = 3,
    Pane01 = 4,
    Pane12 = 5,
    Pane23 = 6,
    Pane012 = 7,
    Pane123 = 8,
    Pane0123 = 9,
    Pane4 = 10,
    Pane5 = 11,
    Pane6 = 12,
    Pane7 = 13,
    Pane45 = 14,
    Pane56 = 15,
    Pane67 = 16,
    Pane456 = 17,
    Pane567 = 18,
    Pane4567 = 19,
    None = -1
};

struct SScanImage
{
    CAssetID        Texture;
    float           AppearPercentage;
    EScanImagePane  Pane;
    int32           AnimationCellWidth;
    int32           AnimationCellHeight;
    float           AnimationSwapInterval;
    float           FadeDuration;
};

struct SScanParametersMP1
{
    CAssetID                  GuiFrame;
    CAssetID                  String;
    EScanSpeed                Speed;
    ELogbookCategory          LogbookCategory;
    bool                      IsCritical;
    std::array<SScanImage, 4> ScanImages;
};

#endif // SSCANPARAMETERSMP1_H
