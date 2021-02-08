#include "CPropertyModel.h"
#include "Editor/UICommon.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/Script/Property/IProperty.h>
#include <QFont>
#include <QSize>

#include <array>

CPropertyModel::CPropertyModel(QObject *pParent)
    : QAbstractItemModel(pParent)
{
}

int CPropertyModel::RecursiveBuildArrays(IProperty* pProperty, int ParentID)
{
    // Insert into an unused slot if one exists. Otherwise, append to the end of the array.
    int MyID = -1;

    if (mFirstUnusedID >= 0)
    {
        MyID = mFirstUnusedID;
        mFirstUnusedID = mProperties[MyID].ParentID; // on unused slots ParentID stores the ID of the next unused slot
    }
    else
    {
        MyID = mProperties.size();
        mProperties.push_back(SProperty());
    }

    mProperties[MyID].pProperty = pProperty;
    mProperties[MyID].ParentID = ParentID;

    const int RowNumber = (ParentID >= 0 ? static_cast<int>(mProperties[ParentID].ChildIDs.size()) : 0);
    mProperties[MyID].Index = createIndex(RowNumber, 0, MyID);

    if (pProperty->Type() == EPropertyType::Array)
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
        const uint32 ArrayCount = pArray->ArrayCount(mpPropertyData);
        void* pOldData = mpPropertyData;

        for (uint32 ElementIdx = 0; ElementIdx < ArrayCount; ElementIdx++)
        {
            mpPropertyData = pArray->ItemPointer(pOldData, ElementIdx);
            const int NewChildID = RecursiveBuildArrays(pArray->ItemArchetype(), MyID);
            mProperties[MyID].ChildIDs.push_back(NewChildID);
        }

        mpPropertyData = pOldData;
    }
    else
    {
        for (size_t ChildIdx = 0; ChildIdx < pProperty->NumChildren(); ChildIdx++)
        {
            const int NewChildID = RecursiveBuildArrays(pProperty->ChildByIndex(ChildIdx), MyID);
            mProperties[MyID].ChildIDs.push_back(NewChildID);
        }
    }

    if (!pProperty->IsArrayArchetype())
    {
        mPropertyToIDMap[pProperty] = MyID;
    }

    return MyID;
}

void CPropertyModel::ConfigureIntrinsic(CGameProject* pProject, IProperty* pRootProperty, void* pPropertyData)
{
    beginResetModel();

    mpProject = pProject;
    mpObject = nullptr;
    mpRootProperty = pRootProperty;
    mpPropertyData = pPropertyData;

    mProperties.clear();
    mPropertyToIDMap.clear();
    mFirstUnusedID = -1;

    if (pRootProperty != nullptr)
        RecursiveBuildArrays(pRootProperty, -1);

    endResetModel();
}

void CPropertyModel::ConfigureScript(CGameProject* pProject, IProperty* pRootProperty, CScriptObject* pObject)
{
    ConfigureIntrinsic(pProject, pRootProperty, pObject ? pObject->PropertyData() : nullptr);
    mpObject = pObject;
}

IProperty* CPropertyModel::PropertyForIndex(const QModelIndex& rkIndex, bool HandleFlaggedIndices) const
{
    if (!rkIndex.isValid())
        return mpRootProperty;

    int Index = rkIndex.internalId();

    if ((Index & 0x80000000) != 0)
    {
        if (HandleFlaggedIndices)
            Index &= ~0x80000000;
        else
            return nullptr;
    }

    return mProperties[Index].pProperty;
}

QModelIndex CPropertyModel::IndexForProperty(IProperty *pProp) const
{
    // Array archetype properties cannot be associated with a single index because the same IProperty
    // is used for every element of the array. So instead fetch the index for the array itself.
    if (pProp->IsArrayArchetype())
    {
        while (pProp != nullptr && pProp->IsArrayArchetype())
            pProp = pProp->Parent();

        ASSERT(pProp != nullptr && pProp->Type() == EPropertyType::Array);
    }

    if (pProp == mpRootProperty) return QModelIndex();

    const int ID = mPropertyToIDMap[pProp];
    ASSERT(ID >= 0);

    return mProperties[ID].Index;
}

