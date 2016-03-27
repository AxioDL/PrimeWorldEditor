#ifndef IPROPERTYVALUE_H
#define IPROPERTYVALUE_H

#include "EPropertyType.h"
#include <Common/Log.h>
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
    virtual IPropertyValue* Clone() const = 0;
    virtual void Copy(const IPropertyValue *pkValue) = 0;
    virtual bool Matches(const IPropertyValue *pkValue) const = 0;
};

template<typename PropType>
class TTypedPropertyValue : public IPropertyValue
{
protected:
    PropType mValue;

public:
    TTypedPropertyValue() {}

    TTypedPropertyValue(PropType rkVal)
        : mValue(rkVal) {}

    virtual void Copy(const IPropertyValue *pkValue)
    {
        const TTypedPropertyValue *pkOther = static_cast<const TTypedPropertyValue*>(pkValue);
        mValue = pkOther->mValue;
    }

    virtual bool Matches(const IPropertyValue *pkValue) const
    {
        const TTypedPropertyValue *pkOther = static_cast<const TTypedPropertyValue*>(pkValue);
        return ((pkValue != nullptr) && (mValue == pkOther->mValue));
    }

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

    IPropertyValue* Clone() const
    {
        return new CBoolValue(mValue);
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

    IPropertyValue* Clone() const
    {
        return new CByteValue(mValue);
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

    IPropertyValue* Clone() const
    {
        return new CShortValue(mValue);
    }
};

class CLongValue : public TTypedPropertyValue<s32>
{
public:
    CLongValue()        { mValue = 0; }
    CLongValue(s32 Val) { mValue = Val; }

    TString ToString() const
    {
        return TString::FromInt32(mValue, 0, 10);
    }

    void FromString(const TString& rkString)
    {
        u32 base = (rkString.StartsWith("0x") ? 16 : 10);
        mValue = (s32) rkString.ToInt32(base);
    }

    IPropertyValue* Clone() const
    {
        return new CLongValue(mValue);
    }
};

class CHexLongValue : public TTypedPropertyValue<u32>
{
public:
    CHexLongValue()         { mValue = 0; }
    CHexLongValue(u32 Val)  { mValue = Val; }

    TString ToString() const
    {
        return TString::HexString(mValue, 8);
    }

    void FromString(const TString& rkString)
    {
        u32 Base = (rkString.StartsWith("0x") ? 16 : 10);
        mValue = (s32) rkString.ToInt32(Base);
    }

    IPropertyValue* Clone() const
    {
        return new CHexLongValue(mValue);
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

    IPropertyValue* Clone() const
    {
        return new CFloatValue(mValue);
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

    IPropertyValue* Clone() const
    {
        return new CStringValue(mValue);
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
        out += TString::FromFloat(mValue.R) + ", ";
        out += TString::FromFloat(mValue.G) + ", ";
        out += TString::FromFloat(mValue.B) + ", ";
        out += TString::FromFloat(mValue.A);
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

        float *pPtr = &mValue.R;
        mValue.A = 1.0f;

        for (auto it = Components.begin(); it != Components.end(); it++)
        {
            *pPtr = it->ToFloat();
            pPtr++;
        }
    }

    IPropertyValue* Clone() const
    {
        return new CColorValue(mValue);
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
        out += TString::FromFloat(mValue.X) + ", ";
        out += TString::FromFloat(mValue.Y) + ", ";
        out += TString::FromFloat(mValue.Z);
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

        float *pPtr = &mValue.X;

        for (auto it = Components.begin(); it != Components.end(); it++)
        {
            *pPtr = it->ToFloat();
            pPtr++;
        }
    }

    IPropertyValue* Clone() const
    {
        return new CVector3Value(mValue);
    }
};

class CCharacterValue : public TTypedPropertyValue<CAnimationParameters>
{
public:
    CCharacterValue() {}
    CCharacterValue(const CAnimationParameters& rkParams) { mValue = rkParams; }

    TString ToString() const { return ""; }
    void FromString(const TString&) { }

    IPropertyValue* Clone() const
    {
        return new CCharacterValue(mValue);
    }
};

class CMayaSplineValue : public TTypedPropertyValue<std::vector<u8>>
{
public:
    CMayaSplineValue() {}
    CMayaSplineValue(const std::vector<u8>& rkData) { mValue = rkData; }

    TString ToString() const { return "[MayaSpline]"; }
    void FromString(const TString&) {}

    IPropertyValue* Clone() const
    {
        return new CMayaSplineValue(mValue);
    }
};

class CFileValue : public TTypedPropertyValue<CResourceInfo>
{
public:
    CFileValue() {}
    CFileValue(const CResourceInfo& rkInfo) { mValue = rkInfo; }

    TString ToString() const { return ""; }
    void FromString(const TString&) {}

    IPropertyValue* Clone() const
    {
        return new CFileValue(mValue);
    }
};

class CUnknownValue : public TTypedPropertyValue<std::vector<u8>>
{
public:
    CUnknownValue();
    CUnknownValue(const std::vector<u8>& rkVec) { mValue = rkVec; }

    TString ToString() const { return ""; }
    void FromString(const TString&) {}

    IPropertyValue* Clone() const
    {
        return new CUnknownValue(mValue);
    }
};

#endif // IPROPERTYVALUE_H
