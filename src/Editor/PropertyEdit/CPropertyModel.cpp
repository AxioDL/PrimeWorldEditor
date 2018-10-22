#include "CPropertyModel.h"
#include "Editor/UICommon.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/Script/Property/IProperty.h>
#include <QFont>
#include <QSize>

CPropertyModel::CPropertyModel(QObject *pParent /*= 0*/)
    : QAbstractItemModel(pParent)
    , mpProject(nullptr)
    , mpRootProperty(nullptr)
    , mpPropertyData(nullptr)
    , mBoldModifiedProperties(true)
    , mShowNameValidity(false)
    , mFirstUnusedID(-1)
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
        mProperties << SProperty();
    }

    mProperties[MyID].pProperty = pProperty;
    mProperties[MyID].ParentID = ParentID;

    int RowNumber = (ParentID >= 0 ? mProperties[ParentID].ChildIDs.size() : 0);
    mProperties[MyID].Index = createIndex(RowNumber, 0, MyID);

    if (pProperty->Type() == EPropertyType::Array)
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
        u32 ArrayCount = pArray->ArrayCount(mpPropertyData);
        void* pOldData = mpPropertyData;

        for (u32 ElementIdx = 0; ElementIdx < ArrayCount; ElementIdx++)
        {
            mpPropertyData = pArray->ItemPointer(pOldData, ElementIdx);
            int NewChildID = RecursiveBuildArrays( pArray->ItemArchetype(), MyID );
            mProperties[MyID].ChildIDs.push_back(NewChildID);
        }

        mpPropertyData = pOldData;
    }
    else
    {
        for (u32 ChildIdx = 0; ChildIdx < pProperty->NumChildren(); ChildIdx++)
        {
            int NewChildID = RecursiveBuildArrays( pProperty->ChildByIndex(ChildIdx), MyID );
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

    if (pRootProperty)
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
    if (!rkIndex.isValid()) return mpRootProperty;

    int Index = rkIndex.internalId();

    if (Index & 0x80000000)
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
        while (pProp && pProp->IsArrayArchetype())
            pProp = pProp->Parent();

        ASSERT(pProp != nullptr && pProp->Type() == EPropertyType::Array);
    }

    if (pProp == mpRootProperty) return QModelIndex();

    int ID = mPropertyToIDMap[pProp];
    ASSERT(ID >= 0);

    return mProperties[ID].Index;
}

void* CPropertyModel::DataPointerForIndex(const QModelIndex& rkIndex) const
{
    // Going to be the base pointer in 99% of cases, but we need to account for arrays in some cases
    int ID = rkIndex.internalId() & ~0x80000000;

    if (!mProperties[ID].pProperty->IsArrayArchetype())
        return mpPropertyData;

    // Head up the hierarchy until we find a non-array property, keeping track of array indices along the way
    // Static arrays to avoid memory allocations, we never have more than 2 nested arrays
    CArrayProperty* ArrayProperties[2];
    int ArrayIndices[2];
    int MaxIndex = -1;

    IProperty* pProperty = mProperties[ID].pProperty;

    while (pProperty->IsArrayArchetype())
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty->Parent());

        if (pArray)
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
        CArrayProperty* pArray = ArrayProperties[i];
        int ArrayIndex = ArrayIndices[i];
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
    if (!mpRootProperty) return 0;
    if (!rkParent.isValid()) return mpRootProperty->NumChildren();
    if (rkParent.column() != 0) return 0;
    if (rkParent.internalId() & 0x80000000) return 0;

    IProperty *pProp = PropertyForIndex(rkParent, false);
    int ID = rkParent.internalId();

    switch (pProp->Type())
    {
    case EPropertyType::Flags:
        return TPropCast<CFlagsProperty>(pProp)->NumFlags();

    case EPropertyType::AnimationSet:
    {
        void* pData = DataPointerForIndex(rkParent);
        CAnimationParameters Params = TPropCast<CAnimationSetProperty>(pProp)->Value(pData);

        if (Params.Version() <= EGame::Echoes) return 3;
        if (Params.Version() <= EGame::Corruption) return 2;
        return 4;
    }

    default:
        return mProperties[ID].ChildIDs.size();
    }
}

