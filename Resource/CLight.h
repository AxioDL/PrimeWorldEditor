#ifndef CLIGHT_H
#define CLIGHT_H

#include <FileIO/CInputStream.h>
#include <Common/CColor.h>
#include <Common/CVector3f.h>
#include <GL/glew.h>

/**
 * CLight is currently heavily based on the lights system from Metroid Prime,
 * including code reverse engineered from the game's executable. Not yet sure
 * how much needs to be modified to properly support Prime 3 and DKCR; they
 * have a new light structure.
 */
enum ELightType
{
    eLocalAmbient = 0,
    eDirectional = 1,
    eSpot = 3,
    eCustom = 2
};

class CLight
{
    ELightType mType;
    CVector3f mPosition;
    CVector3f mDirection;
    CColor mColor;
    float mSpotCutoff;
    CVector3f mDistAttenCoefficients;
    CVector3f mAngleAttenCoefficients;
    float mRadius;
    float mIntensity;
    u8 mFlags;

public:
    CLight();

private:
    // Data Manipulation
    float CalculateRadius();
    float CalculateIntensity();
    CVector3f CalculateSpotAngleAtten();

public:
    // Getters
    ELightType GetType() const;
    CVector3f GetPosition() const;
    CVector3f GetDirection() const;
    CColor GetColor() const;
    CVector3f GetDistAttenuation() const;
    CVector3f GetAngleAttenuation() const;
    float GetRadius();
    float GetIntensity();

    // Setters
    void SetPosition(const CVector3f& Position);
    void SetDirection(const CVector3f& Direction);
    void SetColor(const CColor& Color);
    void SetSpotCutoff(float Cutoff);
    void SetDistAtten(float DistCoefA, float DistCoefB, float DistCoefC);
    void SetAngleAtten(float AngleCoefA, float AngleCoefB, float AngleCoefC);

    // Other
    void Load() const;

    // Static
    static CLight* BuildLocalAmbient(const CVector3f& Position, const CColor& Color);
    static CLight* BuildDirectional(const CVector3f& Position, const CVector3f& Direction, const CColor& Color);
    static CLight* BuildSpot(const CVector3f& Position, const CVector3f& Direction, const CColor& Color, float Cutoff);
    static CLight* BuildCustom(const CVector3f& Position, const CVector3f& Direction, const CColor& Color,
                              float DistAttenA, float DistAttenB, float DistAttenC,
                              float AngleAttenA, float AngleAttenB, float AngleAttenC);

    // Constants
    static const CVector3f skDefaultLightPos;
    static const CVector3f skDefaultLightDir;
};

#endif // CLIGHT_H