void* CPropertyModel::DataPointerForIndex(const QModelIndex& rkIndex) const
{
    // Going to be the base pointer in 99% of cases, but we need to account for arrays in some cases
    int ID = static_cast<int>(rkIndex.internalId() & ~0x80000000);

    if (!mProperties[ID].pProperty->IsArrayArchetype())
        return mpPropertyData;

    // Head up the hierarchy until we find a non-array property, keeping track of array indices along the way
    // Static arrays to avoid memory allocations, we never have more than 2 nested arrays
    std::array<CArrayProperty*, 2> ArrayProperties{};
    std::array<int, 2> ArrayIndices{};
    int MaxIndex = -1;

    IProperty* pProperty = mProperties[ID].pProperty;

    while (pProperty->IsArrayArchetype())
    {
        if (CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty->Parent()))
        {
            MaxIndex++;
            ArrayProperties[MaxIndex] = pArray;
            ArrayIndices[MaxIndex] = mProperties[ID].Index.row();
        }

        ID = mProperties[ID].ParentID;
        pProperty = pProperty->Parent();
    }

    // Now fetch the correct pointer from the array properties
    void* pOutData = mpPropertyData;

    for (int i = MaxIndex; i >= 0; i--)
    {
        const CArrayProperty* pArray = ArrayProperties[i];
        const int ArrayIndex = ArrayIndices[i];
        pOutData = pArray->ItemPointer(pOutData, ArrayIndex);
    }

    return pOutData;
}

int CPropertyModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return 2;
}

int CPropertyModel::rowCount(const QModelIndex& rkParent) const
{
    if (mpRootProperty == nullptr)
        return 0;

    if (!rkParent.isValid())
        return static_cast<int>(mpRootProperty->NumChildren());

    if (rkParent.column() != 0)
        return 0;

    if ((rkParent.internalId() & 0x80000000) != 0)
        return 0;

    IProperty *pProp = PropertyForIndex(rkParent, false);
    const int ID = rkParent.internalId();

    switch (pProp->Type())
    {
    case EPropertyType::Flags:
        return TPropCast<CFlagsProperty>(pProp)->NumFlags();

    case EPropertyType::AnimationSet:
    {
        void* pData = DataPointerForIndex(rkParent);
        const CAnimationParameters Params = TPropCast<CAnimationSetProperty>(pProp)->Value(pData);

        if (Params.Version() <= EGame::Echoes) return 3;
        if (Params.Version() <= EGame::Corruption) return 2;
        return 4;
    }

    default:
        return static_cast<int>(mProperties[ID].ChildIDs.size());
    }
}

QVariant CPropertyModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Orientation == Qt::Horizontal && Role == Qt::DisplayRole)
    {
        if (Section == 0) return tr("Name");
        if (Section == 1) return tr("Value");
    }
    return QVariant::Invalid;
}

