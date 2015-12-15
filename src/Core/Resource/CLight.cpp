#include "CLight.h"
#include <cmath>
#include <float.h>
#include <Core/CGraphics.h>

#define CLIGHT_NO_RADIUS 0x40
#define CLIGHT_NO_INTENSITY 0x80

CLight::CLight()
{
    mPosition = skDefaultLightPos;
    mDirection = skDefaultLightDir;
    mDistAttenCoefficients = CVector3f(0.f, 1.f, 0.f);
    mAngleAttenCoefficients = CVector3f(0.f, 1.f, 0.f);
    mCachedRadius = 0.f;
    mCachedIntensity = 0.f;
    mDirtyFlags = CLIGHT_NO_RADIUS | CLIGHT_NO_INTENSITY;
}

// ************ DATA MANIPULATION ************

// This function is reverse engineered from the kiosk demo's code
float CLight::CalculateRadius() const
{
    if ((mDistAttenCoefficients.y >= FLT_EPSILON) ||
        (mDistAttenCoefficients.z >= FLT_EPSILON))
    {
        float Intensity = GetIntensity();

        if (mDistAttenCoefficients.z > FLT_EPSILON)
        {
            if (Intensity <= FLT_EPSILON)
                return 0.f;

            float IntensityMod = (Intensity * 5.f / 255.f * mDistAttenCoefficients.z);
            return sqrt(Intensity / IntensityMod);
        }

        else
        {
            if (mDistAttenCoefficients.y <= FLT_EPSILON)
                return 0.f;

            float IntensityMod = (Intensity * 5.f) / 255.f;
            if (IntensityMod < 0.2f)
                IntensityMod = 0.2f;

            return Intensity / (IntensityMod * mDistAttenCoefficients.y);
        }
    }

    else return 3000000000000000000000000000000000000.f;
}

// This function is also reverse engineered from the kiosk demo's code
float CLight::CalculateIntensity() const
{
    float Multiplier = (mType == eCustom) ? mAngleAttenCoefficients.x : 1.0f;
    float ColorR = float(mColor.r) / 255.f;
    float ColorG = float(mColor.g) / 255.f;
    float ColorB = float(mColor.b) / 255.f;

    // Get the color component with the greatest numeric value
    float Greatest = (ColorG >= ColorB) ? ColorG : ColorB;
    Greatest = (ColorR >= Greatest) ? ColorR : Greatest;

    return Greatest * Multiplier;
}

// As is this one... partly
CVector3f CLight::CalculateSpotAngleAtten()
{
    if (mType != eSpot) return CVector3f(1.f, 0.f, 0.f);

    if ((mSpotCutoff < 0.f) || (mSpotCutoff > 90.f))
        return CVector3f(1.f, 0.f, 0.f);

    float RadianCutoff = (mSpotCutoff * 3.1415927f) / 180.f;
    float RadianCosine = cosf(RadianCutoff);
    float InvCosine = 1.f - RadianCosine;

    return CVector3f(0.f, -RadianCosine / InvCosine, 1.f / InvCosine);
}

// ************ GETTERS ************
ELightType CLight::GetType() const
{
    return mType;
}

u32 CLight::GetLayerIndex() const
{
    return mLayerIndex;
}

CVector3f CLight::GetPosition() const
{
    return mPosition;
}

CVector3f CLight::GetDirection() const
{
    return mDirection;
}

CColor CLight::GetColor() const
{
    return mColor;
}

CVector3f CLight::GetDistAttenuation() const
{
    return mDistAttenCoefficients;
}

CVector3f CLight::GetAngleAttenuation() const
{
    return mAngleAttenCoefficients;
}

float CLight::GetRadius() const
{
    if (mDirtyFlags & CLIGHT_NO_RADIUS)
    {
        mCachedRadius = CalculateRadius();
        mDirtyFlags &= ~CLIGHT_NO_RADIUS;
    }

    return mCachedRadius * 2;
}

float CLight::GetIntensity() const
{
    if (mDirtyFlags & CLIGHT_NO_INTENSITY)
    {
        mCachedIntensity = CalculateIntensity();
        mDirtyFlags &= ~CLIGHT_NO_INTENSITY;
    }

    return mCachedIntensity;
}

// ************ SETTERS ************
void CLight::SetLayer(u32 index)
{
    mLayerIndex = index;
}

void CLight::SetPosition(const CVector3f& Position)
{
    mPosition = Position;
}

void CLight::SetDirection(const CVector3f& Direction)
{
    mDirection = Direction;
}

void CLight::SetColor(const CColor& Color)
{
    mColor = Color;
    mDirtyFlags = CLIGHT_NO_RADIUS | CLIGHT_NO_INTENSITY;
}

void CLight::SetSpotCutoff(float Cutoff)
{
    mSpotCutoff = Cutoff * 0.5f;
    CalculateSpotAngleAtten();
}

