#include "CPropertyModel.h"
#include "Editor/UICommon.h"
#include <Core/GameProject/CGameProject.h>
#include <Core/Resource/Script/IProperty.h>
#include <Core/Resource/Script/IPropertyTemplate.h>
#include <QFont>
#include <QSize>

CPropertyModel::CPropertyModel(QObject *pParent /*= 0*/)
    : QAbstractItemModel(pParent)
    , mpProject(nullptr)
    , mpRootProperty(nullptr)
    , mpPropertyData(nullptr)
    , mBoldModifiedProperties(true)
    , mShowNameValidity(false)
{
}

int CPropertyModel::RecursiveBuildArrays(IPropertyNew* pProperty, int ParentID)
{
    int MyID = mProperties.size();
    mProperties << SProperty();

    mProperties[MyID].pProperty = pProperty;
    mProperties[MyID].ParentID = ParentID;

    int RowNumber = (ParentID >= 0 ? mProperties[ParentID].ChildIDs.size() : 0);
    mProperties[MyID].Index = createIndex(RowNumber, 0, MyID);

    if (pProperty->Type() == EPropertyTypeNew::Array)
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);

        for (u32 ElementIdx = 0; ElementIdx < pArray->ArrayCount(mpPropertyData); ElementIdx++)
        {
            int NewChildID = RecursiveBuildArrays( pArray->Archetype(), MyID );
            mProperties[MyID].ChildIDs.push_back(NewChildID);
        }
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

void CPropertyModel::ConfigureIntrinsic(CGameProject* pProject, IPropertyNew* pRootProperty, void* pPropertyData)
{
    beginResetModel();

    mpProject = pProject;
    mpObject = nullptr;
    mpRootProperty = pRootProperty;
    mpPropertyData = pPropertyData;

    mProperties.clear();
    mPropertyToIDMap.clear();

    if (pRootProperty)
        RecursiveBuildArrays(pRootProperty, -1);

    endResetModel();
}

void CPropertyModel::ConfigureScript(CGameProject* pProject, IPropertyNew* pRootProperty, CScriptObject* pObject)
{
    ConfigureIntrinsic(pProject, pRootProperty, pObject ? pObject->PropertyData() : nullptr);
    mpObject = pObject;
}

IPropertyNew* CPropertyModel::PropertyForIndex(const QModelIndex& rkIndex, bool HandleFlaggedIndices) const
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

