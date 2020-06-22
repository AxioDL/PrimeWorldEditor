#include "CLight.h"
#include "Core/Render/CGraphics.h"
#include <Common/Common.h>
#include <cfloat>
#include <cmath>

constexpr uint32_t CLIGHT_NO_RADIUS = 0x40;
constexpr uint32_t CLIGHT_NO_INTENSITY = 0x80;

CLight::CLight()
    : mDirtyFlags(CLIGHT_NO_RADIUS | CLIGHT_NO_INTENSITY)
{
}

// ************ DATA MANIPULATION ************

// This function is reverse engineered from the kiosk demo's code
float CLight::CalculateRadius() const
{
    if (mDistAttenCoefficients.Y < FLT_EPSILON && mDistAttenCoefficients.Z < FLT_EPSILON)
        return FLT_MAX;

    float Intensity = GetIntensity();
    if (mDistAttenCoefficients.Z > FLT_EPSILON) {
        if (Intensity <= FLT_EPSILON)
            return 0.f;
        return std::sqrt(Intensity / (0.0588235f * mDistAttenCoefficients.Z));
    }

    if (mDistAttenCoefficients.Y > FLT_EPSILON)
        return Intensity / (0.0588235f * mDistAttenCoefficients.Y);
    return 0.f;

#if 0
    if ((mDistAttenCoefficients.Y >= FLT_EPSILON) ||
        (mDistAttenCoefficients.Z >= FLT_EPSILON))
    {
        float Intensity = GetIntensity();

        if (mDistAttenCoefficients.Z > FLT_EPSILON)
        {
            if (Intensity <= FLT_EPSILON)
                return 0.f;

            float IntensityMod = (Intensity * 5.f / 255.f * mDistAttenCoefficients.Z);
            return sqrtf(Intensity / IntensityMod);
        }

        else
        {
            if (mDistAttenCoefficients.Y <= FLT_EPSILON)
                return 0.f;

            float IntensityMod = (Intensity * 5.f) / 255.f;
            if (IntensityMod < 0.2f)
                IntensityMod = 0.2f;

            return Intensity / (IntensityMod * mDistAttenCoefficients.Y);
        }
    }

    else return 3000000000000000000000000000000000000.f;
#endif
}

// This function is also reverse engineered from the kiosk demo's code
float CLight::CalculateIntensity() const
{
    float coef = 1.f;
    if (mType == ELightType::Custom)
        coef = mAngleAttenCoefficients.X;

    return coef * std::max({mColor.R, mColor.G, mColor.B});
#if 0
    // Get the color component with the greatest numeric value
    float Greatest = (mColor.G >= mColor.B) ? mColor.G : mColor.B;
    Greatest = (mColor.R >= Greatest) ? mColor.R : Greatest;

    float Multiplier = (mType == ELightType::Custom) ? mAngleAttenCoefficients.X : 1.0f;
    return Greatest * Multiplier;
#endif
}

// As is this one... partly
CVector3f CLight::CalculateSpotAngleAtten() const
{
    if (mType != ELightType::Spot)
        return CVector3f(1.f, 0.f, 0.f);

    const float RadianCutoff = mSpotCutoff * (3.1415927f / 180.f);
    const float RadianCosine = cosf(RadianCutoff);
    const float InvCosine = 1.f - RadianCosine;

    return CVector3f(0.f, -RadianCosine / InvCosine, 1.f / InvCosine);
}

// ************ ACCESSORS ************
float CLight::GetRadius() const
{
    if ((mDirtyFlags & CLIGHT_NO_RADIUS) != 0)
    {
        mCachedRadius = CalculateRadius();
        mDirtyFlags &= ~CLIGHT_NO_RADIUS;
    }

    return mCachedRadius;
}

float CLight::GetIntensity() const
{
    if ((mDirtyFlags & CLIGHT_NO_INTENSITY) != 0)
    {
        mCachedIntensity = CalculateIntensity();
        mDirtyFlags &= ~CLIGHT_NO_INTENSITY;
    }

    return mCachedIntensity;
}

void CLight::SetColor(const CColor& rkColor)
{
    mColor = rkColor;
    mDirtyFlags = CLIGHT_NO_RADIUS | CLIGHT_NO_INTENSITY;
}

void CLight::SetSpotCutoff(float Cutoff)
{
    mSpotCutoff = Cutoff * 0.5f;
    mAngleAttenCoefficients = CalculateSpotAngleAtten();
}

void CLight::SetDistAtten(float DistCoefA, float DistCoefB, float DistCoefC)
{
    mDistAttenCoefficients.X = DistCoefA;
    mDistAttenCoefficients.Y = DistCoefB;
    mDistAttenCoefficients.Z = DistCoefC;
}

void CLight::SetAngleAtten(float AngleCoefA, float AngleCoefB, float AngleCoefC)
{
    mAngleAttenCoefficients.X = AngleCoefA;
    mAngleAttenCoefficients.Y = AngleCoefB;
    mAngleAttenCoefficients.Z = AngleCoefC;
}

