#ifndef CTEMPLATECATEGORY_H
#define CTEMPLATECATEGORY_H

#include "CScriptTemplate.h"
#include <algorithm>

class CTemplateCategory
{
    std::string mCategoryName;
    std::vector<CScriptTemplate*> mTemplates;

public:
    CTemplateCategory() {}

    inline CTemplateCategory(const std::string& Name) {
        SetName(Name);
    }

    inline void SetName(const std::string& Name) {
        mCategoryName = Name;
    }

    inline void AddTemplate(CScriptTemplate *pTmp) {
        mTemplates.push_back(pTmp);
    }

    inline void Sort() {
        std::sort(mTemplates.begin(), mTemplates.end(), [](CScriptTemplate* pA, CScriptTemplate* pB) -> bool {
            return (pA->TemplateName() < pB->TemplateName());
        });
    }

    inline u32 NumTemplates() {
        return mTemplates.size();
    }

    inline CScriptTemplate* GetTemplate(u32 index) {
        return mTemplates[index];
    }

    inline CScriptTemplate* operator[](u32 index) {
        return mTemplates[index];
    }
};

#endif // CTEMPLATECATEGORY_H
