#ifndef CPROPERTYMODEL_H
#define CPROPERTYMODEL_H

#include <Core/Resource/Script/Property/Properties.h>
#include <QAbstractItemModel>
#include <QFont>

class CPropertyModel : public QAbstractItemModel
{
    Q_OBJECT

    struct SProperty
    {
        IPropertyNew* pProperty;
        QModelIndex Index;
        int ParentID;
        std::vector<int> ChildIDs;
    };
    QVector<SProperty> mProperties;
    QMap<IPropertyNew*, int> mPropertyToIDMap;

    CGameProject* mpProject;
    CScriptObject* mpObject; // may be null
    IPropertyNew* mpRootProperty;
    void* mpPropertyData;

    bool mBoldModifiedProperties;
    bool mShowNameValidity;
    QFont mFont;

    int RecursiveBuildArrays(IPropertyNew* pProperty, int ParentID);

public:
    CPropertyModel(QObject *pParent = 0);
    void ConfigureIntrinsic(CGameProject* pProject, IPropertyNew* pRootProperty, void* pPropertyData);
    void ConfigureScript(CGameProject* pProject, IPropertyNew* pRootProperty, CScriptObject* pObject);
    IPropertyNew* PropertyForIndex(const QModelIndex& rkIndex, bool HandleFlaggedIndices) const;
    QModelIndex IndexForProperty(IPropertyNew *pProp) const;

    int columnCount(const QModelIndex& rkParent) const;
    int rowCount(const QModelIndex& rkParent) const;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const;
    QModelIndex parent(const QModelIndex& rkChild) const;
    Qt::ItemFlags flags(const QModelIndex& rkIndex) const;

    void ArrayAboutToBeResized(const QModelIndex& rkIndex, u32 NewSize);
    void ArrayResized(const QModelIndex& rkIndex, u32 OldSize);
    void ResizeArray(const QModelIndex& rkIndex, u32 NewSize);

    void SetShowPropertyNameValidity(bool Enable);

    inline void SetFont(QFont Font) { mFont = Font; }
    inline void SetBoldModifiedProperties(bool Enable) { mBoldModifiedProperties = Enable; }
    inline void* GetPropertyData() const { return mpPropertyData; }
    inline CScriptObject* GetScriptObject() const { return mpObject; }

public slots:
    void NotifyPropertyModified(class CScriptObject *pInst, IPropertyNew *pProp);
    void NotifyPropertyModified(const QModelIndex& rkIndex);

signals:
    void PropertyModified(const QModelIndex& rkIndex);
};

#endif // CPROPERTYMODEL_H