QModelIndex CPropertyModel::IndexForProperty(IPropertyNew *pProp) const
{
    // Array archetype properties cannot be associated with a single index because the same IProperty
    // is used for every element of the array. So instead fetch the index for the array itself.
    if (pProp->IsArrayArchetype())
    {
        while (pProp && pProp->IsArrayArchetype())
            pProp = pProp->Parent();

        ASSERT(pProp != nullptr && pProp->Type() == EPropertyTypeNew::Array);
    }

    if (pProp == mpRootProperty) return QModelIndex();

    int ID = mPropertyToIDMap[pProp];
    ASSERT(ID >= 0);

    return mProperties[ID].Index;
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

    IPropertyNew *pProp = PropertyForIndex(rkParent, false);
    int ID = mPropertyToIDMap[pProp];

    switch (pProp->Type())
    {
    case EPropertyTypeNew::Flags:
        return TPropCast<CFlagsProperty>(pProp)->NumFlags();

    case EPropertyTypeNew::AnimationSet:
    {
        CAnimationParameters Params = TPropCast<CAnimationSetProperty>(pProp)->Value(mpPropertyData);

        if (Params.Version() <= eEchoes) return 3;
        if (Params.Version() <= eCorruption) return 2;
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
            IPropertyNew *pProp = PropertyForIndex(rkIndex, true);
            EPropertyTypeNew Type = pProp->Type();

            if (Type == EPropertyTypeNew::Flags)
            {
                CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);

                if (rkIndex.column() == 0)
                    return TO_QSTRING( pFlags->FlagName(rkIndex.row()) );

                if (rkIndex.column() == 1)
                {
                    if (Role == Qt::DisplayRole)
                        return "";
                    else
                        return TO_QSTRING(TString::HexString( pFlags->FlagMask(rkIndex.row())));
                }
            }

            else if (Type == EPropertyTypeNew::AnimationSet)
            {
                CAnimationSetProperty* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
                CAnimationParameters Params = pAnimSet->Value(mpPropertyData);

                // There are three different layouts for this property - one for MP1/2, one for MP3, and one for DKCR
                if (Params.Version() <= eEchoes)
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return "AnimSet";
                        if (rkIndex.row() == 1) return "Character";
                        if (rkIndex.row() == 2) return "Default Anim";
                    }

                    // For column 1, rows 0/1 have persistent editors so we only handle 2
                    if (rkIndex.column() == 1 && rkIndex.row() == 2)
                        return QString::number(Params.Unknown(0));
                }

                else if (Params.Version() <= eCorruption)
                {
                    if (rkIndex.column() == 0)
                    {
                        if (rkIndex.row() == 0) return "Character";
                        if (rkIndex.row() == 1) return "Default Anim";
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
                        else if (rkIndex.row() == 1) return "Default Anim";
                        else return "Unknown " + QString::number(rkIndex.row() - 1);
                    }

                    if (rkIndex.column() == 1 && rkIndex.row() > 0)
                        return QString::number(Params.Unknown(rkIndex.row() - 1));
                }
            }
        }

        else
        {
            IPropertyNew *pProp = PropertyForIndex(rkIndex, false);

            if (rkIndex.column() == 0)
            {
                // Check for arrays
                IPropertyNew *pParent = pProp->Parent();

                if (pParent && pParent->Type() == EPropertyTypeNew::Array)
                {
                    // For direct array sub-properties, display the element index after the name
                    TString ElementName = pParent->Name();
                    return QString("%1 %2").arg( TO_QSTRING(ElementName) ).arg(rkIndex.row() + 1);
                }

                // Display property name for everything else
                return TO_QSTRING(pProp->Name());
            }

            if (rkIndex.column() == 1)
            {
                switch (pProp->Type())
                {
                // Enclose vector property text in parentheses
                case EPropertyTypeNew::Vector:
                {
                    CVector3f Value = TPropCast<CVectorProperty>(pProp)->Value(mpPropertyData);
                    return TO_QSTRING("(" + Value.ToString() + ")");
                }

                // Display the AGSC/sound name for sounds
                case EPropertyTypeNew::Sound:
                {
                    CSoundProperty* pSound = TPropCast<CSoundProperty>(pProp);
                    u32 SoundID = pSound->Value(mpPropertyData);
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
                case EPropertyTypeNew::AnimationSet:
                    return TO_QSTRING(TPropCast<CAnimationSetProperty>(pProp)->Value(mpPropertyData).GetCurrentCharacterName());

                // Display enumerator name for enums (but only on ToolTipRole)
                case EPropertyTypeNew::Choice:
                case EPropertyTypeNew::Enum:
                    if (Role == Qt::ToolTipRole)
                    {
                        CEnumProperty *pEnum = TPropCast<CEnumProperty>(pProp);
                        u32 ValueID = pEnum->Value(mpPropertyData);
                        u32 ValueIndex = pEnum->ValueIndex(ValueID);
                        return TO_QSTRING( pEnum->ValueName(ValueIndex) );
                    }
                    else return "";

                // Display the element count for arrays
                case EPropertyTypeNew::Array:
                {
                    u32 Count = TPropCast<CArrayProperty>(pProp)->Value(mpPropertyData);
                    return QString("%1 element%2").arg(Count).arg(Count != 1 ? "s" : "");
                }

                // Display "[spline]" for splines (todo: proper support)
                case EPropertyTypeNew::Spline:
                    return "[spline]";

                // No display text on properties with persistent editors
                case EPropertyTypeNew::Bool:
                case EPropertyTypeNew::Asset:
                case EPropertyTypeNew::Color:
                    if (Role == Qt::DisplayRole)
                        return "";
                // fall through
                // Display property value to string for everything else
                default:
                    return TO_QSTRING(pProp->ValueAsString(mpPropertyData) + pProp->Suffix());
                }
            }
        }
    }

    if (Role == Qt::ToolTipRole && rkIndex.column() == 0)
    {
        if (!(rkIndex.internalId() & 0x80000000))
        {
            // Add name
            IPropertyNew *pProp = PropertyForIndex(rkIndex, false);
            QString DisplayText = data(rkIndex, Qt::DisplayRole).toString();
            QString Text = QString("<b>%1</b> <i>(%2)</i>").arg(DisplayText).arg(pProp->HashableTypeName());

            // Add uncooked notification
            if (pProp->CookPreference() == ECookPreferenceNew::Never)
            {
                Text.prepend("<i>[uncooked]</i>");
            }

            // Add description
            TString Desc = pProp->Description();
            if (!Desc.IsEmpty()) Text += "<br/>" + TO_QSTRING(Desc);

            // Spline notification
            if (pProp->Type() == EPropertyTypeNew::Spline)
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
            IPropertyNew *pProp = PropertyForIndex(rkIndex, true);

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
        if (mShowNameValidity && mpRootProperty->ScriptTemplate()->Game() >= eEchoesDemo)
        {
            IPropertyNew *pProp = PropertyForIndex(rkIndex, true);

            // Don't highlight the name of the root property
            if (pProp && pProp->Parent() != nullptr && !pProp->IsArrayArchetype())
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
    IPropertyNew* pParent = (rkParent.isValid() ? PropertyForIndex(rkParent, false) : mpRootProperty);
    EPropertyTypeNew ParentType = pParent->Type();
    int ParentID = mPropertyToIDMap[pParent];

    if (ParentType == EPropertyTypeNew::Flags || ParentType == EPropertyTypeNew::AnimationSet)
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

void CPropertyModel::NotifyPropertyModified(class CScriptObject*, IPropertyNew* pProp)
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
    //FIXME
    /*QModelIndex Index = rkIndex.sibling(rkIndex.row(), 0);
    CArrayProperty *pArray = static_cast<CArrayProperty*>(PropertyForIndex(Index, false));

    if (pArray && pArray->Type() == eArrayProperty)
    {
        u32 OldSize = pArray->Count();

        if (NewSize != OldSize)
        {
            if (NewSize > OldSize)
                beginInsertRows(Index, OldSize, NewSize - 1);
            else
                beginRemoveRows(Index, NewSize, OldSize - 1);
        }
    }*/
}

void CPropertyModel::ArrayResized(const QModelIndex& rkIndex, u32 OldSize)
{
    //FIXME
    /*CArrayProperty *pArray = static_cast<CArrayProperty*>(PropertyForIndex(rkIndex, false));
    u32 NewSize = pArray->Count();

    if (NewSize != OldSize)
    {
        if (pArray->Count() > OldSize)
            endInsertRows();
        else
            endRemoveRows();
    }*/
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