CStructProperty* CLight::GetProperties() const
{
    //@todo MP1 properties only
    //@todo we cannot display full properties because a lot of them are discarded on load
    static CStructProperty* pProperties = nullptr;

    if (!pProperties)
    {
        pProperties = (CStructProperty*) IProperty::CreateIntrinsic(EPropertyType::Struct,
                                                                          EGame::Prime,
                                                                          0,
                                                                          "Light");

        //@todo it would be really cool if the property could detect all possible values automatically from TEnumReflection
        CChoiceProperty* pLightType = (CChoiceProperty*) IProperty::CreateIntrinsic(EPropertyType::Choice,
                                                                                       pProperties,
                                                                                       MEMBER_OFFSET(CLight, mType),
                                                                                       "LightType");
        pLightType->AddValue("LocalAmbient", (uint32) ELightType::LocalAmbient);
        pLightType->AddValue("Directional", (uint32) ELightType::Directional);
        pLightType->AddValue("Spot", (uint32) ELightType::Spot);
        pLightType->AddValue("Custom", (uint32) ELightType::Custom);

        IProperty::CreateIntrinsic(EPropertyType::Color,
                                      pProperties,
                                      MEMBER_OFFSET(CLight, mColor),
                                      "Color");

        IProperty::CreateIntrinsic(EPropertyType::Vector,
                                      pProperties,
                                      MEMBER_OFFSET(CLight, mPosition),
                                      "Position");

        IProperty::CreateIntrinsic(EPropertyType::Vector,
                                      pProperties,
                                      MEMBER_OFFSET(CLight, mDirection),
                                      "Direction");

        IProperty::CreateIntrinsic(EPropertyType::Float,
                                      pProperties,
                                      MEMBER_OFFSET(CLight, mSpotCutoff),
                                      "SpotCutoff");
    }

    return pProperties;
}

// ************ OTHER ************
void CLight::Load() const
{
    const auto Index = static_cast<uint8>(CGraphics::sNumLights);
    if (Index >= CGraphics::sLightBlock.Lights.size())
        return;

    CGraphics::SLightBlock::SGXLight *pLight = &CGraphics::sLightBlock.Lights[Index];

    switch (mType)
    {
    case ELightType::LocalAmbient:
        // LocalAmbient is already accounted for in CGraphics::sAreaAmbientColor
        return;
    case ELightType::Directional:
        pLight->Position = CVector4f(-mDirection * 1048576.f, 1.f);
        pLight->Direction = CVector4f(mDirection, 0.f);
        pLight->Color = mColor * CGraphics::sWorldLightMultiplier;
        pLight->DistAtten = CVector4f(1.f, 0.f, 0.f, 0.f);
        pLight->AngleAtten = CVector4f(1.f, 0.f, 0.f, 0.f);
        break;
    case ELightType::Spot:
        pLight->Position = CVector4f(mPosition,  1.f);
        pLight->Direction = CVector4f(mDirection, 0.f);
        pLight->Color = mColor * CGraphics::sWorldLightMultiplier;
        pLight->DistAtten = mDistAttenCoefficients;
        pLight->AngleAtten = mAngleAttenCoefficients;
        break;
    case ELightType::Custom:
        pLight->Position = CVector4f(mPosition,  1.f);
        pLight->Direction = CVector4f(mDirection, 0.f);
        pLight->Color = mColor * CGraphics::sWorldLightMultiplier;
        pLight->DistAtten = mDistAttenCoefficients;
        pLight->AngleAtten = mAngleAttenCoefficients;
        break;
    default:
        return;
    }
    CGraphics::sNumLights++;
}

// ************ STATIC ************
CLight CLight::BuildLocalAmbient(const CVector3f& rkPosition, const CColor& rkColor)
{
    CLight pLight;
    pLight.mType = ELightType::LocalAmbient;
    pLight.mPosition = rkPosition;
    pLight.mDirection = skDefaultLightDir;
    pLight.mColor = rkColor;
    pLight.mSpotCutoff = 180.f;
    return pLight;
}

CLight CLight::BuildDirectional(const CVector3f& rkPosition, const CVector3f& rkDirection, const CColor& rkColor)
{
    CLight pLight;
    pLight.mType = ELightType::Directional;
    pLight.mPosition = rkPosition;
    pLight.mDirection = rkDirection;
    pLight.mColor = rkColor;
    pLight.mSpotCutoff = 180.f;
    return pLight;
}

CLight CLight::BuildSpot(const CVector3f& rkPosition, const CVector3f& rkDirection, const CColor& rkColor, float Cutoff)
{
    CLight pLight;
    pLight.mType = ELightType::Spot;
    pLight.mPosition = rkPosition;
    pLight.mDirection = -rkDirection.Normalized();
    pLight.mColor = rkColor;
    pLight.mSpotCutoff = Cutoff;
    pLight.mAngleAttenCoefficients = pLight.CalculateSpotAngleAtten();
    return pLight;
}

CLight CLight::BuildCustom(const CVector3f& rkPosition, const CVector3f& rkDirection, const CColor& rkColor,
                           float DistAttenA, float DistAttenB, float DistAttenC,
                           float AngleAttenA, float AngleAttenB, float AngleAttenC)
{
    CLight pLight;
    pLight.mType = ELightType::Custom;
    pLight.mPosition = rkPosition;
    pLight.mDirection = rkDirection;
    pLight.mColor = rkColor;
    pLight.mSpotCutoff = 0.f;
    pLight.mDistAttenCoefficients.X = DistAttenA;
    pLight.mDistAttenCoefficients.Y = DistAttenB;
    pLight.mDistAttenCoefficients.Z = DistAttenC;
    pLight.mAngleAttenCoefficients.X = AngleAttenA;
    pLight.mAngleAttenCoefficients.Y = AngleAttenB;
    pLight.mAngleAttenCoefficients.Z = AngleAttenC * AngleAttenC;
    return pLight;
}
