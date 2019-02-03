#ifndef CRESTYPEINFO
#define CRESTYPEINFO

#include "EResType.h"
#include <Common/CFourCC.h>
#include <Common/EGame.h>
#include <Common/Flags.h>
#include <Common/TString.h>
#include <unordered_map>
#include <vector>

class CResTypeInfo
{
    struct SGameExtension
    {
        EGame Game;
        CFourCC CookedExt;
    };

    EResourceType mType;
    TString mTypeName;
    std::vector<SGameExtension> mCookedExtensions;
    TString mRetroExtension; // File extension in Retro's directory tree. We don't use it directly but it is needed for generating asset ID hashes
    bool mCanBeSerialized;
    bool mCanHaveDependencies;
    bool mCanBeCreated;

    static std::unordered_map<EResourceType, CResTypeInfo*> smTypeMap;

    // Private Methods
    CResTypeInfo(EResourceType Type, const TString& rkTypeName, const TString& rkRetroExtension);
    ~CResTypeInfo() {}

    // Public Methods
public:
    bool IsInGame(EGame Game) const;
    CFourCC CookedExtension(EGame Game) const;

    // Accessors
    inline EResourceType Type() const       { return mType; }
    inline TString TypeName() const         { return mTypeName; }
    inline bool CanBeSerialized() const     { return mCanBeSerialized; }
    inline bool CanHaveDependencies() const { return mCanHaveDependencies; }
    inline bool CanBeCreated() const        { return mCanBeCreated; }

    // Static
    static void GetAllTypesInGame(EGame Game, std::list<CResTypeInfo*>& rOut);
    static CResTypeInfo* TypeForCookedExtension(EGame, CFourCC Ext);

    inline static CResTypeInfo* FindTypeInfo(EResourceType Type)
    {
        auto Iter = smTypeMap.find(Type);
        return (Iter == smTypeMap.end() ? nullptr : Iter->second);
    }

private:
    // Creation
    friend class CResTypeInfoFactory;

    class CResTypeInfoFactory
    {
    public:
        CResTypeInfoFactory();
        void AddExtension(CResTypeInfo *pType, CFourCC Ext, EGame FirstGame, EGame LastGame);
        void InitTypes();
    };
    static CResTypeInfoFactory smTypeInfoFactory;
};

// Serialization
void Serialize(IArchive& rArc, CResTypeInfo*& rpType);

#endif // CRESTYPEINFO

