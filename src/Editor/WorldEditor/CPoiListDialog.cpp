#include "Editor/WorldEditor/CPoiListDialog.h"

#include "Editor/UICommon.h"
#include "Editor/WorldEditor/CPoiMapModel.h"

#include <Core/Resource/Scan/CScan.h>
#include <Core/Resource/Script/CScriptObject.h>
#include <Core/Resource/Script/CScriptTemplate.h>
#include <Core/Scene/CScene.h>
#include <Core/Scene/CScriptNode.h>
#include <Core/ScriptExtra/CPointOfInterestExtra.h>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QListView>
#include <QVBoxLayout>

CPoiListModel::CPoiListModel(CScriptTemplate* pPoiTemplate, const CPoiMapModel* pMapModel, CScene* pScene, QWidget* pParent)
    : QAbstractListModel(pParent)
    , mpPoiTemplate(pPoiTemplate)
{
    for (const auto* obj : mpPoiTemplate->ObjectList())
    {
        auto* pNode = pScene->NodeForInstance(obj);

        if (!pMapModel->IsPoiTracked(pNode))
            mObjList.push_back(pNode);
    }
}

CPoiListModel::~CPoiListModel() = default;

int CPoiListModel::rowCount(const QModelIndex&) const
{
    return int(mObjList.size());
}

QVariant CPoiListModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!rkIndex.isValid())
        return {};

    if (Role == Qt::DisplayRole)
        return TO_QSTRING(mObjList[rkIndex.row()]->Instance()->InstanceName());

    if (Role == Qt::DecorationRole)
    {
        const CScriptNode* pNode = mObjList[rkIndex.row()];
        CScan* pScan = static_cast<CPointOfInterestExtra*>(pNode->Extra())->GetScan();
        const bool IsImportant = (pScan ? pScan->IsCriticalPropertyRef() : false);

        if (IsImportant)
            return QIcon(QStringLiteral(":/icons/POI Important.svg"));
        else
            return QIcon(QStringLiteral(":/icons/POI Normal.svg"));
    }

    return {};
}

CScriptNode* CPoiListModel::PoiForIndex(const QModelIndex& rkIndex) const
{
    return mObjList[rkIndex.row()];
}

CPoiListDialog::CPoiListDialog(CScriptTemplate* pPoiTemplate, const CPoiMapModel* pMapModel, CScene* pScene, QWidget* pParent)
    : QDialog(pParent)
    , mSourceModel(pPoiTemplate, pMapModel, pScene)
{
    mpListView = new QListView(this);
    mpButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* pButtonLayout = new QHBoxLayout();
    pButtonLayout->addStretch();
    pButtonLayout->addWidget(mpButtonBox);

    auto* pLayout = new QVBoxLayout();
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

CPoiListDialog::~CPoiListDialog() = default;

void CPoiListDialog::OnOkClicked()
{
    const auto SelectedIndices = mpListView->selectionModel()->selectedRows();

    for (const QModelIndex& rkIndex : SelectedIndices)
    {
        const auto SourceIndex = mModel.mapToSource(rkIndex);
        mSelection.push_back(mSourceModel.PoiForIndex(SourceIndex));
    }

    close();
}

void CPoiListDialog::OnCancelClicked()
{
    close();
}
