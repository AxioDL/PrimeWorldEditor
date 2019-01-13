#ifndef CPOILISTDIALOG_H
#define CPOILISTDIALOG_H

#include "CPoiMapModel.h"
#include "Editor/UICommon.h"

#include <Core/Resource/Scan/CScan.h>
#include <Core/Resource/Script/CScriptTemplate.h>
#include <Core/Scene/CScene.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

#include <QAbstractListModel>
#include <QDialogButtonBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

class CPoiListModel : public QAbstractListModel
{
    Q_OBJECT

    CScriptTemplate *mpPoiTemplate;
    QList<CScriptNode*> mObjList;

public:
    CPoiListModel(CScriptTemplate *pPoiTemplate, CPoiMapModel *pMapModel, CScene *pScene, QWidget *pParent = 0)
        : QAbstractListModel(pParent)
        , mpPoiTemplate(pPoiTemplate)
    {
        const std::list<CScriptObject*>& rkObjList = mpPoiTemplate->ObjectList();

        for (auto it = rkObjList.begin(); it != rkObjList.end(); it++)
        {
            CScriptNode *pNode = pScene->NodeForInstance(*it);

            if (!pMapModel->IsPoiTracked(pNode))
                mObjList << pNode;
        }
    }

    int rowCount(const QModelIndex&) const
    {
        return mObjList.size();
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
    {
        if (!rkIndex.isValid()) return QVariant::Invalid;

        if (Role == Qt::DisplayRole)
            return TO_QSTRING(mObjList[rkIndex.row()]->Instance()->InstanceName());

        if (Role == Qt::DecorationRole)
        {
            CScriptNode *pNode = mObjList[rkIndex.row()];
            CScan *pScan = static_cast<CPointOfInterestExtra*>(pNode->Extra())->GetScan();
            bool IsImportant = (pScan ? pScan->IsCriticalPropertyRef() : false);

            if (IsImportant)
                return QIcon(":/icons/POI Important.png");
            else
                return QIcon(":/icons/POI Normal.png");
        }

        return QVariant::Invalid;
    }

    CScriptNode* PoiForIndex(const QModelIndex& rkIndex) const
    {
        return mObjList[rkIndex.row()];
    }
};

class CPoiListDialog : public QDialog
{
    Q_OBJECT

    CPoiListModel mSourceModel;
    QSortFilterProxyModel mModel;
    QList<CScriptNode*> mSelection;

    QListView *mpListView;
    QDialogButtonBox *mpButtonBox;

public:
    CPoiListDialog(CScriptTemplate *pPoiTemplate, CPoiMapModel *pMapModel, CScene *pScene, QWidget *pParent = 0)
        : QDialog(pParent)
        , mSourceModel(pPoiTemplate, pMapModel, pScene)
    {
        mpListView = new QListView(this);
        mpButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

        QHBoxLayout *pButtonLayout = new QHBoxLayout();
        pButtonLayout->addStretch();
        pButtonLayout->addWidget(mpButtonBox);

        QVBoxLayout *pLayout = new QVBoxLayout();
        pLayout->addWidget(mpListView);
        pLayout->addLayout(pButtonLayout);
        setLayout(pLayout);

        mModel.setSourceModel(&mSourceModel);
        mpListView->setModel(&mModel);
        mModel.sort(0);

        setWindowTitle("Add POIs");
        mpListView->setEditTriggers(QListView::NoEditTriggers);
        mpListView->setSelectionMode(QListView::ExtendedSelection);
        mpListView->setVerticalScrollMode(QListView::ScrollPerPixel);

        connect(mpListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnOkClicked()));
        connect(mpButtonBox, SIGNAL(accepted()), this, SLOT(OnOkClicked()));
        connect(mpButtonBox, SIGNAL(rejected()), this, SLOT(OnCancelClicked()));
    }

    const QList<CScriptNode*>& Selection() const
    {
        return mSelection;
    }

public slots:
    void OnOkClicked()
    {
        QModelIndexList SelectedIndices = mpListView->selectionModel()->selectedRows();

        foreach (const QModelIndex& rkIndex, SelectedIndices)
        {
            QModelIndex SourceIndex = mModel.mapToSource(rkIndex);
            mSelection << mSourceModel.PoiForIndex(SourceIndex);
        }

        close();
    }

    void OnCancelClicked()
    {
        close();
    }
};

#endif // CPOILISTDIALOG_H
