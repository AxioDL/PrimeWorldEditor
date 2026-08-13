#ifndef CPROPERTYMODEL_H
#define CPROPERTYMODEL_H

#include <Core/Resource/Script/Property/IProperty.h>
#include <QAbstractItemModel>
#include <QFont>
#include <QList>
#include <QMap>

class CGameProject;
class CScriptObject;
class IProperty;

class CPropertyModel : public QAbstractItemModel
{
    Q_OBJECT

    struct SProperty;
    QList<SProperty> mProperties;
    QMap<const IProperty*, int> mPropertyToIDMap;
    int mFirstUnusedID = -1;

    CGameProject* mpProject = nullptr;
    CScriptObject* mpObject  = nullptr; // may be null
    IProperty* mpRootProperty = nullptr;
    void* mpPropertyData = nullptr;

    bool mBoldModifiedProperties = true;
    bool mShowNameValidity = false;
    QFont mFont;

    int RecursiveBuildArrays(IProperty* pProperty, int ParentID);

public:
    explicit CPropertyModel(QObject *pParent = nullptr);
    ~CPropertyModel() override;

    void ConfigureIntrinsic(CGameProject* pProject, IProperty* pRootProperty, void* pPropertyData);
    void ConfigureScript(CGameProject* pProject, IProperty* pRootProperty, CScriptObject* pObject);
    IProperty* PropertyForIndex(const QModelIndex& rkIndex, bool HandleFlaggedIndices) const;
    QModelIndex IndexForProperty(const IProperty* pProp) const;
    void* DataPointerForIndex(const QModelIndex& rkIndex) const;

    int columnCount(const QModelIndex& rkParent = QModelIndex()) const override;
    int rowCount(const QModelIndex& rkParent = QModelIndex()) const override;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const override;

    void ArrayAboutToBeResized(const QModelIndex& rkIndex, uint32_t NewSize);
    void ArrayResized(const QModelIndex& rkIndex, uint32_t OldSize);
    void ClearSlot(int ID);

    EPropertyType GetEffectiveFieldType(IProperty* pProperty) const;
    void SetShowPropertyNameValidity(bool Enable);

    void SetFont(QFont Font) { mFont = std::move(Font); }
    void SetBoldModifiedProperties(bool Enable) { mBoldModifiedProperties = Enable; }
    CScriptObject* GetScriptObject() const { return mpObject; }

public slots:
    void NotifyPropertyModified(const QModelIndex& rkIndex);

signals:
    void PropertyModified(const QModelIndex& rkIndex);
};

#endif // CPROPERTYMODEL_H
