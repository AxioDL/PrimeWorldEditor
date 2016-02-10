#include "CPropertyModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/IProperty.h>
#include <Core/Resource/Script/IPropertyTemplate.h>
#include <QFont>
#include <QSize>

CPropertyModel::CPropertyModel(QObject *pParent /*= 0*/)
    : QAbstractItemModel(pParent)
    , mpBaseStruct(nullptr)
    , mBoldModifiedProperties(true)
{
}

void CPropertyModel::SetBaseStruct(CPropertyStruct *pBaseStruct)
{
    beginResetModel();
    mpBaseStruct = pBaseStruct;
    endResetModel();
}

IProperty* CPropertyModel::PropertyForIndex(const QModelIndex& rkIndex, bool HandleFlaggedPointers) const
{
    if (!rkIndex.isValid()) return mpBaseStruct;

    if (rkIndex.internalId() & 0x1)
    {
        if (HandleFlaggedPointers)
        {
            void *pID = (void*) (rkIndex.internalId() & ~0x1);
            return static_cast<IProperty*>(pID);
        }
        else
            return nullptr;
    }

    return static_cast<IProperty*>(rkIndex.internalPointer());
}

QModelIndex CPropertyModel::IndexForProperty(IProperty *pProp) const
{
    if (pProp == mpBaseStruct) return QModelIndex();

    QVector<u32> RowNumbers;
    IProperty *pChild = pProp;
    CPropertyStruct *pParent = pProp->Parent();

    while (pParent)
    {
        // Check for array with one sub-property
        CPropertyStruct *pGrandparent = pParent->Parent();
        if (pGrandparent && pGrandparent->Type() == eArrayProperty && pParent->Count() == 1)
        {
            pChild = pParent;
            pParent = pGrandparent;
            continue;
        }

        // Find row index for this child property
        for (u32 iChild = 0; iChild < pParent->Count(); iChild++)
        {
            if (pParent->PropertyByIndex(iChild) == pChild)
            {
                RowNumbers << iChild;
                break;
            }
        }

        pChild = pParent;
        pParent = pGrandparent;
    }

    // Find the corresponding QModelIndex in the same spot
    QModelIndex Index = QModelIndex();

    for (int iChild = RowNumbers.size() - 1; iChild >= 0; iChild--)
        Index = index(RowNumbers[iChild], 0, Index);

    return Index;
}

int CPropertyModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return 2;
}

