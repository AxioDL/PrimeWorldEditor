#ifndef CLIGHT_H
#define CLIGHT_H

#include "Core/Resource/Script/Property/Properties.h"
#include <Common/CColor.h>
#include <Common/FileIO/IInputStream.h>
#include <Common/Math/CVector3f.h>

/* CLight is currently heavily based on the lights system from Metroid Prime,
 * including code reverse engineered from the game's executable. Not yet sure
 * how much needs to be modified to properly support DKCR. */
enum class ELightType
{
    LocalAmbient = 0,
    Directional = 1,
    Spot = 3,
    Custom = 2
};

class CLight
{
    ELightType mType{};
    uint32 mLayerIndex = 0;
    CVector3f mPosition{skDefaultLightPos};
    CVector3f mDirection{skDefaultLightDir};
    CColor mColor;
    float mSpotCutoff = 0.0f;
    CVector3f mDistAttenCoefficients{0.f, 1.f, 0.f};
    CVector3f mAngleAttenCoefficients{0.f, 1.f, 0.f};

    mutable float mCachedRadius = 0.0f;
    mutable float mCachedIntensity = 0.0f;
    mutable uint8 mDirtyFlags;

public:
    CLight();

private:
    // Data Manipulation
    float CalculateRadius() const;
    float CalculateIntensity() const;
    CVector3f CalculateSpotAngleAtten() const;

public:
    // Accessors
    ELightType Type() const              { return mType; }
    uint32 LayerIndex() const            { return mLayerIndex; }
    CVector3f Position() const           { return mPosition; }
    CVector3f Direction() const          { return mDirection; }
    CColor Color() const                 { return mColor; }
    CVector3f DistAttenuation() const    { return mDistAttenCoefficients; }
    CVector3f AngleAttenuation() const   { return mAngleAttenCoefficients; }

    void SetLayer(uint32 Index)                      { mLayerIndex = Index; }
    void SetPosition(const CVector3f& rkPosition)    { mPosition = rkPosition; }
    void SetDirection(const CVector3f& rkDirection)  { mDirection = rkDirection; }

    float GetRadius() const;
    float GetIntensity() const;

    void SetColor(const CColor& rkColor);
    void SetSpotCutoff(float Cutoff);
    void SetDistAtten(float DistCoefA, float DistCoefB, float DistCoefC);
    void SetAngleAtten(float AngleCoefA, float AngleCoefB, float AngleCoefC);

    CStructProperty* GetProperties() const;

    // Other
    void Load() const;

    // Static
    static CLight BuildLocalAmbient(const CVector3f& rkPosition, const CColor& rkColor);
    static CLight BuildDirectional(const CVector3f& rkPosition, const CVector3f& rkDirection, const CColor& rkColor);
    static CLight BuildSpot(const CVector3f& rkPosition, const CVector3f& rkDirection, const CColor& rkColor, float Cutoff);
    static CLight BuildCustom(const CVector3f& rkPosition, const CVector3f& rkDirection, const CColor& rkColor,
                              float DistAttenA, float DistAttenB, float DistAttenC,
                              float AngleAttenA, float AngleAttenB, float AngleAttenC);

    // Constants
    static constexpr CVector3f skDefaultLightPos{0.f, 0.f, 0.f};
    static constexpr CVector3f skDefaultLightDir{0.f, -1.f, 0.f};
};

#endif // CLIGHT_H