QVariant CPropertyModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!rkIndex.isValid())
        return QVariant::Invalid;

    if (Role == Qt::DisplayRole || (Role == Qt::ToolTipRole && rkIndex.column() == 1))
    {
        if ((rkIndex.internalId() & 0x80000000) != 0)
        {
            IProperty *pProp = PropertyForIndex(rkIndex, true);
            const EPropertyType Type = pProp->Type();

            if (Type == EPropertyType::Flags)
            {
                CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);

                if (rkIndex.column() == 0)
                    return TO_QSTRING(pFlags->FlagName(rkIndex.row()));

                if (rkIndex.column() == 1)
                {
                    if (Role == Qt::DisplayRole)
                        return QString{};

                    return TO_QSTRING(TString::HexString(pFlags->FlagMask(rkIndex.row())));
                }
            }
            else if (Type == EPropertyType::AnimationSet)
            {
                void* pData = DataPointerForIndex(rkIndex);
                const CAnimationSetProperty* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
                const CAnimationParameters Params = pAnimSet->Value(pData);

                // There are three different layouts for this property - one for MP1/2, one for MP3, and one for DKCR
                if (Params.Version() <= EGame::Echoes)
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return tr("AnimSet");
                        if (rkIndex.row() == 1) return tr("Character");
                        if (rkIndex.row() == 2) return tr("DefaultAnim");
                    }

                    // For column 1, rows 0/1 have persistent editors so we only handle 2
                    if (rkIndex.column() == 1 && rkIndex.row() == 2)
                        return QString::number(Params.Unknown(0));
                }
                else if (Params.Version() <= EGame::Corruption)
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return tr("Character");
                        if (rkIndex.row() == 1) return tr("DefaultAnim");
                    }

                    // Same deal here, only handle row 1
                    if (rkIndex.column() == 1 && rkIndex.row() == 1)
                        return QString::number(Params.Unknown(0));
                }
                else
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return tr("Character");
                        if (rkIndex.row() == 1) return tr("DefaultAnim");
                        return tr("Unknown%1").arg(rkIndex.row() - 1);
                    }

                    if (rkIndex.column() == 1 && rkIndex.row() > 0)
                        return QString::number(Params.Unknown(rkIndex.row() - 1));
                }
            }
        }
        else
        {
            IProperty *pProp = PropertyForIndex(rkIndex, false);

            if (rkIndex.column() == 0)
            {
                // Check for arrays
                const IProperty *pParent = pProp->Parent();

                if (pParent != nullptr && pParent->Type() == EPropertyType::Array)
                {
                    // For direct array sub-properties, display the element index after the name
                    const TString ElementName = pProp->Name();
                    return tr("%1 %2").arg(TO_QSTRING(ElementName)).arg(rkIndex.row() + 1);
                }

                // Display property name for everything else
                return TO_QSTRING(pProp->Name());
            }

            if (rkIndex.column() == 1)
            {
                void* pData = DataPointerForIndex(rkIndex);
                const EPropertyType Type = GetEffectiveFieldType(pProp);

                switch (Type)
                {
                // Enclose vector property text in parentheses
                case EPropertyType::Vector:
                {
                    const CVector3f Value = TPropCast<CVectorProperty>(pProp)->Value(pData);
                    return TO_QSTRING('(' + Value.ToString() + ')');
                }

                // Display the AGSC/sound name for sounds
                case EPropertyType::Sound:
                {
                    const CSoundProperty* pSound = TPropCast<CSoundProperty>(pProp);
                    const uint32 SoundID = pSound->Value(pData);
                    if (SoundID == UINT32_MAX)
                        return tr("[None]");

                    const SSoundInfo SoundInfo = mpProject->AudioManager()->GetSoundInfo(SoundID);
                    if (SoundInfo.DefineID == 0xFFFF)
                        return tr("%1 [INVALID]").arg(SoundID);

                    // Always display define ID. Display sound name if we have one, otherwise display AGSC ID.
                    QString Out = QString::number(SoundID);
                    Out += QStringLiteral(" [") + TO_QSTRING(TString::HexString(SoundInfo.DefineID, 4));
                    const QString AudioGroupName = (SoundInfo.pAudioGroup ? TO_QSTRING(SoundInfo.pAudioGroup->Entry()->Name()) : tr("NO AUDIO GROUP"));
                    const QString Name = (!SoundInfo.Name.IsEmpty() ? TO_QSTRING(SoundInfo.Name) : AudioGroupName);
                    Out += QLatin1Char{' '} + Name + QLatin1Char{']'};

                    // If we have a sound name and this is a tooltip, add a second line with the AGSC name
                    if (Role == Qt::ToolTipRole && !SoundInfo.Name.IsEmpty())
                        Out += QLatin1Char{'\n'} + AudioGroupName;

                    return Out;
                }

                // Display character name for characters
                case EPropertyType::AnimationSet:
                    return TO_QSTRING(TPropCast<CAnimationSetProperty>(pProp)->Value(pData).GetCurrentCharacterName());

                // Display enumerator name for enums (but only on ToolTipRole)
                case EPropertyType::Choice:
                case EPropertyType::Enum:
                    if (Role == Qt::ToolTipRole)
                    {
                        const CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);
                        const uint32 ValueID = pEnum->Value(pData);
                        const uint32 ValueIndex = pEnum->ValueIndex(ValueID);
                        return TO_QSTRING(pEnum->ValueName(ValueIndex));
                    }
                    return QString{};

                // Display the element count for arrays
                case EPropertyType::Array:
                {
                    const uint32 Count = TPropCast<CArrayProperty>(pProp)->Value(pData);
                    return tr("%1 element%2").arg(Count).arg(Count != 1 ? tr("s") : QString{});
                }

                // Display "[spline]" for splines (todo: proper support)
                case EPropertyType::Spline:
                    return tr("[spline]");

                // No display text on properties with persistent editors
                case EPropertyType::Bool:
                case EPropertyType::Asset:
                case EPropertyType::Color:
                    if (Role == Qt::DisplayRole)
                        return QString{};
                [[fallthrough]];

                // Display property value to string for everything else
                default:
                    return TO_QSTRING(pProp->ValueAsString(pData) + pProp->Suffix());
                }
            }
        }
    }

    if (Role == Qt::ToolTipRole && rkIndex.column() == 0)
    {
        if (!(rkIndex.internalId() & 0x80000000))
        {
            // Add name
            IProperty *pProp = PropertyForIndex(rkIndex, false);
            const QString DisplayText = data(rkIndex, Qt::DisplayRole).toString();
            const QString TypeName = pProp->HashableTypeName();
            QString Text = tr("<b>%1</b> <i>(%2)</i>").arg(DisplayText).arg(TypeName);

            // Add uncooked notification
            if (pProp->CookPreference() == ECookPreference::Never)
            {
                Text.prepend(tr("<i>[uncooked]</i>"));
            }

            // Add description
            const TString Desc = pProp->Description();
            if (!Desc.IsEmpty())
                Text += tr("<br/>%1").arg(TO_QSTRING(Desc));

            // Spline notification
            if (pProp->Type() == EPropertyType::Spline)
                Text += tr("<br/><i>(NOTE: Spline properties are currently unsupported for editing)</i>");

            return Text;
        }
    }

    if (Role == Qt::FontRole && rkIndex.column() == 0)
    {
        QFont Font = mFont;
        bool Bold = false;

        if (mBoldModifiedProperties)
        {
            const IProperty *pProp = PropertyForIndex(rkIndex, true);

            if (!pProp->IsArrayArchetype())
            {
                Bold = !pProp->MatchesDefault(mpPropertyData);
            }
        }

        Font.setBold(Bold);
        return Font;
    }

    if (Role == Qt::SizeHintRole)
    {
        return QSize(0, 23);
    }

    if (Role == Qt::ForegroundRole)
    {
        if (mShowNameValidity && mpRootProperty->ScriptTemplate()->Game() >= EGame::EchoesDemo)
        {
            if (IProperty* pProp = PropertyForIndex(rkIndex, true))
            {
                static const QColor skRightColor = QColor(128, 255, 128);
                static const QColor skWrongColor = QColor(255, 128, 128);
                return QBrush(pProp->HasAccurateName() ? skRightColor : skWrongColor);
            }
        }
    }

    return QVariant::Invalid;
}