int CPropertyModel::rowCount(const QModelIndex& rkParent) const
{
    if (!mpBaseStruct) return 0;
    if (!rkParent.isValid()) return mpBaseStruct->Count();
    if (rkParent.column() != 0) return 0;
    if (rkParent.internalId() & 0x1) return 0;

    IProperty *pProp = PropertyForIndex(rkParent, false);

    switch (pProp->Type())
    {
    case eStructProperty:
    case eArrayProperty:
        return static_cast<CPropertyStruct*>(pProp)->Count();

    case eBitfieldProperty:
        return static_cast<CBitfieldTemplate*>(pProp->Template())->NumFlags();

    case eVector3Property:
        return 3;

    case eColorProperty:
        return 4;

    case eCharacterProperty:
    {
        CAnimationParameters Params = static_cast<TCharacterProperty*>(pProp)->Get();
        if (Params.Version() <= eEchoes) return 3;
        if (Params.Version() <= eCorruption) return 2;
        return 5;
    }

    default:
        return 0;
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
        if (rkIndex.internalId() & 0x1)
        {
            IProperty *pProp = PropertyForIndex(rkIndex, true);

            if (pProp->Type() == eColorProperty)
            {
                if (rkIndex.column() == 0)
                {
                    if (rkIndex.row() == 0) return "R";
                    if (rkIndex.row() == 1) return "G";
                    if (rkIndex.row() == 2) return "B";
                    if (rkIndex.row() == 3) return "A";
                }

                else if (rkIndex.column() == 1)
                {
                    TStringList Strings = pProp->ToString().Split(" ,");

                    int i = 0;
                    for (auto it = Strings.begin(); it != Strings.end(); it++)
                    {
                        if (i == rkIndex.row()) return TO_QSTRING(*it);
                        i++;
                    }
                }
            }

            else if (pProp->Type() == eVector3Property)
            {
                if (rkIndex.column() == 0)
                {
                    if (rkIndex.row() == 0) return "X";
                    if (rkIndex.row() == 1) return "Y";
                    if (rkIndex.row() == 2) return "Z";
                }

                else if (rkIndex.column() == 1)
                {
                    TStringList Strings = pProp->ToString().Split(" ,");

                    int i = 0;
                    for (auto it = Strings.begin(); it != Strings.end(); it++)
                    {
                        if (i == rkIndex.row()) return TO_QSTRING(*it);
                        i++;
                    }
                }
            }

            else if (pProp->Type() == eBitfieldProperty)
            {
                CBitfieldTemplate *pBitfield = static_cast<CBitfieldTemplate*>(pProp->Template());

                if (rkIndex.column() == 0)
                    return TO_QSTRING(pBitfield->FlagName(rkIndex.row()));

                if (rkIndex.column() == 1)
                {
                    if (Role == Qt::DisplayRole)
                        return "";
                    else
                        return TO_QSTRING(TString::HexString(pBitfield->FlagMask(rkIndex.row()), true, true, 8));
                }
            }

            else if (pProp->Type() == eCharacterProperty)
            {
                TCharacterProperty *pChar = static_cast<TCharacterProperty*>(pProp);
                CAnimationParameters Params = pChar->Get();

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
                        else return "Unknown " + QString::number(rkIndex.row());
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

                if (pParent)
                {
                    // For direct array sub-properties, display the element name instead of the property name (the property name is the array name)
                    if (pProp->Type() == eStructProperty && pParent->Type() == eArrayProperty)
                    {
                        TString ElementName = static_cast<CArrayProperty*>(pParent)->ElementName();
                        return QString("%1 %2").arg(TO_QSTRING(ElementName)).arg(rkIndex.row() + 1);
                    }

                    // Check whether the parent struct is an array element with one sub-property
                    if (pParent->Type() == eStructProperty && pParent->Parent() && pParent->Parent()->Type() == eArrayProperty)
                    {
                        if (static_cast<CPropertyStruct*>(pParent)->Count() == 1)
                            return QString("%1 %2").arg(TO_QSTRING(pProp->Name())).arg(rkIndex.row() + 1);
                    }
                }

                // Display property name for everything else
                return TO_QSTRING(pProp->Name());
            }

            if (rkIndex.column() == 1)
            {
                switch (pProp->Type())
                {
                // Enclose vector property text in parentheses
                case eVector3Property:
                    return "(" + TO_QSTRING(pProp->ToString()) + ")";

                // Display character name for characters
                case eCharacterProperty:
                    return TO_QSTRING(static_cast<TCharacterProperty*>(pProp)->Get().GetCurrentCharacterName());

                // Display enumerator name for enums (but only on ToolTipRole)
                case eEnumProperty:
                    if (Role == Qt::ToolTipRole)
                    {
                        TEnumProperty *pEnum = static_cast<TEnumProperty*>(pProp);
                        CEnumTemplate *pTemp = static_cast<CEnumTemplate*>(pEnum->Template());
                        return TO_QSTRING(pTemp->EnumeratorName(pEnum->Get()));
                    }
                    else return "";

                // Display the element count for arrays
                case eArrayProperty:
                {
                    u32 Count = static_cast<CArrayProperty*>(pProp)->Count();
                    return QString("%1 element%2").arg(Count).arg(Count != 1 ? "s" : "");
                }

                // No display text on properties with persistent editors
                case eBoolProperty:
                case eFileProperty:
                case eColorProperty:
                    if (Role == Qt::DisplayRole)
                        return "";
                // fall through
                // Display property value to string for everything else
                default:
                    return TO_QSTRING(pProp->ToString() + pProp->Template()->Suffix());
                }
            }
        }
    }

    if (Role == Qt::ToolTipRole && rkIndex.column() == 0)
    {
        if (!(rkIndex.internalId() & 0x1))
        {
            // Add name
            IProperty *pProp = PropertyForIndex(rkIndex, false);
            QString DisplayText = data(rkIndex, Qt::DisplayRole).toString();
            QString Text = QString("<b>%1</b> <i>(%2)</i>").arg(DisplayText).arg(TO_QSTRING(PropEnumToPropString(pProp->Type())));

            // Add uncooked notification
            if (pProp->Template()->CookPreference() == eNeverCook)
            {
                Text.prepend("<i>[uncooked]</i>");
            }

            // Add description
            TString Desc = pProp->Template()->Description();
            if (!Desc.IsEmpty()) Text += "<br/>" + TO_QSTRING(Desc);
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

            if (!pProp->IsInArray())
            {
                if (rkIndex.internalId() & 0x1)
                {
                    if (pProp->Type() == eVector3Property)
                    {
                        TVector3Property *pVec = static_cast<TVector3Property*>(pProp);
                        TVector3Template *pTemp = static_cast<TVector3Template*>(pProp->Template());

                        CVector3f Value = pVec->Get();
                        CVector3f Default = pTemp->GetDefaultValue();

                        if (rkIndex.row() == 0) Bold = (Value.x != Default.x);
                        if (rkIndex.row() == 1) Bold = (Value.y != Default.y);
                        if (rkIndex.row() == 2) Bold = (Value.z != Default.z);
                    }

                    else if (pProp->Type() == eColorProperty)
                    {
                        TColorProperty *pColor = static_cast<TColorProperty*>(pProp);
                        TColorTemplate *pTemp = static_cast<TColorTemplate*>(pProp->Template());

                        CColor Value = pColor->Get();
                        CColor Default = pTemp->GetDefaultValue();

                        if (rkIndex.row() == 0) Bold = (Value.r != Default.r);
                        if (rkIndex.row() == 1) Bold = (Value.g != Default.g);
                        if (rkIndex.row() == 2) Bold = (Value.b != Default.b);
                        if (rkIndex.row() == 3) Bold = (Value.a != Default.a);
                    }
                }

                else
                {
                    Bold = !pProp->MatchesDefault();
                }
             }
        }

        Font.setBold(Bold);
        return Font;
    }

    if (Role == Qt::SizeHintRole)
    {
        return QSize(0, 23);
    }

    return QVariant::Invalid;
}

