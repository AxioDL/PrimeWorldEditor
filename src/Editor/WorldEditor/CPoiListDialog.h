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
    CPoiListModel(CScriptTemplate *pPoiTemplate, CPoiMapModel *pMapModel, CScene *pScene, QWidget *pParent = nullptr)
        : QAbstractListModel(pParent)
        , mpPoiTemplate(pPoiTemplate)
    {
        const std::list<CScriptObject*>& rkObjList = mpPoiTemplate->ObjectList();

        for (auto it = rkObjList.begin(); it != rkObjList.end(); it++)
        {
            CScriptNode *pNode = pScene->NodeForInstance(*it);

            if (!pMapModel->IsPoiTracked(pNode))
                mObjList.push_back(pNode);
        }
    }

    int rowCount(const QModelIndex&) const override
    {
        return mObjList.size();
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const override
    {
        if (!rkIndex.isValid()) return QVariant::Invalid;

        if (Role == Qt::DisplayRole)
            return TO_QSTRING(mObjList[rkIndex.row()]->Instance()->InstanceName());

        if (Role == Qt::DecorationRole)
        {
            const CScriptNode *pNode = mObjList[rkIndex.row()];
            const CScan *pScan = static_cast<CPointOfInterestExtra*>(pNode->Extra())->GetScan();
            const bool IsImportant = (pScan ? pScan->IsCriticalPropertyRef() : false);

            if (IsImportant)
                return QIcon(QStringLiteral(":/icons/POI Important.svg"));
            else
                return QIcon(QStringLiteral(":/icons/POI Normal.svg"));
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
    CPoiListDialog(CScriptTemplate *pPoiTemplate, CPoiMapModel *pMapModel, CScene *pScene, QWidget *pParent = nullptr)
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

        setWindowTitle(tr("Add POIs"));
        mpListView->setEditTriggers(QListView::NoEditTriggers);
        mpListView->setSelectionMode(QListView::ExtendedSelection);
        mpListView->setVerticalScrollMode(QListView::ScrollPerPixel);

        connect(mpListView, &QListView::doubleClicked, this, &CPoiListDialog::OnOkClicked);
        connect(mpButtonBox, &QDialogButtonBox::accepted, this, &CPoiListDialog::OnOkClicked);
        connect(mpButtonBox, &QDialogButtonBox::rejected, this, &CPoiListDialog::OnCancelClicked);
    }

    const QList<CScriptNode*>& Selection() const
    {
        return mSelection;
    }

public slots:
    void OnOkClicked()
    {
        QModelIndexList SelectedIndices = mpListView->selectionModel()->selectedRows();

        for (const QModelIndex& rkIndex : SelectedIndices)
        {
            QModelIndex SourceIndex = mModel.mapToSource(rkIndex);
            mSelection.push_back(mSourceModel.PoiForIndex(SourceIndex));
        }

        close();
    }

    void OnCancelClicked()
    {
        close();
    }
};

#endif // CPOILISTDIALOG_H