QVariant CPropertyModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Orientation == Qt::Horizontal && Role == Qt::DisplayRole)
    {
        if (Section == 0) return "Name";
        if (Section == 1) return "Value";
    }
    return QVariant::Invalid;
}

QVariant CPropertyModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!rkIndex.isValid())
        return QVariant::Invalid;

    if (Role == Qt::DisplayRole || (Role == Qt::ToolTipRole && rkIndex.column() == 1) )
    {
        if (rkIndex.internalId() & 0x80000000)
        {
            IProperty *pProp = PropertyForIndex(rkIndex, true);
            EPropertyType Type = pProp->Type();

            if (Type == EPropertyType::Flags)
            {
                CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);

                if (rkIndex.column() == 0)
                    return TO_QSTRING( pFlags->FlagName(rkIndex.row()) );

                if (rkIndex.column() == 1)
                {
                    if (Role == Qt::DisplayRole)
                        return "";
                    else
                        return TO_QSTRING(TString::HexString( pFlags->FlagMask(rkIndex.row()) ));
                }
            }

            else if (Type == EPropertyType::AnimationSet)
            {
                void* pData = DataPointerForIndex(rkIndex);
                CAnimationSetProperty* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
                CAnimationParameters Params = pAnimSet->Value(pData);

                // There are three different layouts for this property - one for MP1/2, one for MP3, and one for DKCR
                if (Params.Version() <= EGame::Echoes)
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return "AnimSet";
                        if (rkIndex.row() == 1) return "Character";
                        if (rkIndex.row() == 2) return "DefaultAnim";
                    }

                    // For column 1, rows 0/1 have persistent editors so we only handle 2
                    if (rkIndex.column() == 1 && rkIndex.row() == 2)
                        return QString::number(Params.Unknown(0));
                }

                else if (Params.Version() <= EGame::Corruption)
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return "Character";
                        if (rkIndex.row() == 1) return "DefaultAnim";
                    }

                    // Same deal here, only handle row 1
                    if (rkIndex.column() == 1 && rkIndex.row() == 1)
                        return QString::number(Params.Unknown(0));
                }

                else
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return "Character";
                        else if (rkIndex.row() == 1) return "DefaultAnim";
                        else return "Unknown" + QString::number(rkIndex.row() - 1);
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
                IProperty *pParent = pProp->Parent();

                if (pParent && pParent->Type() == EPropertyType::Array)
                {
                    // For direct array sub-properties, display the element index after the name
                    TString ElementName = pProp->Name();
                    return QString("%1 %2").arg( TO_QSTRING(ElementName) ).arg(rkIndex.row() + 1);
                }

                // Display property name for everything else
                return TO_QSTRING(pProp->Name());
            }

            if (rkIndex.column() == 1)
            {
                void* pData = DataPointerForIndex(rkIndex);
                EPropertyType Type = GetEffectiveFieldType(pProp);

                switch (Type)
                {
                // Enclose vector property text in parentheses
                case EPropertyType::Vector:
                {
                    CVector3f Value = TPropCast<CVectorProperty>(pProp)->Value(pData);
                    return TO_QSTRING("(" + Value.ToString() + ")");
                }

                // Display the AGSC/sound name for sounds
                case EPropertyType::Sound:
                {
                    CSoundProperty* pSound = TPropCast<CSoundProperty>(pProp);
                    u32 SoundID = pSound->Value(pData);
                    if (SoundID == -1) return "[None]";

                    SSoundInfo SoundInfo = mpProject->AudioManager()->GetSoundInfo(SoundID);
                    QString Out = QString::number(SoundID);

                    if (SoundInfo.DefineID == -1)
                        return Out + " [INVALID]";

                    // Always display define ID. Display sound name if we have one, otherwise display AGSC ID.
                    Out += " [" + TO_QSTRING( TString::HexString(SoundInfo.DefineID, 4) );
                    QString AudioGroupName = (SoundInfo.pAudioGroup ? TO_QSTRING(SoundInfo.pAudioGroup->Entry()->Name()) : "NO AUDIO GROUP");
                    QString Name = (!SoundInfo.Name.IsEmpty() ? TO_QSTRING(SoundInfo.Name) : AudioGroupName);
                    Out += " " + Name + "]";

                    // If we have a sound name and this is a tooltip, add a second line with the AGSC name
                    if (Role == Qt::ToolTipRole && !SoundInfo.Name.IsEmpty())
                        Out += "\n" + AudioGroupName;

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
                        CEnumProperty *pEnum = TPropCast<CEnumProperty>(pProp);
                        u32 ValueID = pEnum->Value(pData);
                        u32 ValueIndex = pEnum->ValueIndex(ValueID);
                        return TO_QSTRING( pEnum->ValueName(ValueIndex) );
                    }
                    else return "";

                // Display the element count for arrays
                case EPropertyType::Array:
                {
                    u32 Count = TPropCast<CArrayProperty>(pProp)->Value(pData);
                    return QString("%1 element%2").arg(Count).arg(Count != 1 ? "s" : "");
                }

                // Display "[spline]" for splines (todo: proper support)
                case EPropertyType::Spline:
                    return "[spline]";

                // No display text on properties with persistent editors
                case EPropertyType::Bool:
                case EPropertyType::Asset:
                case EPropertyType::Color:
                    if (Role == Qt::DisplayRole)
                        return "";
                // fall through
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
            QString DisplayText = data(rkIndex, Qt::DisplayRole).toString();
            QString TypeName = pProp->HashableTypeName();
            QString Text = QString("<b>%1</b> <i>(%2)</i>").arg(DisplayText).arg(TypeName);

            // Add uncooked notification
            if (pProp->CookPreference() == ECookPreference::Never)
            {
                Text.prepend("<i>[uncooked]</i>");
            }

            // Add description
            TString Desc = pProp->Description();
            if (!Desc.IsEmpty()) Text += "<br/>" + TO_QSTRING(Desc);

            // Spline notification
            if (pProp->Type() == EPropertyType::Spline)
                Text += "<br/><i>(NOTE: Spline properties are currently unsupported for editing)</i>";

            return Text;
        }
    }

    if (Role == Qt::FontRole && rkIndex.column() == 0)
    {
        QFont Font = mFont;
        bool Bold = false;

        if (mBoldModifiedProperties)
        {
            IProperty *pProp = PropertyForIndex(rkIndex, true);

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
            IProperty *pProp = PropertyForIndex(rkIndex, true);

            if (pProp)
            {
                static const QColor skRightColor = QColor(128, 255, 128);
                static const QColor skWrongColor = QColor(255, 128, 128);
                return QBrush( pProp->HasAccurateName() ? skRightColor : skWrongColor );
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
    IProperty* pParent = (rkParent.isValid() ? PropertyForIndex(rkParent, false) : mpRootProperty);
    EPropertyType ParentType = pParent->Type();
    int ParentID = rkParent.internalId();

    if (ParentType == EPropertyType::Flags || ParentType == EPropertyType::AnimationSet)
    {
        return createIndex(Row, Column, ParentID | 0x80000000);
    }
    else
    {
        int ChildID = mProperties[ParentID].ChildIDs[Row];
        return createIndex(Row, Column, ChildID);
    }
}

QModelIndex CPropertyModel::parent(const QModelIndex& rkChild) const
{
    // Invalid index
    if (!rkChild.isValid())
        return QModelIndex();

    int ID = int(rkChild.internalId());

    if (ID & 0x80000000)
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
    if (rkIndex.column() == 0) return Qt::ItemIsEnabled;
    else return (Qt::ItemIsEnabled | Qt::ItemIsEditable);
}

void CPropertyModel::NotifyPropertyModified(class CScriptObject*, IProperty* pProp)
{
    NotifyPropertyModified(IndexForProperty(pProp));
}

void CPropertyModel::NotifyPropertyModified(const QModelIndex& rkIndex)
{
    if (rowCount(rkIndex) != 0)
        emit dataChanged( index(0, 0, rkIndex), index(rowCount(rkIndex) - 1, 1, rkIndex));

    if (rkIndex.internalId() & 0x80000000)
    {
        QModelIndex Parent = rkIndex.parent();
        QModelIndex Col0 = Parent.sibling(Parent.row(), 0);
        QModelIndex Col1 = Parent.sibling(Parent.row(), 1);
        emit dataChanged(Col0, Col1);
    }

    QModelIndex IndexCol0 = rkIndex.sibling(rkIndex.row(), 0);
    QModelIndex IndexCol1 = rkIndex.sibling(rkIndex.row(), 1);
    emit dataChanged(IndexCol0, IndexCol1);

    emit PropertyModified(rkIndex);
}

void CPropertyModel::ArrayAboutToBeResized(const QModelIndex& rkIndex, u32 NewSize)
{
    QModelIndex Index = rkIndex.sibling(rkIndex.row(), 0);
    IProperty* pProperty = PropertyForIndex(Index, false);
    CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
    ASSERT(pArray);

    void* pArrayData = DataPointerForIndex(Index);
    u32 OldSize = pArray->ArrayCount(pArrayData);

    if (NewSize != OldSize)
    {
        if (NewSize > OldSize)
            beginInsertRows(Index, OldSize, NewSize - 1);
        else
            beginRemoveRows(Index, NewSize, OldSize - 1);
    }
}

void CPropertyModel::ArrayResized(const QModelIndex& rkIndex, u32 OldSize)
{
    QModelIndex Index = rkIndex.sibling(rkIndex.row(), 0);
    IProperty* pProperty = PropertyForIndex(Index, false);
    CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
    ASSERT(pArray);

    void* pArrayData = DataPointerForIndex(Index);
    u32 NewSize = pArray->ArrayCount(pArrayData);

    if (NewSize != OldSize)
    {
        int ID = Index.internalId();

        if (NewSize > OldSize)
        {
            // add new elements
            void* pOldData = mpPropertyData;

            for (u32 ElementIdx = OldSize; ElementIdx < NewSize; ElementIdx++)
            {
                mpPropertyData = pArray->ItemPointer(pArrayData, ElementIdx);
                int NewChildID = RecursiveBuildArrays( pArray->ItemArchetype(), ID );
                mProperties[ID].ChildIDs.push_back(NewChildID);
            }

            mpPropertyData = pOldData;
            endInsertRows();
        }
        else
        {
            // remove old elements
            for (u32 ElementIdx = NewSize; ElementIdx < OldSize; ElementIdx++)
            {
                int ChildID = mProperties[ID].ChildIDs[ElementIdx];
                ClearSlot(ChildID);
            }

            mProperties[ID].ChildIDs.resize(NewSize);
            endRemoveRows();
        }
    }
}


void CPropertyModel::ClearSlot(int ID)
{
    for (int ChildIdx = 0; ChildIdx < mProperties[ID].ChildIDs.size(); ChildIdx++)
    {
        ClearSlot(mProperties[ID].ChildIDs[ChildIdx]);
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

    }

    return Out;
}

void CPropertyModel::SetShowPropertyNameValidity(bool Enable)
{
    mShowNameValidity = Enable;

    // Emit data changed so that name colors are updated;
    QVector<int> Roles;
    Roles << Qt::ForegroundRole;

    QModelIndex TopLeft = index(0, 0, QModelIndex());
    QModelIndex BottomRight = index( rowCount(QModelIndex()) - 1, 0, QModelIndex());
    emit dataChanged(TopLeft, BottomRight, Roles);
}