QModelIndex CPropertyModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    // Invalid index
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    // Check property for children
    IProperty *pParent = (rkParent.isValid() ? PropertyForIndex(rkParent, false) : mpBaseStruct);

    // Struct
    if (pParent->Type() == eStructProperty)
    {
        IProperty *pProp = static_cast<CPropertyStruct*>(pParent)->PropertyByIndex(Row);
        return createIndex(Row, Column, pProp);
    }

    // Array
    if (pParent->Type() == eArrayProperty)
    {
        IProperty *pProp = static_cast<CArrayProperty*>(pParent)->PropertyByIndex(Row);

        // If this array element only has one sub-property then let's just skip the redundant tree node and show the sub-property directly.
        CPropertyStruct *pStruct = static_cast<CPropertyStruct*>(pProp);
        if (pStruct->Count() == 1)
            pProp = pStruct->PropertyByIndex(0);

        return createIndex(Row, Column, pProp);
    }

    // Other property
    if (pParent->Type() == eColorProperty || pParent->Type() == eVector3Property || pParent->Type() == eBitfieldProperty || pParent->Type() == eCharacterProperty)
        return createIndex(Row, Column, u32(pParent) | 0x1);

    return QModelIndex();
}

QModelIndex CPropertyModel::parent(const QModelIndex& rkChild) const
{
    // Invalid index
    if (!rkChild.isValid())
        return QModelIndex();

    // Find parent property
    IProperty *pParent;

    if (rkChild.internalId() & 0x1)
        pParent = PropertyForIndex(rkChild, true);
    else
        pParent = PropertyForIndex(rkChild, false)->Parent();

    if (pParent == mpBaseStruct)
        return QModelIndex();

    // Iterate over grandfather properties until we find the row
    CPropertyStruct *pGrandparent = pParent->Parent();

    // Check for array with one sub-property
    if (pGrandparent->Type() == eArrayProperty)
    {
        CPropertyStruct *pStruct = static_cast<CPropertyStruct*>(pParent);

        if (pStruct->Count() == 1)
        {
            pParent = pGrandparent;
            pGrandparent = pGrandparent->Parent();
        }
    }

    for (u32 iProp = 0; iProp < pGrandparent->Count(); iProp++)
    {
        if (pGrandparent->PropertyByIndex(iProp) == pParent)
            return createIndex(iProp, 0, pParent);
    }

    return QModelIndex();
}

Qt::ItemFlags CPropertyModel::flags(const QModelIndex& rkIndex) const
{
    if (rkIndex.column() == 0) return Qt::ItemIsEnabled;
    else return (Qt::ItemIsEnabled | Qt::ItemIsEditable);
}

void CPropertyModel::NotifyPropertyModified(IProperty *pProp)
{
    NotifyPropertyModified(IndexForProperty(pProp));
}

void CPropertyModel::NotifyPropertyModified(const QModelIndex& rkIndex)
{
    if (rowCount(rkIndex) != 0)
        emit dataChanged( index(0, 0, rkIndex), index(rowCount(rkIndex) - 1, 1, rkIndex));

    if (rkIndex.internalId() & 0x1)
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
    }
}

void CPropertyModel::ArrayResized(const QModelIndex& rkIndex, u32 OldSize)
{
    CArrayProperty *pArray = static_cast<CArrayProperty*>(PropertyForIndex(rkIndex, false));
    u32 NewSize = pArray->Count();

    if (NewSize != OldSize)
    {
        if (pArray->Count() > OldSize)
            endInsertRows();
        else
            endRemoveRows();
    }
}