void CLight::SetDistAtten(float DistCoefA, float DistCoefB, float DistCoefC)
{
    mDistAttenCoefficients.x = DistCoefA;
    mDistAttenCoefficients.y = DistCoefB;
    mDistAttenCoefficients.z = DistCoefC;
}

void CLight::SetAngleAtten(float AngleCoefA, float AngleCoefB, float AngleCoefC)
{
    mAngleAttenCoefficients.x = AngleCoefA;
    mAngleAttenCoefficients.y = AngleCoefB;
    mAngleAttenCoefficients.z = AngleCoefC;
}

// ************ OTHER ************
void CLight::Load() const
{
    u8 Index = (u8) CGraphics::sNumLights;
    if (Index >= 8) return;

    CGraphics::SLightBlock::SGXLight *Light = &CGraphics::sLightBlock.Lights[Index];
    CVector3f PosView = CGraphics::sMVPBlock.ViewMatrix * mPosition;
    CVector3f DirView = CGraphics::sMVPBlock.ViewMatrix * mDirection;

    switch (mType)
    {
    case eLocalAmbient:
        // LocalAmbient is already accounted for in CGraphics::sAreaAmbientColor
        return;
    case eDirectional:
        Light->Position = CVector4f(-mDirection * 1048576.f, 1.f);
        Light->Direction = CVector4f(mDirection, 0.f);
        Light->Color = mColor.ToVector4f() * CGraphics::sWorldLightMultiplier;
        Light->DistAtten = CVector4f(1.f, 0.f, 0.f, 0.f);
        Light->AngleAtten = CVector4f(1.f, 0.f, 0.f, 0.f);
        break;
    case eSpot:
        Light->Position = CVector4f(mPosition,  1.f);
        Light->Direction = CVector4f(mDirection, 0.f);
        Light->Color = mColor.ToVector4f() * CGraphics::sWorldLightMultiplier;
        Light->DistAtten = mDistAttenCoefficients;
        Light->AngleAtten = mAngleAttenCoefficients;
        break;
    case eCustom:
        Light->Position = CVector4f(mPosition,  1.f);
        Light->Direction = CVector4f(mDirection, 0.f);
        Light->Color = mColor.ToVector4f() * CGraphics::sWorldLightMultiplier;
        Light->DistAtten = mDistAttenCoefficients;
        Light->AngleAtten = mAngleAttenCoefficients;
        break;
    default:
        return;
    }
    CGraphics::sNumLights++;
}

// ************ STATIC ************
CLight* CLight::BuildLocalAmbient(const CVector3f& Position, const CColor& Color)
{
    CLight *Light = new CLight;
    Light->mType = eLocalAmbient;
    Light->mPosition = Position;
    Light->mDirection = skDefaultLightDir;
    Light->mColor = Color;
    Light->mSpotCutoff = 0.f;
    return Light;
}

CLight* CLight::BuildDirectional(const CVector3f& Position, const CVector3f& Direction, const CColor& Color)
{
    CLight *Light = new CLight;
    Light->mType = eDirectional;
    Light->mPosition = Position;
    Light->mDirection = Direction;
    Light->mColor = Color;
    Light->mSpotCutoff = 0.f;
    return Light;
}

CLight* CLight::BuildSpot(const CVector3f& Position, const CVector3f& Direction, const CColor& Color, float Cutoff)
{
    CLight *Light = new CLight;
    Light->mType = eSpot;
    Light->mPosition = Position;
    Light->mDirection = -Direction.Normalized();
    Light->mColor = Color;
    Light->mSpotCutoff = Cutoff * 0.5f;
    Light->mAngleAttenCoefficients = Light->CalculateSpotAngleAtten();
    return Light;
}

CLight* CLight::BuildCustom(const CVector3f& Position, const CVector3f& Direction, const CColor& Color,
                                  float DistAttenA, float DistAttenB, float DistAttenC,
                                  float AngleAttenA, float AngleAttenB, float AngleAttenC)
{
    CLight *Light = new CLight;
    Light->mType = eCustom;
    Light->mPosition = Position;
    Light->mDirection = Direction;
    Light->mColor = Color;
    Light->mSpotCutoff = 0.f;
    Light->mDistAttenCoefficients.x = DistAttenA;
    Light->mDistAttenCoefficients.y = DistAttenB;
    Light->mDistAttenCoefficients.z = DistAttenC;
    Light->mAngleAttenCoefficients.x = AngleAttenA;
    Light->mAngleAttenCoefficients.y = AngleAttenB;
    Light->mAngleAttenCoefficients.z = AngleAttenC * AngleAttenC;
    return Light;
}

// ************ CONSTANTS ************
const CVector3f CLight::skDefaultLightPos(0.f, 0.f, 0.f);
const CVector3f CLight::skDefaultLightDir(0.f,-1.f, 0.f);
