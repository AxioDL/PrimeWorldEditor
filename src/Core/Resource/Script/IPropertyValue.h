#ifndef IPROPERTYVALUE_H
#define IPROPERTYVALUE_H

#include "EPropertyType.h"
#include "Core/Log.h"
#include "Core/Resource/CAnimationParameters.h"
#include "Core/Resource/CResource.h"
#include "Core/Resource/CResourceInfo.h"
#include "Core/Resource/TResPtr.h"

#include <Common/CColor.h>
#include <Common/TString.h>
#include <Math/CVector3f.h>

class IPropertyValue
{
public:
    virtual TString ToString() const = 0;
    virtual void FromString(const TString& rkString) = 0;
};

template<typename PropType>
class TTypedPropertyValue : public IPropertyValue
{
protected:
    PropType mValue;

public:
    TTypedPropertyValue() {}

    TTypedPropertyValue(PropType Val)
        : mValue(Val) {}

    PropType Get() const
    {
        return mValue;
    }

    void Set(const PropType& rkIn)
    {
        mValue = rkIn;
    }

    bool operator==(const TTypedPropertyValue& rkOther) const
    {
        return (mValue == rkOther.mValue);
    }

    bool operator==(const PropType& rkOther) const { return (mValue == rkOther); }
    bool operator!=(const PropType& rkOther) const { return (mValue != rkOther); }
    bool operator< (const PropType& rkOther) const { return (mValue <  rkOther); }
    bool operator<=(const PropType& rkOther) const { return (mValue <= rkOther); }
    bool operator> (const PropType& rkOther) const { return (mValue >  rkOther); }
    bool operator>=(const PropType& rkOther) const { return (mValue >= rkOther); }
};

class CBoolValue : public TTypedPropertyValue<bool>
{
public:
    CBoolValue()         { mValue = false; }
    CBoolValue(bool Val) { mValue = Val; }

    TString ToString() const
    {
        return (!mValue ? "false" : "true");
    }

    void FromString(const TString& rkString)
    {
        mValue = (rkString == "true");
    }
};

class CByteValue : public TTypedPropertyValue<s8>
{
public:
    CByteValue()       { mValue = 0; }
    CByteValue(s8 Val) { mValue = Val; }

    TString ToString() const
    {
        return TString::FromInt32(mValue, 0, 10);
    }

    void FromString(const TString& rkString)
    {
        u32 base = (rkString.StartsWith("0x") ? 16 : 10);
        mValue = (s8) rkString.ToInt32(base);
    }
};

class CShortValue : public TTypedPropertyValue<s16>
{
public:
    CShortValue()        { mValue = 0; }
    CShortValue(s16 Val) { mValue = Val; }

    TString ToString() const
    {
        return TString::FromInt32((s32) mValue, 0, 10);
    }

    void FromString(const TString& rkString)
    {
        u32 base = (rkString.StartsWith("0x") ? 16 : 10);
        mValue = (s16) rkString.ToInt32(base);
    }
};

class CLongValue : public TTypedPropertyValue<s32>
{
protected:
    bool mShouldOutputHex;

public:
    CLongValue()        { mShouldOutputHex = false; mValue = 0; }
    CLongValue(s32 Val) { mShouldOutputHex = false; mValue = Val; }

    void SetHexStringOutput(bool enable)
    {
        mShouldOutputHex = enable;
    }

    TString ToString() const
    {
        if (mShouldOutputHex)
            return TString::HexString((u32) mValue, true, true, 8);
        else
            return TString::FromInt32(mValue, 0, 10);
    }

    void FromString(const TString& rkString)
    {
        u32 base = (rkString.StartsWith("0x") ? 16 : 10);
        mValue = (s32) rkString.ToInt32(base);
    }
};

class CFloatValue : public TTypedPropertyValue<float>
{
public:
    CFloatValue()          { mValue = 0.0f; }
    CFloatValue(float Val) { mValue = Val; }

    TString ToString() const
    {
        return TString::FromFloat(mValue);
    }

    void FromString(const TString& rkString)
    {
        mValue = rkString.ToFloat();
    }
};

class CStringValue : public TTypedPropertyValue<TString>
{
public:
    CStringValue() {}
    CStringValue(const TString& rkVal) { mValue = rkVal; }

    // These functions are extremely complicated, but try to follow along
    TString ToString() const
    {
        return mValue;
    }

    void FromString(const TString& rkString)
    {
        mValue = rkString;
    }
};

class CColorValue : public TTypedPropertyValue<CColor>
{
public:
    CColorValue() {}
    CColorValue(const CColor& rkVal) { mValue = rkVal; }

    TString ToString() const
    {
        TString out;
        out += TString::FromFloat(mValue.r) + ", ";
        out += TString::FromFloat(mValue.g) + ", ";
        out += TString::FromFloat(mValue.b) + ", ";
        out += TString::FromFloat(mValue.a);
        return out;
    }

    void FromString(const TString& rkString)
    {
        TStringList Components = rkString.Split(", ");

        if (Components.size() < 3 || Components.size() > 4)
        {
            Log::Error("CColorValue::FromString was passed a string with an invalid number of components");
            mValue = CColor::skTransparentBlack;
            return;
        }

        float *pPtr = &mValue.r;
        mValue.a = 1.0f;

        for (auto it = Components.begin(); it != Components.end(); it++)
        {
            *pPtr = it->ToFloat();
            pPtr++;
        }
    }
};

class CVector3Value : public TTypedPropertyValue<CVector3f>
{
public:
    CVector3Value() {}
    CVector3Value(const CVector3f& rkVal) { mValue = rkVal; }

    TString ToString() const
    {
        TString out;
        out += TString::FromFloat(mValue.x) + ", ";
        out += TString::FromFloat(mValue.y) + ", ";
        out += TString::FromFloat(mValue.z);
        return out;
    }

    void FromString(const TString& rkString)
    {
        TStringList Components = rkString.Split(", ");

        if (Components.size() != 3)
        {
            Log::Error("CVector3Value::FromString was passed a string with an invalid number of components");
            mValue = CVector3f::skInfinite;
            return;
        }

        float *pPtr = &mValue.x;

        for (auto it = Components.begin(); it != Components.end(); it++)
        {
            *pPtr = it->ToFloat();
            pPtr++;
        }
    }
};

class CCharacterValue : public TTypedPropertyValue<CAnimationParameters>
{
public:
    CCharacterValue() {}

    TString ToString() const { return ""; }
    void FromString(const TString&) { }
};

class CFileValue : public TTypedPropertyValue<CResourceInfo>
{
public:
    CFileValue() {}
    CFileValue(const CResourceInfo& rkInfo) { mValue = rkInfo; }

    TString ToString() const { return ""; }
    void FromString(const TString&) { }
};

class CUnknownValue : public TTypedPropertyValue<std::vector<u8>>
{
public:
    CUnknownValue();

    TString ToString() const { return ""; }
    void FromString(const TString&) {}
};

#endif // IPROPERTYVALUE_H
