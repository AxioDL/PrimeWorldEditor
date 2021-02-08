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
    bool mCanBeSerialized = false;
    bool mCanHaveDependencies = true;
    bool mCanBeCreated = false;

    static std::unordered_map<EResourceType, std::unique_ptr<CResTypeInfo>> smTypeMap;

    // Private Methods
    CResTypeInfo(EResourceType type, TString typeName, TString retroExtension);
    ~CResTypeInfo() = default;
    friend struct std::default_delete<CResTypeInfo>;

    // Public Methods
public:
    bool IsInGame(EGame Game) const;
    CFourCC CookedExtension(EGame Game) const;

    // Accessors
    EResourceType Type() const       { return mType; }
    TString TypeName() const         { return mTypeName; }
    bool CanBeSerialized() const     { return mCanBeSerialized; }
    bool CanHaveDependencies() const { return mCanHaveDependencies; }
    bool CanBeCreated() const        { return mCanBeCreated; }

    // Static
    static void GetAllTypesInGame(EGame Game, std::list<CResTypeInfo*>& rOut);
    static CResTypeInfo* TypeForCookedExtension(EGame, CFourCC Ext);

    static CResTypeInfo* FindTypeInfo(EResourceType Type);

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