QModelIndex CPropertyModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    // Invalid index
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    // Check property for children
    const IProperty* pParent = (rkParent.isValid() ? PropertyForIndex(rkParent, false) : mpRootProperty);
    const EPropertyType ParentType = pParent->Type();
    const int ParentID = rkParent.internalId();

    if (ParentType == EPropertyType::Flags || ParentType == EPropertyType::AnimationSet)
    {
        return createIndex(Row, Column, ParentID | 0x80000000);
    }
    else
    {
        const int ChildID = mProperties[ParentID].ChildIDs[Row];
        return createIndex(Row, Column, ChildID);
    }
}

QModelIndex CPropertyModel::parent(const QModelIndex& rkChild) const
{
    // Invalid index
    if (!rkChild.isValid())
        return QModelIndex();

    auto ID = static_cast<int>(rkChild.internalId());

    if ((ID & 0x80000000) != 0)
        ID &= ~0x80000000;
    else
        ID = mProperties[ID].ParentID;

    if (ID >= 0)
        return mProperties[ID].Index;
    else
        return QModelIndex();
}

Qt::ItemFlags CPropertyModel::flags(const QModelIndex& rkIndex) const
{
    if (rkIndex.column() == 0)
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void CPropertyModel::NotifyPropertyModified(class CScriptObject*, IProperty* pProp)
{
    NotifyPropertyModified(IndexForProperty(pProp));
}

void CPropertyModel::NotifyPropertyModified(const QModelIndex& rkIndex)
{
    if (rowCount(rkIndex) != 0)
        emit dataChanged( index(0, 0, rkIndex), index(rowCount(rkIndex) - 1, 1, rkIndex));

    if ((rkIndex.internalId() & 0x80000000) != 0)
    {
        const QModelIndex Parent = rkIndex.parent();
        const QModelIndex Col0 = Parent.sibling(Parent.row(), 0);
        const QModelIndex Col1 = Parent.sibling(Parent.row(), 1);
        emit dataChanged(Col0, Col1);
    }

    const QModelIndex IndexCol0 = rkIndex.sibling(rkIndex.row(), 0);
    const QModelIndex IndexCol1 = rkIndex.sibling(rkIndex.row(), 1);
    emit dataChanged(IndexCol0, IndexCol1);

    emit PropertyModified(rkIndex);
}

void CPropertyModel::ArrayAboutToBeResized(const QModelIndex& rkIndex, uint32 NewSize)
{
    const QModelIndex Index = rkIndex.sibling(rkIndex.row(), 0);
    IProperty* pProperty = PropertyForIndex(Index, false);
    const CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
    ASSERT(pArray);

    void* pArrayData = DataPointerForIndex(Index);
    const uint32 OldSize = pArray->ArrayCount(pArrayData);

    if (NewSize == OldSize)
        return;

    if (NewSize > OldSize)
        beginInsertRows(Index, static_cast<int>(OldSize), static_cast<int>(NewSize - 1));
    else
        beginRemoveRows(Index, static_cast<int>(NewSize), static_cast<int>(OldSize - 1));
}

void CPropertyModel::ArrayResized(const QModelIndex& rkIndex, uint32 OldSize)
{
    const QModelIndex Index = rkIndex.sibling(rkIndex.row(), 0);
    IProperty* pProperty = PropertyForIndex(Index, false);
    const CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
    ASSERT(pArray);

    void* pArrayData = DataPointerForIndex(Index);
    const uint32 NewSize = pArray->ArrayCount(pArrayData);

    if (NewSize != OldSize)
    {
        const int ID = Index.internalId();

        if (NewSize > OldSize)
        {
            // add new elements
            void* pOldData = mpPropertyData;

            for (uint32 ElementIdx = OldSize; ElementIdx < NewSize; ElementIdx++)
            {
                mpPropertyData = pArray->ItemPointer(pArrayData, ElementIdx);
                const int NewChildID = RecursiveBuildArrays( pArray->ItemArchetype(), ID );
                mProperties[ID].ChildIDs.push_back(NewChildID);
            }

            mpPropertyData = pOldData;
            endInsertRows();
        }
        else
        {
            // remove old elements
            for (uint32 ElementIdx = NewSize; ElementIdx < OldSize; ElementIdx++)
            {
                const int ChildID = mProperties[ID].ChildIDs[ElementIdx];
                ClearSlot(ChildID);
            }

            mProperties[ID].ChildIDs.resize(NewSize);
            endRemoveRows();
        }
    }
}


void CPropertyModel::ClearSlot(int ID)
{
    for (const int ChildID : mProperties[ID].ChildIDs)
    {
        ClearSlot(ChildID);
    }

    mProperties[ID].ChildIDs.clear();
    mProperties[ID].Index = QModelIndex();
    mProperties[ID].ParentID = mFirstUnusedID;
    mProperties[ID].pProperty = nullptr;
    mFirstUnusedID = ID;
}

/** Determine the effective property type to use. Allows some types to be treated as other types. */
EPropertyType CPropertyModel::GetEffectiveFieldType(IProperty* pProperty) const
{
    EPropertyType Out = pProperty->Type();

    switch (Out)
    {
    // Allow Choice/Enum properties to be edited as Int properties if they don't have any values set.
    case EPropertyType::Choice:
    case EPropertyType::Enum:
    {
        CChoiceProperty* pChoice = TPropCast<CChoiceProperty>(pProperty);

        if (pChoice->NumPossibleValues() == 0)
        {
            Out = EPropertyType::Int;
        }

        break;
    }

    // Same deal with Flag properties
    case EPropertyType::Flags:
    {
        CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProperty);

        if (pFlags->NumFlags() == 0)
        {
            Out = EPropertyType::Int;
        }

        break;
    }

    default:
        break;
    }

    return Out;
}

void CPropertyModel::SetShowPropertyNameValidity(bool Enable)
{
    mShowNameValidity = Enable;

    // Emit data changed so that name colors are updated;
    const QVector<int> Roles{static_cast<int>(Qt::ForegroundRole)};
    const QModelIndex TopLeft = index(0, 0, QModelIndex());
    const QModelIndex BottomRight = index(rowCount(QModelIndex()) - 1, 0, QModelIndex());
    emit dataChanged(TopLeft, BottomRight, Roles);
}
